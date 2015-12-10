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

#define MSAA_RATE	             1
#define LIGHT_INTENSITY     240000.0f
#define LIGHT_ANGLE_FALLOFF      0.8f
#define LIGHT_CUTOFF             0.05f

#define LIGHTS_NB 4

Terrain::Terrain(int argc, const char* argv[])
{
	Log::View::Init();

	window = Window::Create("Geometry Shader babobiboo Test", RES_X, RES_Y, MSAA_RATE, false);
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
	mCamera.mWorld.SetTranslate(v3f(0.f, 0.f, 2.f));//v3f(sceneScale * 0.17f, sceneScale * 0.03f, 0.0f));
	mCamera.mRotation.x = 0;// / 2.0f;
	mCamera.mWorld.SetRotateY(0);//fPI / 2.0f);
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = sceneScale * 0.25f;


	//-------------------------Bonobo version----------------------------------

	//Generate FBO and storing texture
	//bonobo::Texture *rtTestMap = bonobo::loadTexture2D(RES_X, RES_Y, bonobo::TEXTURE_UNORM, v4i(8, 8, 8, 8), MSAA_RATE);
	//glEnable(GL_TEXTURE_3D);

	bonobo::Texture *rtTestTexture3D = bonobo::loadTexture3D(nullptr, DENSITY_RES_X, DENSITY_RES_Y, DENSITY_RES_Z, bonobo::TEXTURE_FLOAT, v4i(32, 0, 0, 0), 0);
	//const bonobo::Texture *gTest[1] = { rtTestTexture3D };
	//bonobo::FBO *texture3DFbo = bonobo::loadFrameBufferObject(gTest, 1);

	//
	// Load all the shader programs used
	std::string densityShaderNames[2] = { SHADERS_PATH("density.vert"), SHADERS_PATH("density.frag") };
	std::string testingShaderNames[3] = { SHADERS_PATH("testing.vert"),	SHADERS_PATH("testing.geo"), SHADERS_PATH("testing.frag") };
	std::string terrainShaderNames[3] = { SHADERS_PATH("terrain.vert"),	SHADERS_PATH("terrain.geo"), SHADERS_PATH("terrain.frag") };
	bonobo::ShaderProgram *densityShader = bonobo::loadShaderProgram(densityShaderNames, 2);
	bonobo::ShaderProgram *testingShader = bonobo::loadShaderProgram(testingShaderNames, 3);
	bonobo::ShaderProgram *terrainShader = bonobo::loadShaderProgram(terrainShaderNames, 3);

	if (densityShader == nullptr) {
		LogError("Failed to load density shader\n");
		exit(-1);
	}
	if (testingShader == nullptr) {
		LogError("Failed to load testing shader\n");
		exit(-1);
	}
	if (terrainShader == nullptr) {
		LogError("Failed to load density shader\n");
		exit(-1);
	}

	// Generate and bind Vertex Buffer Object
	GLuint vbo = 0u;
	
	glGenBuffers(1, &vbo);
	bonobo::checkForErrors();

	GLuint ibo = 0u;
	glGenBuffers(1, &ibo);
	bonobo::checkForErrors();

	GLuint vboT = 0u;
	glGenBuffers(1, &vboT);
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
	float voxelPoints[voxelgridsize*voxelgridsize*voxelgridsize * 3];
	float originPoint[3] = { -1.0f,-1.0f,-1.0f };

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
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	bonobo::checkForErrors();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(indices), indices, GL_STATIC_DRAW);
	bonobo::checkForErrors();
	glBindBuffer(GL_ARRAY_BUFFER, vboT);
	bonobo::checkForErrors();
	glBufferData(GL_ARRAY_BUFFER, voxelgridsize * voxelgridsize *voxelgridsize * 3 * sizeof(float), voxelPoints, GL_STATIC_DRAW);


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

	GLint testingVertexAttrib = glGetAttribLocation(testingShader->mId, "Vertex");
	glEnableVertexAttribArray(testingVertexAttrib);
	bonobo::checkForErrors();
	glVertexAttribPointer(testingVertexAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	bonobo::checkForErrors();

	GLint terrainVertexAttrib = glGetAttribLocation(terrainShader->mId, "Vertex");
	glEnableVertexAttribArray(terrainVertexAttrib);
	bonobo::checkForErrors();
	glVertexAttribPointer(terrainVertexAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	bonobo::checkForErrors();

	// Color Attribute
	GLint colorAttrib = glGetAttribLocation(testingShader->mId, "Color");
	glEnableVertexAttribArray(colorAttrib);
	bonobo::checkForErrors();
	glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	bonobo::checkForErrors();

	glBindVertexArray(0u);
	bonobo::checkForErrors();
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	bonobo::checkForErrors();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
	bonobo::checkForErrors();

	// glEnable(GL_DEPTH_TEST); // If this is enabled, the points don't show up
	glEnable(GL_CULL_FACE);
	//int i = 1;


	//create testure sampler
	bonobo::Sampler *sampler3D = bonobo::loadSampler();

	//create texture layer buffer whibbly whobbly stuffy thingi...
	//for-loop for filling 3d texture
	for (int i = 0; i < 33; i++) {
		//Create layer fbo
		GLuint fbo;
		glGenFramebuffers(1, &fbo);
		bonobo::checkForErrors();
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		bonobo::checkForErrors();
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rtTestTexture3D->mId, 0, i);

		//bind said fbo
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		bonobo::checkForErrors();

		//draw buffer
		glNamedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		bonobo::checkForErrors();

		//test fbo
		int result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (result != GL_FRAMEBUFFER_COMPLETE)
			LogWarning("Failed to bind FBO/RBO");
		bonobo::checkForErrors();



		//Pass 1: Generate Density Function
		//bonobo::setRenderTarget(fbo, 0);
		glUseProgram(densityShader->mId);
		glViewport(0, 0, DENSITY_RES_X, DENSITY_RES_Y);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		bonobo::checkForErrors();
		bonobo::setUniform(*densityShader, "model_to_clip_matrix", mat4f::Identity()/*cast<f32>(mCamera.GetWorldToClipMatrix())*/);

		glBindVertexArray(vao);
		bonobo::checkForErrors();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		bonobo::checkForErrors();

		//GLStateInspection::CaptureSnapshot("Terrain");

		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		bonobo::checkForErrors();
		glBindVertexArray(0u);
		//bonobo::drawFullscreen(*testingShader); // This is not needed! glDrawArrays and ImGUI::Render do the trick :)
		bonobo::checkForErrors();
	}

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

		//glDepthFunc(GL_LESS);


		//printf("Camera Rotation no:1: %f, no:2: %f\n", (mCamera.mRotation).x, (mCamera.mRotation).y);


		//Pass 2: Stuff
		bonobo::setRenderTarget(0, 0);
		glUseProgram(terrainShader->mId);
		glViewport(0, 0, RES_X, RES_Y);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		bonobo::checkForErrors();
		bonobo::setUniform(*terrainShader, "model_to_clip_matrix", cast<f32>(mCamera.GetWorldToClipMatrix()));

		bonobo::bindTextureSampler(*terrainShader, "Density_texture", 0, *rtTestTexture3D, *sampler3D);

		glBindVertexArray(vao);
		bonobo::checkForErrors();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		bonobo::checkForErrors();

		//GLStateInspection::CaptureSnapshot("Testing");

		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		bonobo::checkForErrors();
		glBindVertexArray(0u);
		//bonobo::drawFullscreen(*testingShader); // This is not needed! glDrawArrays and ImGUI::Render do the trick :)
		bonobo::checkForErrors();


		//GLStateInspection::CaptureSnapshot("Resolve Pass");
		bonobo::checkForErrors();


		//output stuff
		//gSimpleDraw.Texture(v2f(-0.95f, -0.95f), v2f(-0.55f, -0.55f), *rtTestTexture3D, v4i(0, 1, 2, -1));

		GLStateInspection::View::Render(); // Disabling this turns off the GLStateInspection console within the render window
										   //Log::View::Render();
		ImGui::Render();

		window->Swap();
		lastTime = nowTime;
	}

	glDeleteBuffers(1, &vbo);
	bonobo::checkForErrors();
	vbo = 0u;
	glDeleteBuffers(1, &ibo);
	bonobo::checkForErrors();
	ibo = 0u;


	glDeleteVertexArrays(1, &vao);
	bonobo::checkForErrors();
	vao = 0u;
}
