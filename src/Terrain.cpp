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
//#include <../externals/Simple OpenGL Image Library/src/SOIL.h>

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

#define LIGHTS_NB 


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

	FPSCameraf mCamera = FPSCameraf(fPI / 4.0f, static_cast<float>(RES_X) / static_cast<float>(RES_Y), sceneScale * 0.01f, sceneScale * 50.0f);
	mCamera.mWorld.SetTranslate(v3f(51.3, 18.2, 15.0));//v3f(16.f, 20.f, 33.f));//v3f(sceneScale * 0.17f, sceneScale * 0.03f, 0.0f));
	mCamera.mRotation.x = fPI/2;// / 2.0f;
	mCamera.mRotation.y = -fPI / 10;
	
	mCamera.mWorld.PreRotateY(fPI/2);//fPI / 2.0f);
	mCamera.mWorld.PreRotateX(-fPI / 10);
	

	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = sceneScale * 2.f;

	bonobo::Texture *rtTestTexture3D = bonobo::loadTexture3D(nullptr, DENSITY_RES_X, DENSITY_RES_Y, DENSITY_RES_Z, bonobo::TEXTURE_FLOAT, v4i(32, 0, 0, 0), 0);
	bonobo::Texture *edgeTexture = new bonobo::Texture();
	bonobo::Texture *densityTexture3D = new bonobo::Texture();
	bonobo::Texture *cloudTexture = bonobo::loadTexture2D(RESOURCES_PATH("Clouds_diff.png"));
	bonobo::Texture *BumpMapTexture = bonobo::loadTexture2D(RESOURCES_PATH("waves.png"));
	//bonobo::Texture *skyBox = new bonobo::Texture();




	//
	// Load all the shader programs used
	//
	std::string densityShaderNames[2] = { SHADERS_PATH("density.vert"), SHADERS_PATH("density.frag") };
	std::string terrainShaderNames[3] = { SHADERS_PATH("terrain.vert"),	SHADERS_PATH("terrain.geo"), SHADERS_PATH("terrain.frag") };
	std::string oceanShaderNames[2] = { SHADERS_PATH("glslWater.vert"), SHADERS_PATH("glslWater.frag") };
	bonobo::ShaderProgram *densityShader = bonobo::loadShaderProgram(densityShaderNames, 2);
	bonobo::ShaderProgram *terrainShader = bonobo::loadShaderProgram(terrainShaderNames, 3);
	bonobo::ShaderProgram *oceanShader =   bonobo::loadShaderProgram(oceanShaderNames, 2);

	if (densityShader == nullptr) {
		LogError("Failed to load density shader\n");
		exit(-1);
	}
	if (terrainShader == nullptr) {
		LogError("Failed to load density shader\n");
		exit(-1);
	}
	if (oceanShader == nullptr) {
		LogError("Failed to load density shader\n");
		exit(-1);
	}


	//Create CubeMap, probably not working, but hey; no errors!
	/*glGenTextures(1, &skyBox->mId);
	bonobo::checkForErrors();
	glBindTexture(GL_TEXTURE_CUBE_MAP,skyBox->mId);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	int width=512, height=512;
	unsigned char* image =
		SOIL_load_image("cloudyhills_posx.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X,0,GL_RGB,width,height,0, GL_RGB, GL_UNSIGNED_INT,image);
	
	bonobo::checkForErrors();
	SOIL_free_image_data(image);
	image= SOIL_load_image("cloudyhills_negx.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_INT, image);


	bonobo::checkForErrors();
	SOIL_free_image_data(image);
	image=SOIL_load_image("cloudyhills_posy.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_INT, image);
	

	bonobo::checkForErrors();
	SOIL_free_image_data(image);
	image=SOIL_load_image("cloudyhills_negy.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_INT, image);

	bonobo::checkForErrors();
	SOIL_free_image_data(image);
	image=SOIL_load_image("cloudyhills_posz.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_INT, image);

	bonobo::checkForErrors();
	
	SOIL_free_image_data(image);
	image=SOIL_load_image("cloudyhills_negz.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_INT, image);

	bonobo::checkForErrors();
	
	skyBox->mTarget = bonobo::TEXTURE_CUBE_MAP;
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	SOIL_free_image_data(image);*/
	

	//Create Mountain
	const int EDGES_SIZE = 256 * 15;
	int *edges = createLookupTable();
	
	glGenTextures(1, &edgeTexture->mId);
	bonobo::checkForErrors();
	glBindTexture(GL_TEXTURE_1D, edgeTexture->mId);
	bonobo::checkForErrors();
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32I, EDGES_SIZE, 0, GL_RED_INTEGER, GL_INT, edges);
	bonobo::checkForErrors();
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, 0);
	edgeTexture->mTarget = bonobo::TEXTURE_1D;
	glBindTexture(GL_TEXTURE_1D, 0);

	
	// Create density function
	//float densityFunction3D[DENSITY_SIZE][DENSITY_SIZE][DENSITY_SIZE];
	//for (int x = 0; x < DENSITY_SIZE; x++)
	//{
	//	for (int y = 0; y < DENSITY_SIZE; y++)
	//	{
	//		for (int z = 0; z < DENSITY_SIZE; z++)
	//		{
	//			densityFunction3D[x][y][z] = y - 7.f + 0.6f*cos(x*rand() / (10.0f + rand() % 3) + rand()) - 0.5f*sin(z / (30.f + rand() % 10) + rand() % 2) + 1 / (rand() % 4 + 1);
	//			if ( fabs(16.0f+rand()%10-x)*fabs(16.0f + rand() % 10 - x) +fabs(15.0f+rand()% 7-z)*fabs(15.0f + rand() % 7 - z) <  30.f-0.9*std::max(30.f- (float) y,0.0f))
	//			{
	//				densityFunction3D[x][y][z] = -1;//-=// (10  + rand() % 4);
	//			}
	//			if (fabs(16.0f - z)*fabs(16.0f - z) + fabs(17.0f  - y)*fabs(17.0f   - y) <  (5*5  ))
	//			{
	//				densityFunction3D[x][y][z] = 1;
	//			}

	//			else if (fabs(15.0f - z)*fabs(15.0f - z) + fabs(15.0f - x)*fabs(15.0f - x) >  (15.0f+rand()%2)*(15.0f + rand() % 2))
	//			{
	//				densityFunction3D[x][y][z] = 1;// += (10 + rand() % 5);
	//			}
	//		}
	//	}
	//}
	// Create density function
	float densityFunction3D[DENSITY_SIZE][DENSITY_SIZE][DENSITY_SIZE];
	for (int x = 0; x < DENSITY_SIZE; x++)
	{
		for (int y = 0; y < DENSITY_SIZE; y++)
		{
			for (int z = 0; z < DENSITY_SIZE; z++)
			{
				// Create terrain
				densityFunction3D[x][y][z] = y - 7.f + 0.6f*cos(x*rand() / (10.0f + rand() % 3) + rand()) - 0.5f*sin(z / (30.f + rand() % 10) + rand() % 2) + 1 / (rand() % 4 + 1);

				// Create mountain
				if (fabs(18.0f + rand() % 10 - x)*fabs(18.0f + rand() % 10 - x) + fabs(15.0f + rand() % 7 - z)*fabs(15.0f + rand() % 7 - z) < 35 && y < 20 - rand() % 3)//20.f-0.9*std::max(30.f- (float) y,0.0f))
				{
					densityFunction3D[x][y][z] = -1;
				}

				// Create bay
				if (fabs(10.0f + rand() % 10 - x)*fabs(10.0f + rand() % 10 - x) + fabs(25.0f + rand() % 7 - z)*fabs(25.0f + rand() % 7 - z) < 35)
				{
					densityFunction3D[x][y][z] = 1;
				}

				// Round off terrain
				if (fabs(15.0f - z)*fabs(15.0f - z) + fabs(15.0f - x)*fabs(15.0f - x) >  (13.0f + rand() % 2)*(15.0f + rand() % 2))
				{
					densityFunction3D[x][y][z] = 1;// += (10 + rand() % 5);
				}
			}
		}
	}

	
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



	GLuint vboTerrain = 0u;
	glGenBuffers(1, &vboTerrain);
	bonobo::checkForErrors();


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

	GLint terrainVertexAttrib = glGetAttribLocation(terrainShader->mId, "Vertex");
	glEnableVertexAttribArray(terrainVertexAttrib);
	bonobo::checkForErrors();
	glVertexAttribPointer(terrainVertexAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	bonobo::checkForErrors();

	glBindVertexArray(0u);
	bonobo::checkForErrors();
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	bonobo::checkForErrors();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
	bonobo::checkForErrors();
	

	
	
	
	
	
	//
	//Ocean test:
	//
	// constants
	const int QUAD_GRID_SIZE = 150;
	const int NR_VERTICES = (QUAD_GRID_SIZE + 1)*(QUAD_GRID_SIZE + 1);
	const int NR_TRIANGLES = 2 * QUAD_GRID_SIZE*QUAD_GRID_SIZE;
	const int NR_INDICES = 3 * NR_TRIANGLES;
	

	// Variables
	GLuint gDrawNumber = NR_INDICES; // How many indices to draw 

	//GLuint gShaderProgramID; // ID for shader program	
	GLuint gVaoID=0u;			 // ID for vertex array object
	GLuint gVboID=0u;			 // ID for vertex array object
	GLuint gIndexID=0u;		 // ID for vertex array object

							 //Generate grid positions
	const float scale = 250.0f;
	const float delta = 2.0f / QUAD_GRID_SIZE;
	
	vec3f* vertices = new vec3f[NR_VERTICES];
	vec2f* texcoords = new vec2f[NR_VERTICES];
	GLuint* indices = new GLuint[3 * NR_TRIANGLES];
	
	for (int y = 0; y <= QUAD_GRID_SIZE; y++) {
		for (int x = 0; x <= QUAD_GRID_SIZE; x++) {
			int vertexPosition = y*(QUAD_GRID_SIZE + 1) + x;
			vertices[vertexPosition].x = (x*delta - 1.0) * scale;
			vertices[vertexPosition].y = 0.0f;
			vertices[vertexPosition].z = (y*delta - 1.0) * scale;
			texcoords[vertexPosition].x = x*delta;
			texcoords[vertexPosition].y = y*delta;
		}
	}
	
	// Generate indices into vertex list
	for (int y = 0; y < QUAD_GRID_SIZE; y++) {
		for (int x = 0; x < QUAD_GRID_SIZE; x++) {
			int indexPosition = y*QUAD_GRID_SIZE + x;
			// tri 0
			indices[6 * indexPosition] = y    * (QUAD_GRID_SIZE + 1) + x;    //bl  			
			indices[6 * indexPosition + 2] = y    *(QUAD_GRID_SIZE + 1) + x + 1;//br
			indices[6 * indexPosition + 1] = (y + 1)*(QUAD_GRID_SIZE + 1) + x + 1;//tr
			
			
																				// tri 1
			indices[6 * indexPosition + 3] = y    *(QUAD_GRID_SIZE + 1) + x;    //bl
			indices[6 * indexPosition + 5] = (y + 1)*(QUAD_GRID_SIZE + 1) + x + 1;//tr
			indices[6 * indexPosition + 4] = (y + 1)*(QUAD_GRID_SIZE + 1) + x;    //tl
			//printf("%d\n%d\n%d\n%d\n%d\n%d\n", indices[6 * indexPosition], indices[6 * indexPosition+1],indices[6 * indexPosition+2], indices[6 * indexPosition+3], indices[6 * indexPosition+4], indices[6 * indexPosition+5]);
		}
	}
	
	//Put all values in one long array, harhar
	const int allAttribSize = NR_VERTICES;
	float Allattrib[allAttribSize*5];
	for (int i = 0; i < allAttribSize; i++) {
		Allattrib[5 * i] = vertices[i].x;
		Allattrib[5 * i+1] = vertices[i].y;
		Allattrib[5 * i+2] = vertices[i].z;
		Allattrib[5 * i+3] = texcoords[i].x;
		Allattrib[5 * i+4] = texcoords[i].y;
	}

	delete[] vertices;
	delete[] texcoords;

	// Create a vertex array object
	glGenVertexArrays(1, &gVaoID);
	bonobo::checkForErrors();
	glBindVertexArray(gVaoID);
	bonobo::checkForErrors();

	// Create and initialize a buffer object
	glGenBuffers(1, &gVboID);
	bonobo::checkForErrors();
	glBindBuffer(GL_ARRAY_BUFFER, gVboID);
	bonobo::checkForErrors();
	glBufferData(GL_ARRAY_BUFFER, 5*allAttribSize*sizeof(float), Allattrib, GL_STATIC_DRAW);
	bonobo::checkForErrors();


	
	

	// Bind the index buffer
	glGenBuffers(1, &gIndexID);
	bonobo::checkForErrors();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexID);
	bonobo::checkForErrors();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * NR_TRIANGLES*sizeof(unsigned int), indices, GL_STATIC_DRAW);
	bonobo::checkForErrors();
	

	// Initialize the vertex position attribute from the vertex shader
	GLuint posOcean = glGetAttribLocation(oceanShader->mId, "vPosition");	
	bonobo::checkForErrors();
	glVertexAttribPointer(posOcean, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),0);
	bonobo::checkForErrors();
	glEnableVertexAttribArray(posOcean);
	bonobo::checkForErrors();

	GLuint texOcean = glGetAttribLocation(oceanShader->mId, "vTexCoord");
	bonobo::checkForErrors();
	//printf("%d\n", texOcean);
	glEnableVertexAttribArray(texOcean);
	bonobo::checkForErrors();
	glVertexAttribPointer(texOcean, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	bonobo::checkForErrors();

	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	bonobo::checkForErrors();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
	bonobo::checkForErrors();

	glBindVertexArray(0u);
	bonobo::checkForErrors();
	

	delete[] indices;
	

	
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	//create testure sampler
	bonobo::Sampler *sampler = bonobo::loadSampler();
	bonobo::Sampler *oceanSampler = bonobo::loadSampler();

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


		//
		//Pass 2: Generate terrain
		//
		bonobo::setRenderTarget(0, 0);
		glUseProgram(terrainShader->mId);
		glViewport(0, 0, RES_X, RES_Y);
		glClearColor(0.7, 0.8, 0.9, 1.0f);//glClearColor(0.53f, 0.8f, 0.98f, 1.0f);
		
		glClearDepthf(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		bonobo::checkForErrors();
		glBindTexture(GL_TEXTURE_3D, densityTexture3D->mId);
		glBindTexture(GL_TEXTURE_1D, edgeTexture->mId);
		bonobo::setUniform(*terrainShader, "model_to_clip_matrix", cast<f32>(mCamera.GetWorldToClipMatrix()));
		bonobo::bindTextureSampler(*terrainShader, "density_texture", 0, *densityTexture3D, *sampler);
		bonobo::bindTextureSampler(*terrainShader, "edge_texture", 1, *edgeTexture, *sampler);
		bonobo::bindTextureSampler(*terrainShader, "clouds", 2, *cloudTexture, *sampler);
		//Probably better way of doing this, but tired!!
		bonobo::setUniform(*terrainShader, "origin_x", originPoint[0]);
		bonobo::setUniform(*terrainShader, "origin_y", originPoint[1]);
		bonobo::setUniform(*terrainShader, "origin_z", originPoint[2]);
		bonobo::setUniform(*terrainShader, "ResY", (float) RES_Y);
		bonobo::setUniform(*terrainShader, "ResX", (float) RES_X);

		glBindVertexArray(vao);
		bonobo::checkForErrors();

		glBindBuffer(GL_ARRAY_BUFFER, vboTerrain);
		bonobo::checkForErrors();
		glEnableVertexAttribArray(0);
		bonobo::checkForErrors();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		bonobo::checkForErrors();

		GLStateInspection::CaptureSnapshot("Terrain");
		
	//	glDrawArrays(GL_TRIANGLES, 0, 3);
		
		glDrawArrays(GL_POINTS, 0, nbr_voxelPoints);
		bonobo::checkForErrors();

		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindSampler(2u, 0u);
		bonobo::checkForErrors();
		glBindSampler(1u, 0u);
		bonobo::checkForErrors();
		glBindSampler(0u, 0u);
		bonobo::checkForErrors();
		glBindTexture(GL_TEXTURE_3D, 0);
		glBindTexture(GL_TEXTURE_1D, 0);
		glBindVertexArray(0u);
		bonobo::checkForErrors();

		GLStateInspection::CaptureSnapshot("Terrain");
		bonobo::checkForErrors();


		//Render Ocean
		bonobo::setRenderTarget(0, 0);
		bonobo::checkForErrors();
		glUseProgram(oceanShader->mId);
		bonobo::checkForErrors();
		glBindTexture(GL_TEXTURE_2D, BumpMapTexture->mId);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox->mId);
		bonobo::setUniform(*oceanShader, "ModelViewProjectionMatrix", cast<f32>(mCamera.GetWorldToClipMatrix()));
		bonobo::setUniform(*oceanShader, "vCameraPos", mCamera.mWorld.GetTranslation());
		bonobo::setUniform(*oceanShader, "time", float(nowTime));
		bonobo::bindTextureSampler(*oceanShader, "BumpMapTexture", 0, *BumpMapTexture, *oceanSampler);
		//bonobo::bindTextureSampler(*oceanShader, "SkyboxTexture", 1, *skyBox, *oceanSampler);

		//auto match = oceanShader.mUniformLocs.find(oceanSampler);
		//glActiveTexture(GL_TEXTURE0 + 1);
		//auto match = oceanShader.mUniformLocs.find(oceanSampler);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox->mId);
		//glUniform1i(match->second.mLoc, 1);
		//glBindSampler(1, (*oceanSampler).mId);
		bonobo::checkForErrors();



		bonobo::checkForErrors();

		glBindVertexArray(gVaoID);
		bonobo::checkForErrors();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexID);
		bonobo::checkForErrors();

		glDrawElements(GL_TRIANGLES, 3 * NR_TRIANGLES, GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		bonobo::checkForErrors();
		glBindSampler(1u, 0u);
		bonobo::checkForErrors();
		glBindSampler(0u, 0u);
		bonobo::checkForErrors();
		glBindTexture(GL_TEXTURE_2D, 0);
		bonobo::checkForErrors();
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		bonobo::checkForErrors();
		glBindVertexArray(0u);
		bonobo::checkForErrors();
		//printf("CamPos: x: %f, y: %f, z: %f\n Rot: x: %f, y: %f\n\n", mCamera.mWorld.GetTranslation().x, mCamera.mWorld.GetTranslation().y, mCamera.mWorld.GetTranslation().z, mCamera.mRotation.x, mCamera.mRotation.y);

		// Render window
		GLStateInspection::View::Render(); // Disabling this turns off the GLStateInspection console within the render window
		//Log::View::Render();
		ImGui::Render();

		window->Swap();
		lastTime = nowTime;
	}

	glDeleteBuffers(1, &vboTerrain);
	bonobo::checkForErrors();
	vboTerrain = 0u;

	glDeleteBuffers(1, &gIndexID);
	bonobo::checkForErrors();
	gIndexID = 0u;
	glDeleteBuffers(1, &gVboID);
	bonobo::checkForErrors();
	gVboID = 0u;
	glDeleteVertexArrays(1, &gVaoID);
	bonobo::checkForErrors();
	gVaoID = 0u;


	glDeleteVertexArrays(1, &vao);
	bonobo::checkForErrors();
	vao = 0u;

}

int * Terrain::createLookupTable() 
{
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