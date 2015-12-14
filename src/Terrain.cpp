#define NOMINMAX

#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "AABox.h"
#ifndef STANDALONE_VERSION
#include "config.hpp"
#else
#define SHADERS_PATH(n) SHADERS_DIR n
#define RESOURCES_PATH(n) RESOURCES_DIR n
#endif
#include "Terrain.h"
#include "FPSCamera.h"
#include "GLB.h"
#include "GLStateInspection.h"
#include "GLStateInspectionView.h"
#include "imgui_impl_glfw_gl3.h"
#include "InputHandler.h"
#include "Log.h"
#include "LogView.h"
#include "ObjReader.h"
#include "Profiler.h"
#include "SimpleDraw.h"
#include "Window.h"


#define RES_X			 1600
#define RES_Y            900
#define DENSITY_RES_X  33
#define DENSITY_RES_Y  33
#define DENSITY_RES_Z  33
#define DENSITY_SIZE  33

#define MSAA_RATE	             1
#define LIGHT_INTENSITY     240000.0f
#define LIGHT_ANGLE_FALLOFF      0.8f
#define LIGHT_CUTOFF             0.05f

#define LIGHTS_NB 4

float noise[DENSITY_SIZE][DENSITY_SIZE][DENSITY_SIZE];

Terrain::Terrain(int argc, const char* argv[])
{
	Log::View::Init();

	window = Window::Create("Terrain Generation", RES_X, RES_Y, MSAA_RATE, false);
	inputHandler = new InputHandler();
	window->SetInputHandler(inputHandler);

	SimpleDraw::Init();
	GLStateInspection::Init();
	GLStateInspection::View::Init();
}

Terrain::~Terrain()
{
	GLStateInspection::View::Destroy();
	GLStateInspection::Destroy();
	SimpleDraw::Destroy();

	delete inputHandler;
	inputHandler = nullptr;

	Window::Destroy(window);
	window = nullptr;

	Log::View::Destroy();
}

