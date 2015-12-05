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

Testing::Testing(int argc, const char* argv[])
{
	Log::View::Init();

	window = Window::Create("Geometry Shader Test", RES_X, RES_Y, MSAA_RATE, false);
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
	// Compute the scene bounding box to scale the camera speed, near and far plane accordingly
	//
	
	float sceneScale = 10.0;

	//
	// Setup the camera. 
	//
	// NOTE: If the points don't show up at first, just press the left mouse button and they should appear. The camera orientation shouldn't need to be changed
	// 
	FPSCameraf mCamera = FPSCameraf(fPI / 4.0f, static_cast<float>(RES_X) / static_cast<float>(RES_Y), sceneScale * 0.01f, sceneScale * 4.0f);
	mCamera.mWorld.SetTranslate(v3f(0.f, 0.f, -2.f));//v3f(sceneScale * 0.17f, sceneScale * 0.03f, 0.0f));
	mCamera.mRotation.x = fPI;// / 2.0f;
	mCamera.mWorld.SetRotateY(0);//fPI / 2.0f);
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = sceneScale * 0.25f;


	//
	// Load all the shader programs used
	std::string testingShaderNames[3] = { SHADERS_PATH("testing.vert"),	SHADERS_PATH("testing.geo"), SHADERS_PATH("testing.frag") };
	bonobo::ShaderProgram *testingShader = bonobo::loadShaderProgram(testingShaderNames, 3);

	if (testingShader == nullptr) {
		LogError("Failed to load testing shader\n");
		exit(-1);
	}

	float points[4*3] = {
		-0.45f,  0.45f, 0.0f,
		0.45f,  0.45f, 0.0f,
		0.45f, -0.45f, 0.0f,
		-0.45f, -0.45f, 0.0f
	};

	//Generate and Bind Vertex Array Object
	GLuint vao = 0u;;
	glGenVertexArrays(1, &vao);
	bonobo::checkForErrors();
	glBindVertexArray(vao);
	bonobo::checkForErrors();
	
	//Generate and bind Vertex Buffer Object
	GLuint vbo = 0u;
	GLint posAttrib = glGetAttribLocation(testingShader->mId, "Vertex");
	glGenBuffers(1, &vbo);
	bonobo::checkForErrors();

	// Specify layout of point data
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	bonobo::checkForErrors();
	glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(float), points, GL_STATIC_DRAW);
	bonobo::checkForErrors();
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	bonobo::checkForErrors();
	glEnableVertexAttribArray(posAttrib);
	bonobo::checkForErrors();
	
	glBindVertexArray(0u);
	bonobo::checkForErrors();
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	bonobo::checkForErrors();

	//glEnable(GL_DEPTH_TEST); // If this is enabled, the points don't show up
	glEnable(GL_CULL_FACE);


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

		//bonobo::setRenderTarget(testingFbo, 0);
		glUseProgram(testingShader->mId);
		glViewport(0, 0, RES_X, RES_Y);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		bonobo::checkForErrors();
		bonobo::setUniform(*testingShader, "model_to_clip_matrix", cast<f32>(mCamera.GetWorldToClipMatrix()));
		bonobo::setUniform(*testingShader, "model_to_world_matrix", mat4f::Identity());
		
		glBindVertexArray(vao);
		bonobo::checkForErrors();
		glDrawArrays(GL_POINTS, 0, 4);
		bonobo::checkForErrors();
		glBindVertexArray(0u);
		//bonobo::drawFullscreen(*testingShader);
		bonobo::checkForErrors();

		//printf("Camera Rotation no:1: %f, no:2: %f\n", (mCamera.mRotation).x, (mCamera.mRotation).y);

		//GLStateInspection::CaptureSnapshot("Resolve Pass");
		bonobo::checkForErrors();

		//GLStateInspection::View::Render(); // Disabling this turns off the GLStateInspection console within the render window
		//Log::View::Render();
		ImGui::Render();

		window->Swap();
		lastTime = nowTime;
	}

	glDeleteBuffers(1, &vbo);
	bonobo::checkForErrors();
	vbo = 0u;
	glDeleteVertexArrays(1, &vao);
	bonobo::checkForErrors();
	vao = 0u;
}
