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
#include "Testing.h"
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


#define RES_X           1600
#define RES_Y            900
#define SHADOWMAP_RES_X  512
#define SHADOWMAP_RES_Y  512

#define MSAA_RATE	             1
#define LIGHT_INTENSITY     240000.0f
#define LIGHT_ANGLE_FALLOFF      0.8f
#define LIGHT_CUTOFF             0.05f

#define LIGHTS_NB 4


static GLuint loadCone(GLuint& vboId, GLsizei& verticesNb, bonobo::ShaderProgram* shader);



Testing::Testing(int argc, const char* argv[])
{
	Log::View::Init();

	window = Window::Create("Awesome Super Duper Mega Terrain Generator 9001", RES_X, RES_Y, MSAA_RATE, false);
	inputHandler = new InputHandler();
	window->SetInputHandler(inputHandler);

	SimpleDraw::Init();
	GLStateInspection::Init();
	GLStateInspection::View::Init();
}

Testing::~Testing()
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

void Testing::run()
{
	//
	// Load the scene coming
	//
	


	//
	// Compute the scene bounding box to scale the camera speed, near and far plane accordingly
	//
	
	float sceneScale = 10.0;

	//
	// Setup the camera
	//
	FPSCameraf mCamera = FPSCameraf(fPI / 4.0f, static_cast<float>(RES_X) / static_cast<float>(RES_Y), sceneScale * 0.01f, sceneScale * 4.0f);
	mCamera.mWorld.SetTranslate(v3f(sceneScale * 0.17f, sceneScale * 0.03f, 0.0f));
	mCamera.mRotation.x = fPI / 2.0f;
	mCamera.mWorld.SetRotateY(fPI / 2.0f);
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = sceneScale * 0.25f;


	//
	// Load all the shader programs used

	std::string defaultShaderNames[2] = { SHADERS_PATH("default.vert"),           SHADERS_PATH("default.frag") };
	std::string terrainShaderNames[2]		= { SHADERS_PATH("terrain_gen.vert"),		SHADERS_PATH("terrain_gen.frag") };

	bonobo::ShaderProgram *defaultShader         = bonobo::loadShaderProgram(defaultShaderNames,   2);

	bonobo::ShaderProgram *terrainShader		 = nullptr;

	if (defaultShader == nullptr) {
		LogError("Failed to load default shader\n");
		exit(-1);
	}

	auto reloadShaders = [&](){
	
		if (terrainShader != nullptr && terrainShader != defaultShader) {
			bonobo::unloadShader(&terrainShader->mShaders[0]);
			bonobo::unloadShader(&terrainShader->mShaders[1]);
			bonobo::unloadShaderProgram(terrainShader);
		}

		terrainShader		  = bonobo::loadShaderProgram(terrainShaderNames,   2);

		if (terrainShader == nullptr)
			terrainShader = defaultShader;

	};
	reloadShaders();


	//
	// Setup VAOs, VBOs and IBOs for the scene and cone meshes
	//
	GLuint vbo;
	glGenBuffers(1, &vbo);

	float points[] = {
		-0.45f,  0.45f, 0.0f,
		0.45f,  0.45f, 0.0f,
		0.45f, -0.45f, 0.0f,
		-0.45f, -0.45f, 0.0f
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);


	//
	// Setup FBOs
	//
	bonobo::Texture *testTex = bonobo::loadTexture2D(RES_X, RES_Y, bonobo::TEXTURE_UNORM, v4i(8, 8, 8, 8), MSAA_RATE);

	const bonobo::Texture *gbufferColorAttachments[1] = {testTex};

	bonobo::FBO *testingFbo = bonobo::loadFrameBufferObject(gbufferColorAttachments);

	//
	// Setup texture samplers
	//
	
	

	GLfloat borderColor[4] = { 1.0f, 0.0, 0.0f, 0.0f };

	


	//
	// Load the default texture
	//
	


	//
	// Setup lights properties
	//
	

	//
	// Setup some additionnal parameters
	//
	

	//
	// Set initial OpenGL state
	//


	//Own stuff
	//Create Vertax array
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Specify layout of point data
	GLint posAttrib = glGetAttribLocation(terrainShader->mId, "pos");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//end of own stuff


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

		if ((inputHandler->GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED)) {
			LogInfo("Reloading all shaders\n");
			reloadShaders();
		}

		ImGui_ImplGlfwGL3_NewFrame();



		glDepthFunc(GL_LESS);

		//Own implenetation, Render directly to screen

		bonobo::setRenderTarget(testingFbo, 0);
		glUseProgram(terrainShader->mId);
		glViewport(0, 0, RES_X, RES_Y);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		bonobo::checkForErrors();
		bonobo::setUniform(*terrainShader, "model_to_clip_matrix", cast<f32>(mCamera.GetWorldToClipMatrix()));
		bonobo::setUniform(*terrainShader, "model_to_world_matrix", mat4f::Identity());
		
		

		//glBindVertexArray(vao->mId);
		bonobo::drawFullscreen(*terrainShader);
		
		//
		// Pass 1: Render scene into the g-buffer
		//


	
	
		
	
		
		
			//
			// Pass 2.2: Accumulate light i contribution
			
		//
		// Pass 3: Compute final image using both the g-buffer and  the light accumulation buffer
		//
		
		GLStateInspection::CaptureSnapshot("Resolve Pass");




		//
		// Pass 4: Draw wireframe cones on top of the final image for debugging purposes
		//
		//glUseProgram(shadowMapShader->mId);;
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//for (uint32_t i = 0u; i < LIGHTS_NB; ++i) {
		//	bonobo::setUniform(*shadowMapShader, "model_to_clip_matrix", mCamera.GetWorldToClipMatrix() * lightTransforms[i].GetMatrix() * lightOffsetTransform.GetMatrix() * coneScaleTransform.GetMatrix());

		//	glBindVertexArray(coneVao);
		//	bonobo::checkForErrors();

		//	glDrawArrays(GL_TRIANGLE_STRIP, 0, coneVerticesNb);

		//	glBindVertexArray(0u);
		//	bonobo::checkForErrors();
		//}
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		//
		// Output content of the g-buffer as well as of the shadowmap, for debugging purposes
		//
		

		bonobo::checkForErrors();

		GLStateInspection::View::Render();
		//Log::View::Render();
		ImGui::Render();

		window->Swap();
		lastTime = nowTime;
	}

	
	glDeleteVertexArrays(1, &vao);
	bonobo::checkForErrors();
	vao = 0u;
}