void Terrain::run()
{

	//
	// Compute the scene bounding box to scale the camera speed, near and far plane accordingly
	//

	float sceneScale = 10.0;

	//
	// Setup the camera. 
	//
	// NOTE: If the points don't show up at first, just press the left mouse button and they should appear. The camera orientation shouldn't need to be changed
	// 
	FPSCameraf mCamera = FPSCameraf(fPI / 4.0f, static_cast<float>(RES_X) / static_cast<float>(RES_Y), sceneScale * 0.01f, sceneScale * 4.0f);
	mCamera.mWorld.SetTranslate(v3f(16.f, 20.f, 33.f));//v3f(sceneScale * 0.17f, sceneScale * 0.03f, 0.0f));
	mCamera.mRotation.x = 0;// / 2.0f;
	mCamera.mWorld.SetRotateY(0);//fPI / 2.0f);
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = sceneScale * 0.25f;


	//-------------------------Bonobo version----------------------------------

	//Generate FBO and storing texture
	//bonobo::Texture *rtTestMap = bonobo::loadTexture2D(RES_X, RES_Y, bonobo::TEXTURE_UNORM, v4i(8, 8, 8, 8), MSAA_RATE);
	//glEnable(GL_TEXTURE_3D);

	bonobo::Texture *rtTestTexture3D = bonobo::loadTexture3D(nullptr, DENSITY_RES_X, DENSITY_RES_Y, DENSITY_RES_Z, bonobo::TEXTURE_FLOAT, v4i(32, 0, 0, 0), 0);
	bonobo::Texture *edgeTexture = new bonobo::Texture();
	bonobo::Texture *densityTexture3D = new bonobo::Texture();

	//const bonobo::Texture *gTest[1] = { rtTestTexture3D };
	//bonobo::FBO *texture3DFbo = bonobo::loadFrameBufferObject(gTest, 1);

	//
	// Load all the shader programs used
	std::string densityShaderNames[2] = { SHADERS_PATH("density.vert"), SHADERS_PATH("density.frag") };
	//std::string testingShaderNames[3] = { SHADERS_PATH("testing.vert"),	SHADERS_PATH("testing.geo"), SHADERS_PATH("testing.frag") };
	std::string terrainShaderNames[3] = { SHADERS_PATH("terrain.vert"),	SHADERS_PATH("terrain.geo"), SHADERS_PATH("terrain.frag") };
	bonobo::ShaderProgram *densityShader = bonobo::loadShaderProgram(densityShaderNames, 2);
	//bonobo::ShaderProgram *testingShader = bonobo::loadShaderProgram(testingShaderNames, 3);
	bonobo::ShaderProgram *terrainShader = bonobo::loadShaderProgram(terrainShaderNames, 3);

	if (densityShader == nullptr) {
		LogError("Failed to load density shader\n");
		exit(-1);
	}
	/*if (testingShader == nullptr) {
		LogError("Failed to load testing shader\n");
		exit(-1);
	}*/
	if (terrainShader == nullptr) {
		LogError("Failed to load density shader\n");
		exit(-1);
	}

	const int EDGES_SIZE = 256 * 15;
	int *edges = createLookupTable();
	
	glGenTextures(1, &edgeTexture->mId);
	bonobo::checkForErrors();
	glBindTexture(GL_TEXTURE_1D, edgeTexture->mId);
	bonobo::checkForErrors();
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32I, EDGES_SIZE, 0, GL_RED_INTEGER, GL_INT, edges);
	//glGenerateTextureMipmap(edgeTexture->mId);
	bonobo::checkForErrors();
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, 0);
	edgeTexture->mTarget = bonobo::TEXTURE_1D;

	//facesTexture = bonobo::loadTexture1D(facesSize, bonobo::TEXTURE_INT, v4i(32, 0, 0, 0));

	/*bonobo::checkForErrors();
	glBindTexture(GL_TEXTURE_1D, edgeTexture->mId);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, edgesSize, GL_RED_INTEGER, GL_INT, edges);
	bonobo::checkForErrors();*/

	// Check texture values
	/*int facesArrayTest[edgesSize];
	glGetTexImage(GL_TEXTURE_1D, 0, GL_RED_INTEGER, GL_INT, facesArrayTest);
	for (int i = 0; i < edgesSize; i++)
	{
		printf("%d\n", facesArrayTest[i]);
	}*/

	glBindTexture(GL_TEXTURE_1D, 0);



	/*glGenTextures(1, &facesTexture->mId);
	glBindTexture(GL_TEXTURE_1D, facesTexture->mId);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, 256 * 15, 0, GL_RED, GL_FLOAT, faces);

	facesTexture->mTarget = bonobo::TEXTURE_1D;
	glBindTexture(GL_TEXTURE_1D, 0);*/

	
	// Create noise function
	for (int x = 0; x < DENSITY_SIZE; x++)
	{
		for (int y = 0; y < DENSITY_SIZE; y++)
		{
			for (int z = 0; z < DENSITY_SIZE; z++)
			{
				noise[x][y][z] = (rand() % 32768) / 32768;
			}
		}
	}
	
	// Create density function
	float densityFunction3D[DENSITY_SIZE][DENSITY_SIZE][DENSITY_SIZE];
	for (int x = 0; x < DENSITY_SIZE; x++)
	{
		for (int y = 0; y < DENSITY_SIZE; y++)
		{
			for (int z = 0; z < DENSITY_SIZE; z++)
			{
				densityFunction3D[x][y][z] = (y - 16.f);// + turbulence(x, y, z);
			}
		}
	}

	/*glGenTextures(1, &densityTexture3D->mId);
	bonobo::checkForErrors();
	glBindTexture(GL_TEXTURE_3D, densityTexture3D->mId);
	bonobo::checkForErrors();
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	bonobo::checkForErrors();
	
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, densitySize, densitySize, densitySize, 0, GL_RED, GL_FLOAT, densityFunction3D);
	glGenerateTextureMipmap(densityTexture3D->mId);
	bonobo::checkForErrors();
	densityTexture3D->mTarget = bonobo::TEXTURE_3D;*/

	densityTexture3D = bonobo::loadTexture3D(nullptr, DENSITY_SIZE, DENSITY_SIZE, DENSITY_SIZE, bonobo::TEXTURE_FLOAT, v4i(32, 0, 0, 0));
	glBindTexture(GL_TEXTURE_3D, densityTexture3D->mId);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, DENSITY_SIZE, DENSITY_SIZE, DENSITY_SIZE, GL_RED, GL_FLOAT, densityFunction3D);
	glBindTexture(GL_TEXTURE_3D, 0);

	/*// Check texture values
	float densityArrayTest[densitySize][densitySize][densitySize];
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, densityArrayTest);
	for (int i = 0; i < densitySize; i++)
	{
		printf("%f\n", densityArrayTest[0][i][0]);
	}*/

	//glBindTexture(GL_TEXTURE_3D, 0);

	/*glGenTextures(1, &facesTexture->mId);
	bonobo::checkForErrors();
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_BASE_LEVEL, 0);
	bonobo::checkForErrors();
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, 0);
	bonobo::checkForErrors();
	glBindTexture(GL_TEXTURE_1D, facesTexture->mId);
	bonobo::checkForErrors();
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, 256 * 15, 0, GL_RED, GL_UNSIGNED_INT, faces);
	bonobo::checkForErrors();
	glBindTexture(GL_TEXTURE_1D, 0);*/

	/*int *faces = createLookupTable();
	const int FACES_SIZE = 256 * 15;

	GLuint facesUBO;
	glGenBuffers(1, &facesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, facesUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * FACES_SIZE, NULL, GL_DYNAMIC_DRAW);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * FACES_SIZE, faces);

	GLuint arrayBlockIdx = glGetUniformBlockIndex(terrainShader->mId, "arrayBlock");
	glUniformBlockBinding(terrainShader->mId, arrayBlockIdx, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, facesUBO);*/

	// Generate and bind Vertex Buffer Object
	GLuint vbo = 0u;
	
	glGenBuffers(1, &vbo);
	bonobo::checkForErrors();

	GLuint ibo = 0u;
	glGenBuffers(1, &ibo);
	bonobo::checkForErrors();

	GLuint vboTerrain = 0u;
	glGenBuffers(1, &vboTerrain);
	bonobo::checkForErrors();

	// vec3:Vertex, vec3:Color, float:Sides
	float points[4 * 6] = {
		-1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, //4.0f,
		1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //8.0f,
		1.0f,  -1.0f, 0.0f, 0.0f, 0.0f, 1.0f , //16.0f,
		-1.0f,  -1.0f, 0.0f, 1.0f, 1.0f, 0.0f// 32.0f,
	};

	GLint indices[6] = {
		3, 2, 0,
		2, 1, 0,
	};

	//Generate grid for shady business
	const int voxelgridsize = 32;
	const int nbr_voxelPoints = voxelgridsize*voxelgridsize*voxelgridsize * 3;
	float voxelPoints[nbr_voxelPoints];
	float originPoint[3] = { 0.0f,0.0f,0.0f };

	for (int x = 0; x < voxelgridsize;x++)
	{
		for (int y = 0; y < voxelgridsize; y++)
		{
			for (int z = 0; z < voxelgridsize; z++)
			{
				voxelPoints[(x*voxelgridsize *voxelgridsize + y*voxelgridsize + z) * 3] = (float)x;//originPoint[0] + (float) x / 2.0f;
				voxelPoints[(x*voxelgridsize *voxelgridsize + y*voxelgridsize + z) * 3 + 1] = (float) y;//originPoint[0] + (float) y / 2.0f;
				voxelPoints[(x*voxelgridsize *voxelgridsize + y*voxelgridsize + z) * 3 + 2] = (float) z;//originPoint[0] + (float) z / 2.0f;
			}
		}
	}

	// Specify layout of point data
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	bonobo::checkForErrors();
	glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), points, GL_STATIC_DRAW);
	bonobo::checkForErrors();
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	bonobo::checkForErrors();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(indices), indices, GL_STATIC_DRAW);
	bonobo::checkForErrors();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Array buffer
	glBindBuffer(GL_ARRAY_BUFFER, vboTerrain);
	bonobo::checkForErrors();
	glBufferData(GL_ARRAY_BUFFER, voxelgridsize * voxelgridsize *voxelgridsize * 3 * sizeof(float), voxelPoints, GL_STATIC_DRAW);
	bonobo::checkForErrors();
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Generate and Bind Vertex Array Object
	GLuint vao = 0u;
	glGenVertexArrays(1, &vao);
	bonobo::checkForErrors();
	glBindVertexArray(vao);
	bonobo::checkForErrors();

	// Vertex Attribute
	GLint densityVertexAttrib = glGetAttribLocation(densityShader->mId, "Vertex");
	glEnableVertexAttribArray(densityVertexAttrib);
	bonobo::checkForErrors();
	glVertexAttribPointer(densityVertexAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	bonobo::checkForErrors();

	/*GLint testingVertexAttrib = glGetAttribLocation(testingShader->mId, "Vertex");
	glEnableVertexAttribArray(testingVertexAttrib);
	bonobo::checkForErrors();
	glVertexAttribPointer(testingVertexAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	bonobo::checkForErrors();*/

	GLint terrainVertexAttrib = glGetAttribLocation(terrainShader->mId, "Vertex");
	glEnableVertexAttribArray(terrainVertexAttrib);
	bonobo::checkForErrors();
	glVertexAttribPointer(terrainVertexAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	bonobo::checkForErrors();

	// Color Attribute
	//GLint colorAttrib = glGetAttribLocation(testingShader->mId, "Color");
	//glEnableVertexAttribArray(colorAttrib);
	//bonobo::checkForErrors();
	//glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	//bonobo::checkForErrors();

	glBindVertexArray(0u);
	bonobo::checkForErrors();
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	bonobo::checkForErrors();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
	bonobo::checkForErrors();

	glEnable(GL_DEPTH_TEST); // If this is enabled, the points don't show up
	glEnable(GL_CULL_FACE);
	//int i = 1;

	//create testure sampler
	bonobo::Sampler *sampler = bonobo::loadSampler();
	//bonobo::Sampler *samplerFaces = bonobo::loadSampler();

	//create texture layer buffer whibbly whobbly stuffy thingi...
	//for-loop for filling 3d texture
	//for (int i = 0; i < 33; i++) {
	//	//Create layer fbo
	//	GLuint fbo;
	//	glGenFramebuffers(1, &fbo);
	//	bonobo::checkForErrors();
	//	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//	bonobo::checkForErrors();
	//	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rtTestTexture3D->mId, 0, i);

	//	//bind said fbo
	//	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//	bonobo::checkForErrors();

	//	//draw buffer
	//	glNamedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
	//	bonobo::checkForErrors();

	//	//test fbo
	//	int result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	//	if (result != GL_FRAMEBUFFER_COMPLETE)
	//		LogWarning("Failed to bind FBO/RBO");
	//	bonobo::checkForErrors();



	//	//Pass 1: Generate Density Function
	//	//bonobo::setRenderTarget(fbo, 0);
	//	glUseProgram(densityShader->mId);
	//	glViewport(0, 0, DENSITY_RES_X, DENSITY_RES_Y);
	//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//	glClear(GL_COLOR_BUFFER_BIT);
	//	bonobo::checkForErrors();
	//	bonobo::setUniform(*densityShader, "model_to_clip_matrix", mat4f::Identity()/*cast<f32>(mCamera.GetWorldToClipMatrix())*/);

	//	glBindVertexArray(vao);
	//	bonobo::checkForErrors();

	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	//	bonobo::checkForErrors();

	//	//GLStateInspection::CaptureSnapshot("Terrain");

	//	//glDrawArrays(GL_TRIANGLES, 0, 3);
	//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//	bonobo::checkForErrors();
	//	glBindVertexArray(0u);
	//	//bonobo::drawFullscreen(*testingShader); // This is not needed! glDrawArrays and ImGUI::Render do the trick :)
	//	bonobo::checkForErrors();
	//}

	f64 ddeltatime;
	size_t fpsSamples = 0;
	double nowTime, lastTime = GetTimeSeconds();
	double fpsNextTick = lastTime + 1.0;
	auto startTime = std::chrono::system_clock::now();
	while (!glfwWindowShouldClose(window->GetGLFW_Window())) {
		PROFILE("Main loop");
		nowTime = GetTimeSeconds();
		ddeltatime = nowTime - lastTime;
		if (nowTime > fpsNextTick) {
			fpsNextTick += 1.0;
			fpsSamples = 0;
		}
		fpsSamples++;

		glfwPollEvents();
		inputHandler->Advance();
		mCamera.Update(ddeltatime, *inputHandler);

		ImGui_ImplGlfwGL3_NewFrame();

		glDepthFunc(GL_LESS);


		//printf("Camera Rotation no:1: %f, no:2: %f\n", (mCamera.mRotation).x, (mCamera.mRotation).y);


		//Pass 2: Stuff
		bonobo::setRenderTarget(0, 0);
		glUseProgram(terrainShader->mId);
		glViewport(0, 0, RES_X, RES_Y);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearDepthf(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		bonobo::checkForErrors();
		glBindTexture(GL_TEXTURE_3D, densityTexture3D->mId);
		glBindTexture(GL_TEXTURE_1D, edgeTexture->mId);
		bonobo::setUniform(*terrainShader, "model_to_clip_matrix", cast<f32>(mCamera.GetWorldToClipMatrix()));
		bonobo::bindTextureSampler(*terrainShader, "density_texture", 0, *densityTexture3D, *sampler);
		bonobo::bindTextureSampler(*terrainShader, "edge_texture", 1, *edgeTexture, *sampler);
		//Probably better way of doing this, but tired!!
		bonobo::setUniform(*terrainShader, "origin_x", originPoint[0]);
		bonobo::setUniform(*terrainShader, "origin_y", originPoint[1]);
		bonobo::setUniform(*terrainShader, "origin_z", originPoint[2]);

		glBindVertexArray(vao);
		bonobo::checkForErrors();

		/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		bonobo::checkForErrors();*/

		glBindBuffer(GL_ARRAY_BUFFER, vboTerrain);
		bonobo::checkForErrors();
		glEnableVertexAttribArray(0);
		bonobo::checkForErrors();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		bonobo::checkForErrors();

		GLStateInspection::CaptureSnapshot("Terrain");
		
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		
		glDrawArrays(GL_POINTS, 0, nbr_voxelPoints);
		bonobo::checkForErrors();

		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindSampler(1u, 0u);
		bonobo::checkForErrors();
		glBindSampler(0u, 0u);
		bonobo::checkForErrors();
		glBindTexture(GL_TEXTURE_3D, 0);
		glBindTexture(GL_TEXTURE_1D, 0);
		glBindVertexArray(0u);
		//bonobo::drawFullscreen(*testingShader); // This is not needed! glDrawArrays and ImGUI::Render do the trick :)
		bonobo::checkForErrors();



		//GLStateInspection::CaptureSnapshot("Terrain");
		//bonobo::checkForErrors();


		//output stuff
		//gSimpleDraw.Texture(v2f(-0.95f, -0.95f), v2f(-0.55f, -0.55f), *rtTestTexture3D, v4i(0, 1, 2, -1));

		GLStateInspection::View::Render(); // Disabling this turns off the GLStateInspection console within the render window
										   //Log::View::Render();
		ImGui::Render();

		window->Swap();
		lastTime = nowTime;
	}

	glDeleteBuffers(1, &vboTerrain);
	bonobo::checkForErrors();
	vbo = 0u;
	glDeleteBuffers(1, &ibo);
	bonobo::checkForErrors();
	ibo = 0u;


	glDeleteVertexArrays(1, &vao);
	bonobo::checkForErrors();
	vao = 0u;
}

float Terrain::turbulence(int x, int y, int z)
{
	float size = (float) DENSITY_SIZE;
	double value = 0.0, initialSize = size;

	while (size >= 1)
	{
		value += smoothNoise(x / size, y / size, z / size) * size;
		size /= 2.0;
	}

	return (128.0 * value / initialSize);
}

float Terrain::smoothNoise(int x, int y, int z)
{
	//get fractional part of x and y
	double fractX = x - int(x);
	double fractY = y - int(y);
	double fractZ = z - int(z);

	//wrap around
	int x1 = (int(x) + DENSITY_SIZE) % DENSITY_SIZE;
	int y1 = (int(y) + DENSITY_SIZE) % DENSITY_SIZE;
	int z1 = (int(z) + DENSITY_SIZE) % DENSITY_SIZE;

	//neighbor values
	int x2 = (x1 + DENSITY_SIZE - 1) % DENSITY_SIZE;
	int y2 = (y1 + DENSITY_SIZE - 1) % DENSITY_SIZE;
	int z2 = (z1 + DENSITY_SIZE - 1) % DENSITY_SIZE;

	//smooth the noise with bilinear interpolation
	double value = 0.0;
	value += fractX       * fractY       * fractZ       * noise[x1][y1][z1];
	value += fractX       * (1 - fractY) * fractZ       * noise[x1][y2][z1];
	value += (1 - fractX) * fractY       * fractZ       * noise[x2][y1][z1];
	value += (1 - fractX) * (1 - fractY) * fractZ       * noise[x2][y2][z1];

	value += fractX       * fractY       * (1 - fractZ) * noise[x1][y1][z2];
	value += fractX       * (1 - fractY) * (1 - fractZ) * noise[x1][y2][z2];
	value += (1 - fractX) * fractY       * (1 - fractZ) * noise[x2][y1][z2];
	value += (1 - fractX) * (1 - fractY) * (1 - fractZ) * noise[x2][y2][z2];

	return value;
}

int * Terrain::createLookupTable() 
{
	/*static int faces[256 * 15] =
	{
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 1, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 2, 11, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 8, 3, 2, 11, 8, 11, 9, 8, -1, -1, -1, -1, -1, -1,
		3, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 10, 2, 8, 10, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 9, 0, 2, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 10, 2, 1, 9, 10, 9, 8, 10, -1, -1, -1, -1, -1, -1,
		3, 11, 1, 10, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 11, 1, 0, 8, 11, 8, 10, 11, -1, -1, -1, -1, -1, -1,
		3, 9, 0, 3, 10, 9, 10, 11, 9, -1, -1, -1, -1, -1, -1,
		9, 8, 11, 11, 8, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1,
		1, 2, 11, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 4, 7, 3, 0, 4, 1, 2, 11, -1, -1, -1, -1, -1, -1,
		9, 2, 11, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1,
		2, 11, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1,
		8, 4, 7, 3, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		10, 4, 7, 10, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1,
		9, 0, 1, 8, 4, 7, 2, 3, 10, -1, -1, -1, -1, -1, -1,
		4, 7, 10, 9, 4, 10, 9, 10, 2, 9, 2, 1, -1, -1, -1,
		3, 11, 1, 3, 10, 11, 7, 8, 4, -1, -1, -1, -1, -1, -1,
		1, 10, 11, 1, 4, 10, 1, 0, 4, 7, 10, 4, -1, -1, -1,
		4, 7, 8, 9, 0, 10, 9, 10, 11, 10, 0, 3, -1, -1, -1,
		4, 7, 10, 4, 10, 9, 9, 10, 11, -1, -1, -1, -1, -1, -1,
		9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1,
		1, 2, 11, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 8, 1, 2, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1,
		5, 2, 11, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1,
		2, 11, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1,
		9, 5, 4, 2, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 10, 2, 0, 8, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1,
		0, 5, 4, 0, 1, 5, 2, 3, 10, -1, -1, -1, -1, -1, -1,
		2, 1, 5, 2, 5, 8, 2, 8, 10, 4, 8, 5, -1, -1, -1,
		11, 3, 10, 11, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1,
		4, 9, 5, 0, 8, 1, 8, 11, 1, 8, 10, 11, -1, -1, -1,
		5, 4, 0, 5, 0, 10, 5, 10, 11, 10, 0, 3, -1, -1, -1,
		5, 4, 8, 5, 8, 11, 11, 8, 10, -1, -1, -1, -1, -1, -1,
		9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1,
		0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1,
		1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 7, 8, 9, 5, 7, 11, 1, 2, -1, -1, -1, -1, -1, -1,
		11, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1,
		8, 0, 2, 8, 2, 5, 8, 5, 7, 11, 5, 2, -1, -1, -1,
		2, 11, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1,
		7, 9, 5, 7, 8, 9, 3, 10, 2, -1, -1, -1, -1, -1, -1,
		9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 10, -1, -1, -1,
		2, 3, 10, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1,
		10, 2, 1, 10, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1,
		9, 5, 8, 8, 5, 7, 11, 1, 3, 11, 3, 10, -1, -1, -1,
		5, 7, 10, 5, 10, 11, 1, 0, 9, -1, -1, -1, -1, -1, -1,
		10, 11, 5, 10, 5, 7, 8, 0, 3, -1, -1, -1, -1, -1, -1,
		10, 11, 5, 7, 10, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 5, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 0, 1, 5, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 8, 3, 1, 9, 8, 5, 11, 6, -1, -1, -1, -1, -1, -1,
		1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1,
		9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1,
		5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1,
		2, 3, 10, 11, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		10, 0, 8, 10, 2, 0, 11, 6, 5, -1, -1, -1, -1, -1, -1,
		0, 1, 9, 2, 3, 10, 5, 11, 6, -1, -1, -1, -1, -1, -1,
		5, 11, 6, 1, 9, 2, 9, 10, 2, 9, 8, 10, -1, -1, -1,
		6, 3, 10, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1,
		0, 8, 10, 0, 10, 5, 0, 5, 1, 5, 10, 6, -1, -1, -1,
		3, 10, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1,
		6, 5, 9, 6, 9, 10, 10, 9, 8, -1, -1, -1, -1, -1, -1,
		5, 11, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 3, 0, 4, 7, 3, 6, 5, 11, -1, -1, -1, -1, -1, -1,
		1, 9, 0, 5, 11, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1,
		11, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1,
		6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1,
		1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1,
		8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1,
		7, 3, 2, 7, 2, 6, 5, 9, 4, -1, -1, -1, -1, -1, -1,
		3, 10, 2, 7, 8, 4, 11, 6, 5, -1, -1, -1, -1, -1, -1,
		5, 11, 6, 4, 7, 2, 4, 2, 0, 2, 7, 10, -1, -1, -1,
		0, 1, 9, 4, 7, 8, 2, 3, 10, 5, 11, 6, -1, -1, -1,
		9, 4, 5, 11, 2, 1, 7, 10, 6, -1, -1, -1, -1, -1, -1,
		8, 4, 7, 3, 10, 5, 3, 5, 1, 5, 10, 6, -1, -1, -1,
		5, 1, 0, 5, 0, 4, 7, 10, 6, -1, -1, -1, -1, -1, -1,
		0, 3, 8, 4, 5, 9, 10, 6, 7, -1, -1, -1, -1, -1, -1,
		4, 5, 9, 7, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11, 4, 9, 6, 4, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 11, 6, 4, 9, 11, 0, 8, 3, -1, -1, -1, -1, -1, -1,
		11, 0, 1, 11, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1,
		8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 11, -1, -1, -1,
		1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1,
		3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1,
		0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1,
		11, 4, 9, 11, 6, 4, 10, 2, 3, -1, -1, -1, -1, -1, -1,
		0, 8, 2, 2, 8, 10, 4, 9, 11, 4, 11, 6, -1, -1, -1,
		3, 10, 2, 0, 1, 6, 0, 6, 4, 6, 1, 11, -1, -1, -1,
		6, 4, 8, 6, 8, 10, 2, 1, 11, -1, -1, -1, -1, -1, -1,
		9, 6, 4, 9, 3, 6, 9, 1, 3, 10, 6, 3, -1, -1, -1,
		8, 10, 6, 8, 6, 4, 9, 1, 0, -1, -1, -1, -1, -1, -1,
		3, 10, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1,
		6, 4, 8, 10, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 11, 6, 7, 8, 11, 8, 9, 11, -1, -1, -1, -1, -1, -1,
		0, 7, 3, 0, 11, 7, 0, 9, 11, 6, 7, 11, -1, -1, -1,
		11, 6, 7, 1, 11, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1,
		11, 6, 7, 11, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1,
		1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1,
		2, 6, 7, 2, 7, 3, 0, 9, 1, -1, -1, -1, -1, -1, -1,
		7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1,
		7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 3, 10, 11, 6, 8, 11, 8, 9, 8, 6, 7, -1, -1, -1,
		2, 0, 9, 2, 9, 11, 6, 7, 10, -1, -1, -1, -1, -1, -1,
		1, 11, 2, 3, 8, 0, 6, 7, 10, -1, -1, -1, -1, -1, -1,
		11, 2, 1, 6, 7, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 9, 1, 8, 1, 3, 10, 6, 7, -1, -1, -1, -1, -1, -1,
		0, 9, 1, 10, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 8, 0, 10, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 8, 10, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 9, 10, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 1, 9, 8, 3, 1, 10, 7, 6, -1, -1, -1, -1, -1, -1,
		11, 1, 2, 6, 10, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 11, 3, 0, 8, 6, 10, 7, -1, -1, -1, -1, -1, -1,
		2, 9, 0, 2, 11, 9, 6, 10, 7, -1, -1, -1, -1, -1, -1,
		2, 10, 3, 11, 8, 6, 11, 9, 8, 8, 7, 6, -1, -1, -1,
		7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1,
		2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1,
		1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1,
		11, 7, 6, 11, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1,
		11, 7, 6, 1, 7, 11, 1, 8, 7, 1, 0, 8, -1, -1, -1,
		0, 3, 7, 0, 7, 11, 0, 11, 9, 6, 11, 7, -1, -1, -1,
		7, 6, 11, 7, 11, 8, 8, 11, 9, -1, -1, -1, -1, -1, -1,
		6, 8, 4, 10, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 6, 10, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1,
		8, 6, 10, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1,
		9, 4, 6, 9, 6, 3, 9, 3, 1, 10, 3, 6, -1, -1, -1,
		6, 8, 4, 6, 10, 8, 2, 11, 1, -1, -1, -1, -1, -1, -1,
		3, 2, 10, 0, 6, 1, 0, 4, 6, 6, 11, 1, -1, -1, -1,
		0, 2, 8, 2, 10, 8, 4, 11, 9, 4, 6, 11, -1, -1, -1,
		11, 9, 4, 11, 4, 6, 10, 3, 2, -1, -1, -1, -1, -1, -1,
		8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1,
		0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 8, 0, 1, 9, 2, 2, 9, 4, 2, 4, 6, -1, -1, -1,
		1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1,
		8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 11, 1, -1, -1, -1,
		11, 1, 0, 11, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1,
		4, 6, 11, 4, 11, 9, 0, 3, 8, -1, -1, -1, -1, -1, -1,
		11, 9, 4, 6, 11, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 9, 5, 7, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 3, 4, 9, 5, 10, 7, 6, -1, -1, -1, -1, -1, -1,
		5, 0, 1, 5, 4, 0, 7, 6, 10, -1, -1, -1, -1, -1, -1,
		8, 7, 4, 3, 5, 10, 3, 1, 5, 5, 6, 10, -1, -1, -1,
		9, 5, 4, 11, 1, 2, 7, 6, 10, -1, -1, -1, -1, -1, -1,
		0, 9, 1, 4, 8, 7, 2, 10, 3, 5, 6, 11, -1, -1, -1,
		5, 6, 11, 4, 2, 7, 4, 0, 2, 2, 10, 7, -1, -1, -1,
		3, 2, 10, 7, 4, 8, 11, 5, 6, -1, -1, -1, -1, -1, -1,
		7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1,
		8, 7, 4, 9, 5, 0, 0, 5, 6, 0, 6, 2, -1, -1, -1,
		1, 5, 2, 5, 6, 2, 3, 4, 0, 3, 7, 4, -1, -1, -1,
		6, 2, 1, 6, 1, 5, 4, 8, 7, -1, -1, -1, -1, -1, -1,
		11, 5, 6, 1, 7, 9, 1, 3, 7, 7, 4, 9, -1, -1, -1,
		1, 0, 9, 5, 6, 11, 8, 7, 4, -1, -1, -1, -1, -1, -1,
		4, 0, 3, 4, 3, 7, 6, 11, 5, -1, -1, -1, -1, -1, -1,
		5, 6, 11, 4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		6, 9, 5, 6, 10, 9, 10, 8, 9, -1, -1, -1, -1, -1, -1,
		3, 6, 10, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1,
		0, 10, 8, 0, 5, 10, 0, 1, 5, 5, 6, 10, -1, -1, -1,
		6, 10, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1,
		5, 6, 11, 1, 2, 9, 9, 2, 10, 9, 10, 8, -1, -1, -1,
		0, 9, 1, 2, 10, 3, 5, 6, 11, -1, -1, -1, -1, -1, -1,
		10, 8, 0, 10, 0, 2, 11, 5, 6, -1, -1, -1, -1, -1, -1,
		2, 10, 3, 11, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1,
		9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1,
		1, 5, 6, 1, 6, 2, 3, 8, 0, -1, -1, -1, -1, -1, -1,
		1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 3, 8, 1, 8, 9, 5, 6, 11, -1, -1, -1, -1, -1, -1,
		9, 1, 0, 5, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 3, 8, 5, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		10, 5, 11, 7, 5, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		10, 5, 11, 10, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1,
		5, 10, 7, 5, 11, 10, 1, 9, 0, -1, -1, -1, -1, -1, -1,
		9, 8, 5, 8, 7, 5, 11, 3, 1, 11, 10, 3, -1, -1, -1,
		10, 1, 2, 10, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1,
		2, 10, 3, 0, 8, 1, 1, 8, 7, 1, 7, 5, -1, -1, -1,
		9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 10, 7, -1, -1, -1,
		7, 5, 9, 7, 9, 8, 3, 2, 10, -1, -1, -1, -1, -1, -1,
		2, 5, 11, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1,
		8, 2, 0, 8, 5, 2, 8, 7, 5, 11, 2, 5, -1, -1, -1,
		11, 2, 1, 9, 0, 5, 5, 0, 3, 5, 3, 7, -1, -1, -1,
		9, 8, 7, 9, 7, 5, 11, 2, 1, -1, -1, -1, -1, -1, -1,
		1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1,
		9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1,
		9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		5, 8, 4, 5, 11, 8, 11, 10, 8, -1, -1, -1, -1, -1, -1,
		5, 0, 4, 5, 10, 0, 5, 11, 10, 10, 3, 0, -1, -1, -1,
		4, 5, 9, 0, 1, 8, 8, 1, 11, 8, 11, 10, -1, -1, -1,
		11, 10, 3, 11, 3, 1, 9, 4, 5, -1, -1, -1, -1, -1, -1,
		2, 5, 1, 2, 8, 5, 2, 10, 8, 4, 5, 8, -1, -1, -1,
		0, 4, 5, 0, 5, 1, 2, 10, 3, -1, -1, -1, -1, -1, -1,
		0, 2, 10, 0, 10, 8, 4, 5, 9, -1, -1, -1, -1, -1, -1,
		9, 4, 5, 2, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 5, 11, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1,
		5, 11, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1,
		3, 8, 0, 1, 11, 2, 4, 5, 9, -1, -1, -1, -1, -1, -1,
		1, 11, 2, 9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1,
		0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 4, 5, 0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 10, 7, 4, 9, 10, 9, 11, 10, -1, -1, -1, -1, -1, -1,
		4, 8, 7, 9, 10, 0, 9, 11, 10, 10, 3, 0, -1, -1, -1,
		1, 11, 10, 1, 10, 4, 1, 4, 0, 7, 4, 10, -1, -1, -1,
		3, 1, 11, 3, 11, 10, 7, 4, 8, -1, -1, -1, -1, -1, -1,
		4, 10, 7, 9, 10, 4, 9, 2, 10, 9, 1, 2, -1, -1, -1,
		9, 1, 0, 8, 7, 4, 2, 10, 3, -1, -1, -1, -1, -1, -1,
		10, 7, 4, 10, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1,
		8, 7, 4, 3, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 9, 11, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1,
		9, 11, 2, 9, 2, 0, 8, 7, 4, -1, -1, -1, -1, -1, -1,
		3, 7, 4, 3, 4, 0, 1, 11, 2, -1, -1, -1, -1, -1, -1,
		1, 11, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1,
		0, 9, 1, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 11, 8, 11, 10, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 0, 9, 3, 9, 10, 10, 9, 11, -1, -1, -1, -1, -1, -1,
		0, 1, 11, 0, 11, 8, 8, 11, 10, -1, -1, -1, -1, -1, -1,
		3, 1, 11, 10, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 2, 10, 1, 10, 9, 9, 10, 8, -1, -1, -1, -1, -1, -1,
		1, 0, 9, 2, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 2, 10, 8, 0, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2, 3, 8, 2, 8, 11, 11, 8, 9, -1, -1, -1, -1, -1, -1,
		9, 11, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 3, 8, 1, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};
	*/
	static int edges[256 * 15] = 
	{
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  8,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  8,  3,  9,  8,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  8,  3,  1,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9,  2, 10,  0,  2,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2,  8,  3,  2, 10,  8, 10,  9,  8, -1, -1, -1, -1, -1, -1,
		3, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 11,  2,  8, 11,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  9,  0,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1, 11,  2,  1,  9, 11,  9,  8, 11, -1, -1, -1, -1, -1, -1,
		3, 10,  1, 11, 10,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 10,  1,  0,  8, 10,  8, 11, 10, -1, -1, -1, -1, -1, -1,
		3,  9,  0,  3, 11,  9, 11, 10,  9, -1, -1, -1, -1, -1, -1,
		9,  8, 10, 10,  8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4,  3,  0,  7,  3,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  1,  9,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4,  1,  9,  4,  7,  1,  7,  3,  1, -1, -1, -1, -1, -1, -1,
		1,  2, 10,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3,  4,  7,  3,  0,  4,  1,  2, 10, -1, -1, -1, -1, -1, -1,
		9,  2, 10,  9,  0,  2,  8,  4,  7, -1, -1, -1, -1, -1, -1,
		2, 10,  9,  2,  9,  7,  2,  7,  3,  7,  9,  4, -1, -1, -1,
		8,  4,  7,  3, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11,  4,  7, 11,  2,  4,  2,  0,  4, -1, -1, -1, -1, -1, -1,
		9,  0,  1,  8,  4,  7,  2,  3, 11, -1, -1, -1, -1, -1, -1,
		4,  7, 11,  9,  4, 11,  9, 11,  2,  9,  2,  1, -1, -1, -1,
		3, 10,  1,  3, 11, 10,  7,  8,  4, -1, -1, -1, -1, -1, -1,
		1, 11, 10,  1,  4, 11,  1,  0,  4,  7, 11,  4, -1, -1, -1,
		4,  7,  8,  9,  0, 11,  9, 11, 10, 11,  0,  3, -1, -1, -1,
		4,  7, 11,  4, 11,  9,  9, 11, 10, -1, -1, -1, -1, -1, -1,
		9,  5,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9,  5,  4,  0,  8,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  5,  4,  1,  5,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8,  5,  4,  8,  3,  5,  3,  1,  5, -1, -1, -1, -1, -1, -1,
		1,  2, 10,  9,  5,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3,  0,  8,  1,  2, 10,  4,  9,  5, -1, -1, -1, -1, -1, -1,
		5,  2, 10,  5,  4,  2,  4,  0,  2, -1, -1, -1, -1, -1, -1,
		2, 10,  5,  3,  2,  5,  3,  5,  4,  3,  4,  8, -1, -1, -1,
		9,  5,  4,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 11,  2,  0,  8, 11,  4,  9,  5, -1, -1, -1, -1, -1, -1,
		0,  5,  4,  0,  1,  5,  2,  3, 11, -1, -1, -1, -1, -1, -1,
		2,  1,  5,  2,  5,  8,  2,  8, 11,  4,  8,  5, -1, -1, -1,
		10,  3, 11, 10,  1,  3,  9,  5,  4, -1, -1, -1, -1, -1, -1,
		4,  9,  5,  0,  8,  1,  8, 10,  1,  8, 11, 10, -1, -1, -1,
		5,  4,  0,  5,  0, 11,  5, 11, 10, 11,  0,  3, -1, -1, -1,
		5,  4,  8,  5,  8, 10, 10,  8, 11, -1, -1, -1, -1, -1, -1,
		9,  7,  8,  5,  7,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9,  3,  0,  9,  5,  3,  5,  7,  3, -1, -1, -1, -1, -1, -1,
		0,  7,  8,  0,  1,  7,  1,  5,  7, -1, -1, -1, -1, -1, -1,
		1,  5,  3,  3,  5,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9,  7,  8,  9,  5,  7, 10,  1,  2, -1, -1, -1, -1, -1, -1,
		10,  1,  2,  9,  5,  0,  5,  3,  0,  5,  7,  3, -1, -1, -1,
		8,  0,  2,  8,  2,  5,  8,  5,  7, 10,  5,  2, -1, -1, -1,
		2, 10,  5,  2,  5,  3,  3,  5,  7, -1, -1, -1, -1, -1, -1,
		7,  9,  5,  7,  8,  9,  3, 11,  2, -1, -1, -1, -1, -1, -1,
		9,  5,  7,  9,  7,  2,  9,  2,  0,  2,  7, 11, -1, -1, -1,
		2,  3, 11,  0,  1,  8,  1,  7,  8,  1,  5,  7, -1, -1, -1,
		11,  2,  1, 11,  1,  7,  7,  1,  5, -1, -1, -1, -1, -1, -1,
		9,  5,  8,  8,  5,  7, 10,  1,  3, 10,  3, 11, -1, -1, -1,
		5,  7,  0,  5,  0,  9,  7, 11,  0,  1,  0, 10, 11, 10,  0,
		11, 10,  0, 11,  0,  3, 10,  5,  0,  8,  0,  7,  5,  7,  0,
		11, 10,  5,  7, 11,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		10,  6,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  8,  3,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9,  0,  1,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  8,  3,  1,  9,  8,  5, 10,  6, -1, -1, -1, -1, -1, -1,
		1,  6,  5,  2,  6,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  6,  5,  1,  2,  6,  3,  0,  8, -1, -1, -1, -1, -1, -1,
		9,  6,  5,  9,  0,  6,  0,  2,  6, -1, -1, -1, -1, -1, -1,
		5,  9,  8,  5,  8,  2,  5,  2,  6,  3,  2,  8, -1, -1, -1,
		2,  3, 11, 10,  6,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11,  0,  8, 11,  2,  0, 10,  6,  5, -1, -1, -1, -1, -1, -1,
		0,  1,  9,  2,  3, 11,  5, 10,  6, -1, -1, -1, -1, -1, -1,
		5, 10,  6,  1,  9,  2,  9, 11,  2,  9,  8, 11, -1, -1, -1,
		6,  3, 11,  6,  5,  3,  5,  1,  3, -1, -1, -1, -1, -1, -1,
		0,  8, 11,  0, 11,  5,  0,  5,  1,  5, 11,  6, -1, -1, -1,
		3, 11,  6,  0,  3,  6,  0,  6,  5,  0,  5,  9, -1, -1, -1,
		6,  5,  9,  6,  9, 11, 11,  9,  8, -1, -1, -1, -1, -1, -1,
		5, 10,  6,  4,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4,  3,  0,  4,  7,  3,  6,  5, 10, -1, -1, -1, -1, -1, -1,
		1,  9,  0,  5, 10,  6,  8,  4,  7, -1, -1, -1, -1, -1, -1,
		10,  6,  5,  1,  9,  7,  1,  7,  3,  7,  9,  4, -1, -1, -1,
		6,  1,  2,  6,  5,  1,  4,  7,  8, -1, -1, -1, -1, -1, -1,
		1,  2,  5,  5,  2,  6,  3,  0,  4,  3,  4,  7, -1, -1, -1,
		8,  4,  7,  9,  0,  5,  0,  6,  5,  0,  2,  6, -1, -1, -1,
		7,  3,  9,  7,  9,  4,  3,  2,  9,  5,  9,  6,  2,  6,  9,
		3, 11,  2,  7,  8,  4, 10,  6,  5, -1, -1, -1, -1, -1, -1,
		5, 10,  6,  4,  7,  2,  4,  2,  0,  2,  7, 11, -1, -1, -1,
		0,  1,  9,  4,  7,  8,  2,  3, 11,  5, 10,  6, -1, -1, -1,
		9,  2,  1,  9, 11,  2,  9,  4, 11,  7, 11,  4,  5, 10,  6,
		8,  4,  7,  3, 11,  5,  3,  5,  1,  5, 11,  6, -1, -1, -1,
		5,  1, 11,  5, 11,  6,  1,  0, 11,  7, 11,  4,  0,  4, 11,
		0,  5,  9,  0,  6,  5,  0,  3,  6, 11,  6,  3,  8,  4,  7,
		6,  5,  9,  6,  9, 11,  4,  7,  9,  7, 11,  9, -1, -1, -1,
		10,  4,  9,  6,  4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 10,  6,  4,  9, 10,  0,  8,  3, -1, -1, -1, -1, -1, -1,
		10,  0,  1, 10,  6,  0,  6,  4,  0, -1, -1, -1, -1, -1, -1,
		8,  3,  1,  8,  1,  6,  8,  6,  4,  6,  1, 10, -1, -1, -1,
		1,  4,  9,  1,  2,  4,  2,  6,  4, -1, -1, -1, -1, -1, -1,
		3,  0,  8,  1,  2,  9,  2,  4,  9,  2,  6,  4, -1, -1, -1,
		0,  2,  4,  4,  2,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8,  3,  2,  8,  2,  4,  4,  2,  6, -1, -1, -1, -1, -1, -1,
		10,  4,  9, 10,  6,  4, 11,  2,  3, -1, -1, -1, -1, -1, -1,
		0,  8,  2,  2,  8, 11,  4,  9, 10,  4, 10,  6, -1, -1, -1,
		3, 11,  2,  0,  1,  6,  0,  6,  4,  6,  1, 10, -1, -1, -1,
		6,  4,  1,  6,  1, 10,  4,  8,  1,  2,  1, 11,  8, 11,  1,
		9,  6,  4,  9,  3,  6,  9,  1,  3, 11,  6,  3, -1, -1, -1,
		8, 11,  1,  8,  1,  0, 11,  6,  1,  9,  1,  4,  6,  4,  1,
		3, 11,  6,  3,  6,  0,  0,  6,  4, -1, -1, -1, -1, -1, -1,
		6,  4,  8, 11,  6,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7, 10,  6,  7,  8, 10,  8,  9, 10, -1, -1, -1, -1, -1, -1,
		0,  7,  3,  0, 10,  7,  0,  9, 10,  6,  7, 10, -1, -1, -1,
		10,  6,  7,  1, 10,  7,  1,  7,  8,  1,  8,  0, -1, -1, -1,
		10,  6,  7, 10,  7,  1,  1,  7,  3, -1, -1, -1, -1, -1, -1,
		1,  2,  6,  1,  6,  8,  1,  8,  9,  8,  6,  7, -1, -1, -1,
		2,  6,  9,  2,  9,  1,  6,  7,  9,  0,  9,  3,  7,  3,  9,
		7,  8,  0,  7,  0,  6,  6,  0,  2, -1, -1, -1, -1, -1, -1,
		7,  3,  2,  6,  7,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2,  3, 11, 10,  6,  8, 10,  8,  9,  8,  6,  7, -1, -1, -1,
		2,  0,  7,  2,  7, 11,  0,  9,  7,  6,  7, 10,  9, 10,  7,
		1,  8,  0,  1,  7,  8,  1, 10,  7,  6,  7, 10,  2,  3, 11,
		11,  2,  1, 11,  1,  7, 10,  6,  1,  6,  7,  1, -1, -1, -1,
		8,  9,  6,  8,  6,  7,  9,  1,  6, 11,  6,  3,  1,  3,  6,
		0,  9,  1, 11,  6,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7,  8,  0,  7,  0,  6,  3, 11,  0, 11,  6,  0, -1, -1, -1,
		7, 11,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7,  6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3,  0,  8, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  1,  9, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8,  1,  9,  8,  3,  1, 11,  7,  6, -1, -1, -1, -1, -1, -1,
		10,  1,  2,  6, 11,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  2, 10,  3,  0,  8,  6, 11,  7, -1, -1, -1, -1, -1, -1,
		2,  9,  0,  2, 10,  9,  6, 11,  7, -1, -1, -1, -1, -1, -1,
		6, 11,  7,  2, 10,  3, 10,  8,  3, 10,  9,  8, -1, -1, -1,
		7,  2,  3,  6,  2,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		7,  0,  8,  7,  6,  0,  6,  2,  0, -1, -1, -1, -1, -1, -1,
		2,  7,  6,  2,  3,  7,  0,  1,  9, -1, -1, -1, -1, -1, -1,
		1,  6,  2,  1,  8,  6,  1,  9,  8,  8,  7,  6, -1, -1, -1,
		10,  7,  6, 10,  1,  7,  1,  3,  7, -1, -1, -1, -1, -1, -1,
		10,  7,  6,  1,  7, 10,  1,  8,  7,  1,  0,  8, -1, -1, -1,
		0,  3,  7,  0,  7, 10,  0, 10,  9,  6, 10,  7, -1, -1, -1,
		7,  6, 10,  7, 10,  8,  8, 10,  9, -1, -1, -1, -1, -1, -1,
		6,  8,  4, 11,  8,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3,  6, 11,  3,  0,  6,  0,  4,  6, -1, -1, -1, -1, -1, -1,
		8,  6, 11,  8,  4,  6,  9,  0,  1, -1, -1, -1, -1, -1, -1,
		9,  4,  6,  9,  6,  3,  9,  3,  1, 11,  3,  6, -1, -1, -1,
		6,  8,  4,  6, 11,  8,  2, 10,  1, -1, -1, -1, -1, -1, -1,
		1,  2, 10,  3,  0, 11,  0,  6, 11,  0,  4,  6, -1, -1, -1,
		4, 11,  8,  4,  6, 11,  0,  2,  9,  2, 10,  9, -1, -1, -1,
		10,  9,  3, 10,  3,  2,  9,  4,  3, 11,  3,  6,  4,  6,  3,
		8,  2,  3,  8,  4,  2,  4,  6,  2, -1, -1, -1, -1, -1, -1,
		0,  4,  2,  4,  6,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  9,  0,  2,  3,  4,  2,  4,  6,  4,  3,  8, -1, -1, -1,
		1,  9,  4,  1,  4,  2,  2,  4,  6, -1, -1, -1, -1, -1, -1,
		8,  1,  3,  8,  6,  1,  8,  4,  6,  6, 10,  1, -1, -1, -1,
		10,  1,  0, 10,  0,  6,  6,  0,  4, -1, -1, -1, -1, -1, -1,
		4,  6,  3,  4,  3,  8,  6, 10,  3,  0,  3,  9, 10,  9,  3,
		10,  9,  4,  6, 10,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4,  9,  5,  7,  6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  8,  3,  4,  9,  5, 11,  7,  6, -1, -1, -1, -1, -1, -1,
		5,  0,  1,  5,  4,  0,  7,  6, 11, -1, -1, -1, -1, -1, -1,
		11,  7,  6,  8,  3,  4,  3,  5,  4,  3,  1,  5, -1, -1, -1,
		9,  5,  4, 10,  1,  2,  7,  6, 11, -1, -1, -1, -1, -1, -1,
		6, 11,  7,  1,  2, 10,  0,  8,  3,  4,  9,  5, -1, -1, -1,
		7,  6, 11,  5,  4, 10,  4,  2, 10,  4,  0,  2, -1, -1, -1,
		3,  4,  8,  3,  5,  4,  3,  2,  5, 10,  5,  2, 11,  7,  6,
		7,  2,  3,  7,  6,  2,  5,  4,  9, -1, -1, -1, -1, -1, -1,
		9,  5,  4,  0,  8,  6,  0,  6,  2,  6,  8,  7, -1, -1, -1,
		3,  6,  2,  3,  7,  6,  1,  5,  0,  5,  4,  0, -1, -1, -1,
		6,  2,  8,  6,  8,  7,  2,  1,  8,  4,  8,  5,  1,  5,  8,
		9,  5,  4, 10,  1,  6,  1,  7,  6,  1,  3,  7, -1, -1, -1,
		1,  6, 10,  1,  7,  6,  1,  0,  7,  8,  7,  0,  9,  5,  4,
		4,  0, 10,  4, 10,  5,  0,  3, 10,  6, 10,  7,  3,  7, 10,
		7,  6, 10,  7, 10,  8,  5,  4, 10,  4,  8, 10, -1, -1, -1,
		6,  9,  5,  6, 11,  9, 11,  8,  9, -1, -1, -1, -1, -1, -1,
		3,  6, 11,  0,  6,  3,  0,  5,  6,  0,  9,  5, -1, -1, -1,
		0, 11,  8,  0,  5, 11,  0,  1,  5,  5,  6, 11, -1, -1, -1,
		6, 11,  3,  6,  3,  5,  5,  3,  1, -1, -1, -1, -1, -1, -1,
		1,  2, 10,  9,  5, 11,  9, 11,  8, 11,  5,  6, -1, -1, -1,
		0, 11,  3,  0,  6, 11,  0,  9,  6,  5,  6,  9,  1,  2, 10,
		11,  8,  5, 11,  5,  6,  8,  0,  5, 10,  5,  2,  0,  2,  5,
		6, 11,  3,  6,  3,  5,  2, 10,  3, 10,  5,  3, -1, -1, -1,
		5,  8,  9,  5,  2,  8,  5,  6,  2,  3,  8,  2, -1, -1, -1,
		9,  5,  6,  9,  6,  0,  0,  6,  2, -1, -1, -1, -1, -1, -1,
		1,  5,  8,  1,  8,  0,  5,  6,  8,  3,  8,  2,  6,  2,  8,
		1,  5,  6,  2,  1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  3,  6,  1,  6, 10,  3,  8,  6,  5,  6,  9,  8,  9,  6,
		10,  1,  0, 10,  0,  6,  9,  5,  0,  5,  6,  0, -1, -1, -1,
		0,  3,  8,  5,  6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		10,  5,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11,  5, 10,  7,  5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		11,  5, 10, 11,  7,  5,  8,  3,  0, -1, -1, -1, -1, -1, -1,
		5, 11,  7,  5, 10, 11,  1,  9,  0, -1, -1, -1, -1, -1, -1,
		10,  7,  5, 10, 11,  7,  9,  8,  1,  8,  3,  1, -1, -1, -1,
		11,  1,  2, 11,  7,  1,  7,  5,  1, -1, -1, -1, -1, -1, -1,
		0,  8,  3,  1,  2,  7,  1,  7,  5,  7,  2, 11, -1, -1, -1,
		9,  7,  5,  9,  2,  7,  9,  0,  2,  2, 11,  7, -1, -1, -1,
		7,  5,  2,  7,  2, 11,  5,  9,  2,  3,  2,  8,  9,  8,  2,
		2,  5, 10,  2,  3,  5,  3,  7,  5, -1, -1, -1, -1, -1, -1,
		8,  2,  0,  8,  5,  2,  8,  7,  5, 10,  2,  5, -1, -1, -1,
		9,  0,  1,  5, 10,  3,  5,  3,  7,  3, 10,  2, -1, -1, -1,
		9,  8,  2,  9,  2,  1,  8,  7,  2, 10,  2,  5,  7,  5,  2,
		1,  3,  5,  3,  7,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  8,  7,  0,  7,  1,  1,  7,  5, -1, -1, -1, -1, -1, -1,
		9,  0,  3,  9,  3,  5,  5,  3,  7, -1, -1, -1, -1, -1, -1,
		9,  8,  7,  5,  9,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		5,  8,  4,  5, 10,  8, 10, 11,  8, -1, -1, -1, -1, -1, -1,
		5,  0,  4,  5, 11,  0,  5, 10, 11, 11,  3,  0, -1, -1, -1,
		0,  1,  9,  8,  4, 10,  8, 10, 11, 10,  4,  5, -1, -1, -1,
		10, 11,  4, 10,  4,  5, 11,  3,  4,  9,  4,  1,  3,  1,  4,
		2,  5,  1,  2,  8,  5,  2, 11,  8,  4,  5,  8, -1, -1, -1,
		0,  4, 11,  0, 11,  3,  4,  5, 11,  2, 11,  1,  5,  1, 11,
		0,  2,  5,  0,  5,  9,  2, 11,  5,  4,  5,  8, 11,  8,  5,
		9,  4,  5,  2, 11,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2,  5, 10,  3,  5,  2,  3,  4,  5,  3,  8,  4, -1, -1, -1,
		5, 10,  2,  5,  2,  4,  4,  2,  0, -1, -1, -1, -1, -1, -1,
		3, 10,  2,  3,  5, 10,  3,  8,  5,  4,  5,  8,  0,  1,  9,
		5, 10,  2,  5,  2,  4,  1,  9,  2,  9,  4,  2, -1, -1, -1,
		8,  4,  5,  8,  5,  3,  3,  5,  1, -1, -1, -1, -1, -1, -1,
		0,  4,  5,  1,  0,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		8,  4,  5,  8,  5,  3,  9,  0,  5,  0,  3,  5, -1, -1, -1,
		9,  4,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4, 11,  7,  4,  9, 11,  9, 10, 11, -1, -1, -1, -1, -1, -1,
		0,  8,  3,  4,  9,  7,  9, 11,  7,  9, 10, 11, -1, -1, -1,
		1, 10, 11,  1, 11,  4,  1,  4,  0,  7,  4, 11, -1, -1, -1,
		3,  1,  4,  3,  4,  8,  1, 10,  4,  7,  4, 11, 10, 11,  4,
		4, 11,  7,  9, 11,  4,  9,  2, 11,  9,  1,  2, -1, -1, -1,
		9,  7,  4,  9, 11,  7,  9,  1, 11,  2, 11,  1,  0,  8,  3,
		11,  7,  4, 11,  4,  2,  2,  4,  0, -1, -1, -1, -1, -1, -1,
		11,  7,  4, 11,  4,  2,  8,  3,  4,  3,  2,  4, -1, -1, -1,
		2,  9, 10,  2,  7,  9,  2,  3,  7,  7,  4,  9, -1, -1, -1,
		9, 10,  7,  9,  7,  4, 10,  2,  7,  8,  7,  0,  2,  0,  7,
		3,  7, 10,  3, 10,  2,  7,  4, 10,  1, 10,  0,  4,  0, 10,
		1, 10,  2,  8,  7,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4,  9,  1,  4,  1,  7,  7,  1,  3, -1, -1, -1, -1, -1, -1,
		4,  9,  1,  4,  1,  7,  0,  8,  1,  8,  7,  1, -1, -1, -1,
		4,  0,  3,  7,  4,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		4,  8,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		9, 10,  8, 10, 11,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3,  0,  9,  3,  9, 11, 11,  9, 10, -1, -1, -1, -1, -1, -1,
		0,  1, 10,  0, 10,  8,  8, 10, 11, -1, -1, -1, -1, -1, -1,
		3,  1, 10, 11,  3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  2, 11,  1, 11,  9,  9, 11,  8, -1, -1, -1, -1, -1, -1,
		3,  0,  9,  3,  9, 11,  1,  2,  9,  2, 11,  9, -1, -1, -1,
		0,  2, 11,  8,  0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		3,  2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2,  3,  8,  2,  8, 10, 10,  8,  9, -1, -1, -1, -1, -1, -1,
		9, 10,  2,  0,  9,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		2,  3,  8,  2,  8, 10,  0,  1,  8,  1, 10,  8, -1, -1, -1,
		1, 10,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  3,  8,  9,  1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  9,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  3,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};
	return edges;
}