static GLuint loadCone(GLuint& vboId, GLsizei& verticesNb, bonobo::ShaderProgram* shader)
{
	verticesNb = 65;
	float vertexArrayData[65 * 3] = {
		0.f, 1.f, -1.f,
		0.f, 0.f, 0.f,
		0.38268f, 0.92388f, -1.f,
		0.f, 0.f, 0.f,
		0.70711f, 0.70711f, -1.f,
		0.f, 0.f, 0.f,
		0.92388f, 0.38268f, -1.f,
		0.f, 0.f, 0.f,
		1.f, 0.f, -1.f,
		0.f, 0.f, 0.f,
		0.92388f, -0.38268f, -1.f,
		0.f, 0.f, 0.f,
		0.70711f, -0.70711f, -1.f,
		0.f, 0.f, 0.f,
		0.38268f, -0.92388f, -1.f,
		0.f, 0.f, 0.f,
		0.f, -1.f, -1.f,
		0.f, 0.f, 0.f,
		-0.38268f, -0.92388f, -1.f,
		0.f, 0.f, 0.f,
		-0.70711f, -0.70711f, -1.f,
		0.f, 0.f, 0.f,
		-0.92388f, -0.38268f, -1.f,
		0.f, 0.f, 0.f,
		-1.f, 0.f, -1.f,
		0.f, 0.f, 0.f,
		-0.92388f, 0.38268f, -1.f,
		0.f, 0.f, 0.f,
		-0.70711f, 0.70711f, -1.f,
		0.f, 0.f, 0.f,
		-0.38268f, 0.92388f, -1.f,
		0.f, 1.f, -1.f,
		0.f, 1.f, -1.f,
		0.38268f, 0.92388f, -1.f,
		0.f, 1.f, -1.f,
		0.70711f, 0.70711f, -1.f,
		0.f, 0.f, -1.f,
		0.92388f, 0.38268f, -1.f,
		0.f, 0.f, -1.f,
		1.f, 0.f, -1.f,
		0.f, 0.f, -1.f,
		0.92388f, -0.38268f, -1.f,
		0.f, 0.f, -1.f,
		0.70711f, -0.70711f, -1.f,
		0.f, 0.f, -1.f,
		0.38268f, -0.92388f, -1.f,
		0.f, 0.f, -1.f,
		0.f, -1.f, -1.f,
		0.f, 0.f, -1.f,
		-0.38268f, -0.92388f, -1.f,
		0.f, 0.f, -1.f,
		-0.70711f, -0.70711f, -1.f,
		0.f, 0.f, -1.f,
		-0.92388f, -0.38268f, -1.f,
		0.f, 0.f, -1.f,
		-1.f, 0.f, -1.f,
		0.f, 0.f, -1.f,
		-0.92388f, 0.38268f, -1.f,
		0.f, 0.f, -1.f,
		-0.70711f, 0.70711f, -1.f,
		0.f, 0.f, -1.f,
		-0.38268f, 0.92388f, -1.f,
		0.f, 0.f, -1.f,
		0.f, 1.f, -1.f,
		0.f, 0.f, -1.f
	};

	GLuint vaoId = 0u;
	GLint loc = glGetAttribLocation(shader->mId, "Vertex");
	bonobo::checkForErrors();
	glGenVertexArrays(1, &vaoId);
	bonobo::checkForErrors();
	glBindVertexArray(vaoId);
	bonobo::checkForErrors();
	{
		glGenBuffers(1, &vboId);
		bonobo::checkForErrors();
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		bonobo::checkForErrors();
		glBufferData(GL_ARRAY_BUFFER, verticesNb * 3 * sizeof(float), vertexArrayData, GL_STATIC_DRAW);
		bonobo::checkForErrors();

		glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		bonobo::checkForErrors();
		glEnableVertexAttribArray(loc);
		bonobo::checkForErrors();
	}
	glBindVertexArray(0u);
	bonobo::checkForErrors();
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	bonobo::checkForErrors();

	return vaoId;
}
