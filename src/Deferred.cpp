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
#include "Deferred.h"
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



Deferred::Deferred(int argc, const char* argv[])
{
	Log::View::Init();

	window = Window::Create("EDAN35: Assignment 2", RES_X, RES_Y, MSAA_RATE, false);
	inputHandler = new InputHandler();
	window->SetInputHandler(inputHandler);

	SimpleDraw::Init();
	GLStateInspection::Init();
	GLStateInspection::View::Init();
}

Deferred::~Deferred()
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

void Deferred::run()
{
	//
	// Load the scene, Sponza here, and all associated textures
	//
	Log("Loading Sponza...");
	OBJ::Mesh *mesh = OBJ::ReadCachedOBJFile(RESOURCES_PATH("crysponza/sponza.obj"));
	if (mesh == nullptr)
		std::exit(EXIT_FAILURE);
	OBJ::MaterialMap *materials = nullptr;
	std::unordered_map<std::string, bonobo::Texture *> diffuseTextures;
	std::unordered_map<std::string, bonobo::Texture *> specularTextures;
	std::unordered_map<std::string, bonobo::Texture *> normalTextures;
	if (!mesh->mMaterialFileName.empty()) {
		Log("Loading textures...");
		std::ostringstream oss;
		oss << RESOURCES_PATH("crysponza/") << mesh->mMaterialFileName;
		materials = OBJ::ReadMTLFile(oss.str());
		for (auto &it : materials->mMap) {
			std::string dt = it.second.mDiffuseTexture;
			if (!dt.empty()) {
				oss = std::ostringstream();
				oss << RESOURCES_PATH("crysponza/") << dt;
				diffuseTextures[it.first] = bonobo::loadTexture2D(oss.str());
			}
			std::string st = it.second.mSpecularTexture;
			if (!st.empty()) {
				oss = std::ostringstream();
				oss << RESOURCES_PATH("crysponza/") << st;
				specularTextures[it.first] = bonobo::loadTexture2D(oss.str());
			}
			std::string nt = it.second.mBumpTexture;
			if (!nt.empty()) {
				oss = std::ostringstream();
				oss << RESOURCES_PATH("crysponza/") << nt;
				normalTextures[it.first] = bonobo::loadTexture2D(oss.str());
			}

		}
	}
	Log("Done");


	//
	// Compute the scene bounding box to scale the camera speed, near and far plane accordingly
	//
	AABoxf sceneBox;
	for (auto it : mesh->mVertices)
		sceneBox.Include(it.mPosition);
	float sceneScale = length(sceneBox.Diagonal());
	printf("Scene scale %f\n", sceneScale);


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
	//
	std::string defaultShaderNames[2]   = { SHADERS_PATH("default.vert"),           SHADERS_PATH("default.frag") };
	std::string shaderNames[2]          = { SHADERS_PATH("fill_gbuffer.vert"),      SHADERS_PATH("fill_gbuffer.frag") };
	std::string shadowMapShaderNames[2] = { SHADERS_PATH("fill_shadowmap.vert"),    SHADERS_PATH("fill_shadowmap.frag") };
	std::string spotlightShaderNames[2] = { SHADERS_PATH("accumulate_lights.vert"), SHADERS_PATH("accumulate_lights.frag") };
	std::string resolveShaderNames[2]   = { SHADERS_PATH("resolve_deferred.vert"),  SHADERS_PATH("resolve_deferred.frag") };

	bonobo::ShaderProgram *defaultShader         = bonobo::loadShaderProgram(defaultShaderNames,   2);
	bonobo::ShaderProgram *lambertShader         = nullptr;
	bonobo::ShaderProgram *shadowMapShader       = nullptr;
	bonobo::ShaderProgram *spotlightShader       = nullptr;
	bonobo::ShaderProgram *deferredResolveShader = nullptr;

	if (defaultShader == nullptr) {
		LogError("Failed to load default shader\n");
		exit(-1);
	}

	auto reloadShaders = [&](){
		if (lambertShader != nullptr && lambertShader != defaultShader) {
			bonobo::unloadShader(&lambertShader->mShaders[0]);
			bonobo::unloadShader(&lambertShader->mShaders[1]);
			bonobo::unloadShaderProgram(lambertShader);
		}
		if (shadowMapShader != nullptr && shadowMapShader != defaultShader) {
			bonobo::unloadShader(&shadowMapShader->mShaders[0]);
			bonobo::unloadShader(&shadowMapShader->mShaders[1]);
			bonobo::unloadShaderProgram(shadowMapShader);
		}
		if (spotlightShader != nullptr && spotlightShader != defaultShader) {
			bonobo::unloadShader(&spotlightShader->mShaders[0]);
			bonobo::unloadShader(&spotlightShader->mShaders[1]);
			bonobo::unloadShaderProgram(spotlightShader);
		}
		if (deferredResolveShader != nullptr && deferredResolveShader != defaultShader) {
			bonobo::unloadShader(&deferredResolveShader->mShaders[0]);
			bonobo::unloadShader(&deferredResolveShader->mShaders[1]);
			bonobo::unloadShaderProgram(deferredResolveShader);
		}

		lambertShader         = bonobo::loadShaderProgram(shaderNames,          2);
		shadowMapShader       = bonobo::loadShaderProgram(shadowMapShaderNames, 2);
		spotlightShader       = bonobo::loadShaderProgram(spotlightShaderNames, 2);
		deferredResolveShader = bonobo::loadShaderProgram(resolveShaderNames,   2);

		if (lambertShader == nullptr)
			lambertShader = defaultShader;
		if (shadowMapShader == nullptr)
			shadowMapShader = defaultShader;
		if (spotlightShader == nullptr)
			spotlightShader = defaultShader;
		if (deferredResolveShader == nullptr)
			deferredResolveShader = defaultShader;
	};
	reloadShaders();


	//
	// Setup VAOs, VBOs and IBOs for the scene and cone meshes
	//
	bonobo::IBO *meshIbo = bonobo::loadIndexBufferObject(&mesh->mTriangles[0], sizeof(unsigned int), 3u * static_cast<unsigned int>(mesh->mTriangles.size()));

	bonobo::VBO *meshVbo = bonobo::loadVertexBufferObject(&mesh->mVertices[0], mesh->mVertexDesc.mStride, static_cast<unsigned int>(mesh->mVertices.size()));

	bonobo::VAO *meshLambertVao   = bonobo::loadVertexAttributeObject(*lambertShader,   *meshVbo, mesh->mVertexDesc);
	bonobo::VAO *meshShadowMapVao = bonobo::loadVertexAttributeObject(*shadowMapShader, *meshVbo, mesh->mVertexDesc);

	GLuint coneVao = 0u, coneVbo = 0u;
	GLsizei coneVerticesNb = 0;
	coneVao = loadCone(coneVbo, coneVerticesNb, spotlightShader);


	//
	// Setup FBOs
	//
	bonobo::Texture *rtDiffuse             = bonobo::loadTexture2D(RES_X, RES_Y, bonobo::TEXTURE_UNORM, v4i(8, 8, 8, 8), MSAA_RATE);
	bonobo::Texture *rtSpecular            = bonobo::loadTexture2D(RES_X, RES_Y, bonobo::TEXTURE_UNORM, v4i(8, 8, 8, 8), MSAA_RATE);
	bonobo::Texture *rtNormalSpecular      = bonobo::loadTexture2D(RES_X, RES_Y, bonobo::TEXTURE_UNORM, v4i(8, 8, 8, 8), MSAA_RATE);
	bonobo::Texture *rtLightDContributions = bonobo::loadTexture2D(RES_X, RES_Y, bonobo::TEXTURE_UNORM, v4i(8, 8, 8, 8), MSAA_RATE);
	bonobo::Texture *rtLightSContributions = bonobo::loadTexture2D(RES_X, RES_Y, bonobo::TEXTURE_UNORM, v4i(8, 8, 8, 8), MSAA_RATE);

	bonobo::Texture *rtDepthTexture = bonobo::loadTexture(nullptr, RES_X,           RES_Y,           0, 0, 1, 0, bonobo::TEXTURE_FLOAT_DEPTH, v4i(32, 0, 0, 0));
	bonobo::Texture *rtShadowMap    = bonobo::loadTexture(nullptr, SHADOWMAP_RES_X, SHADOWMAP_RES_Y, 0, 0, 1, 0, bonobo::TEXTURE_FLOAT_DEPTH, v4i(32, 0, 0, 0));

	const bonobo::Texture *gbufferColorAttachments[3] = { rtDiffuse, rtSpecular, rtNormalSpecular };
	const bonobo::Texture *lightColorAttachments[2]   = { rtLightDContributions, rtLightSContributions };

	bonobo::FBO *deferredFbo  = bonobo::loadFrameBufferObject(gbufferColorAttachments, 3, rtDepthTexture);
	bonobo::FBO *shadowMapFbo = bonobo::loadFrameBufferObject(                         0, rtShadowMap);
	bonobo::FBO *lightFbo     = bonobo::loadFrameBufferObject(lightColorAttachments,   2, rtDepthTexture);


	//
	// Setup texture samplers
	//
	bonobo::Sampler *sampler       = bonobo::loadSampler();
	bonobo::Sampler *depthSampler  = bonobo::loadSampler(bonobo::TEXTURE_BORDER, bonobo::TEXTURE_BILINEAR);
	bonobo::Sampler *shadowSampler = bonobo::loadSampler(bonobo::TEXTURE_BORDER, bonobo::TEXTURE_BILINEAR);

	GLfloat borderColor[4] = { 1.0f, 0.0, 0.0f, 0.0f };

	glSamplerParameteri (shadowSampler->mId, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glSamplerParameteri (shadowSampler->mId, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	glSamplerParameterfv(shadowSampler->mId, GL_TEXTURE_BORDER_COLOR, borderColor);


	//
	// Load the default texture
	//
	bonobo::Texture *defaultTexture = bonobo::loadTexture2D(RESOURCES_PATH("notex.png"));


	//
	// Setup lights properties
	//
	std::array<TRSTransform<f32>, LIGHTS_NB> lightTransforms = std::array<TRSTransform<f32>, LIGHTS_NB>();
	std::array<vec3f,             LIGHTS_NB> lightColors     = std::array<vec3f,             LIGHTS_NB>();

	for (unsigned int i = 0u; i < LIGHTS_NB; ++i) {
		lightTransforms[i].SetTranslate(v3f(0.0f, 125.0f, 0.0f));
		lightColors[i] = v3f(0.5f + 0.5f * (static_cast<f32>(rand()) / static_cast<f32>(RAND_MAX)),
		                     0.5f + 0.5f * (static_cast<f32>(rand()) / static_cast<f32>(RAND_MAX)),
							 0.5f + 0.5f * (static_cast<f32>(rand()) / static_cast<f32>(RAND_MAX)));
	}

	TRSTransform<f32> coneScaleTransform = TRSTransform<f32>();
	coneScaleTransform.SetScale(vec3f::Make(sqrt(LIGHT_INTENSITY / LIGHT_CUTOFF)));

	TRSTransform<f32> lightOffsetTransform = TRSTransform<f32>();
	lightOffsetTransform.SetTranslate(v3f(0.0f, 0.0f, -40.0f));

	mat4f lightProjection = mat4f::PerspectiveProjection(fPI * 0.5f, 1.0f, sceneScale * 0.01f, sceneScale * 4.0f);


	//
	// Setup some additionnal parameters
	//
	vec2f shadowMapTexelSize = v2f(1.0f / static_cast<f32>(SHADOWMAP_RES_X), 1.0f / static_cast<f32>(SHADOWMAP_RES_Y));
	vec2f invRes             = v2f(1.0f / static_cast<f32>(RES_X),           1.0f / static_cast<f32>(RES_Y));


	//
	// Set initial OpenGL state
	//
	glEnable(GL_DEPTH_TEST);
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

		if ((inputHandler->GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED)) {
			LogInfo("Reloading all shaders\n");
			reloadShaders();
		}

		ImGui_ImplGlfwGL3_NewFrame();



		glDepthFunc(GL_LESS);
		//
		// Pass 1: Render scene into the g-buffer
		//
		bonobo::setRenderTarget(deferredFbo, 0);
		glUseProgram(lambertShader->mId);
		glViewport(0, 0, RES_X, RES_Y);
		glClearDepthf(1.0f);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::checkForErrors();
	
		bonobo::setUniform(*lambertShader, "model_to_clip_matrix", cast<f32>(mCamera.GetWorldToClipMatrix()));
		bonobo::setUniform(*lambertShader, "model_to_world_normal_matrix", mat4f::Identity());
	
		glBindVertexArray(meshLambertVao->mId);
		bonobo::bindVertexBufferObject(*meshVbo, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIbo->mId);
		bonobo::checkForErrors();

		for (OBJ::Group const& group : mesh->mGroups) {
			bonobo::Texture *diffuseTexture = defaultTexture;
			bonobo::Texture *specularTexture = defaultTexture;
			bonobo::Texture *normalTexture = defaultTexture;
			auto mat = diffuseTextures.find(group.mMaterial);
			if (mat != diffuseTextures.end())
				diffuseTexture = mat->second;
			mat = specularTextures.find(group.mMaterial);
			if (mat != specularTextures.end())
				specularTexture = mat->second;
			mat = normalTextures.find(group.mMaterial);
			if (mat != normalTextures.end())
				normalTexture = mat->second;

			bonobo::bindTextureSampler(*lambertShader, "diffuse_texture", 0, *diffuseTexture, *sampler);
			bonobo::bindTextureSampler(*lambertShader, "specular_texture", 1, *specularTexture, *sampler);
			bonobo::bindTextureSampler(*lambertShader, "normal_texture", 2, *normalTexture, *sampler);

			GLStateInspection::CaptureSnapshot("Filling Pass");

			glDrawElements(GL_TRIANGLES, (group.mEnd - group.mStart) * 3, GL_UNSIGNED_INT, (void *)(group.mStart * 3 * 4));
			bonobo::checkForErrors();
		}

		glBindSampler(2u, 0u);
		bonobo::checkForErrors();
		glBindSampler(1u, 0u);
		bonobo::checkForErrors();
		glBindSampler(0u, 0u);
		bonobo::checkForErrors();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		bonobo::checkForErrors();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		bonobo::checkForErrors();
		glBindVertexArray(0u);
		bonobo::checkForErrors();



		glCullFace(GL_FRONT);
		//
		// Pass 2: Generate shadowmaps and accumulate lights' contribution
		//
		bonobo::setRenderTarget(lightFbo, 0);
		glUseProgram(lambertShader->mId);
		glViewport(0, 0, RES_X, RES_Y);
		glClearDepthf(1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(/*GL_DEPTH_BUFFER_BIT | */GL_COLOR_BUFFER_BIT);
		bonobo::checkForErrors();
		for (int i = 0; i < LIGHTS_NB; ++i) {
			float secondsNb = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count() * 0.001f;

			TRSTransform<f32>& lightTransform = lightTransforms[i];
			lightTransform.SetRotate(secondsNb * 0.1f + i * 1.57f, v3f(0.0f, 1.0f, 0.0f));

			//
			// Pass 2.1: Generate shadow map for light i
			//
			bonobo::setRenderTarget(shadowMapFbo, 0);
			glUseProgram(shadowMapShader->mId);
			glViewport(0, 0, SHADOWMAP_RES_X, SHADOWMAP_RES_Y);
			glClearDepthf(1.0f);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			bonobo::checkForErrors();

			mat4f lightMatrix = lightProjection * lightOffsetTransform.GetMatrixInverse() * lightTransform.GetMatrixInverse();
			bonobo::setUniform(*shadowMapShader, "model_to_clip_matrix", lightMatrix);

			
			GLStateInspection::CaptureSnapshot("Shadow Map Generation");

			
			glBindVertexArray(meshShadowMapVao->mId);
			bonobo::bindVertexBufferObject(*meshVbo, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIbo->mId);
			bonobo::checkForErrors();

			
			for (OBJ::Group const& group : mesh->mGroups)
				glDrawElements(GL_TRIANGLES, (group.mEnd - group.mStart) * 3, GL_UNSIGNED_INT, (void *)(group.mStart * 3 * 4));

			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			bonobo::checkForErrors();
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			bonobo::checkForErrors();
			glBindVertexArray(0u);
			bonobo::checkForErrors();


			glEnable(GL_BLEND);
			glDepthFunc(GL_GREATER);
			glDepthMask(GL_FALSE);
			glBlendEquationSeparate(GL_FUNC_ADD, GL_MIN);
			glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
			//
			// Pass 2.2: Accumulate light i contribution
			bonobo::setRenderTarget(lightFbo, 0);
			glUseProgram(spotlightShader->mId);
			glViewport(0, 0, RES_X, RES_Y);
			//glClearDepthf(0.0f);
			//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			bonobo::checkForErrors();

			bonobo::bindTextureSampler(*spotlightShader, "depthBuffer",             0, *rtDepthTexture,   *depthSampler);
			bonobo::bindTextureSampler(*spotlightShader, "normalAndSpecularBuffer", 1, *rtNormalSpecular, *sampler);
			bonobo::bindTextureSampler(*spotlightShader, "shadowMap",               2, *rtShadowMap,      *shadowSampler);
			
			bonobo::setUniform(*spotlightShader, "invRes",                invRes);
			bonobo::setUniform(*spotlightShader, "ViewProjectionInverse", mCamera.GetClipToWorldMatrix());
			bonobo::setUniform(*spotlightShader, "ViewPosition",          mCamera.mWorld.GetTranslation());
			bonobo::setUniform(*spotlightShader, "shadowViewProjection",  lightMatrix);
			
			bonobo::setUniform(*spotlightShader, "model_to_clip_matrix", mCamera.GetWorldToClipMatrix() * lightTransform.GetMatrix() * lightOffsetTransform.GetMatrix() * coneScaleTransform.GetMatrix());
			bonobo::setUniform(*spotlightShader, "LightColor",           lightColors[i]);
			bonobo::setUniform(*spotlightShader, "LightPosition",        lightTransform.GetTranslation());
			bonobo::setUniform(*spotlightShader, "LightDirection",       lightTransform.GetFront());
			bonobo::setUniform(*spotlightShader, "LightIntensity",       LIGHT_INTENSITY);
			bonobo::setUniform(*spotlightShader, "LightAngleFalloff",    LIGHT_ANGLE_FALLOFF);
			bonobo::setUniform(*spotlightShader, "ShadowMapTexelSize",   shadowMapTexelSize);
			
			GLStateInspection::CaptureSnapshot("Accumulating");
			
			glBindVertexArray(coneVao);
			bonobo::checkForErrors();
			
			glDrawArrays(GL_TRIANGLE_STRIP, 0, coneVerticesNb);
			bonobo::checkForErrors();
			
			glBindVertexArray(0u);
			bonobo::checkForErrors();
			
			glBindSampler(2u, 0u);
			bonobo::checkForErrors();
			glBindSampler(1u, 0u);
			bonobo::checkForErrors();
			glBindSampler(0u, 0u);
			bonobo::checkForErrors();

			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);
			glDisable(GL_BLEND);
		}


		glCullFace(GL_BACK);
		glDepthFunc(GL_ALWAYS);
		//
		// Pass 3: Compute final image using both the g-buffer and  the light accumulation buffer
		//
		bonobo::setRenderTarget(0, 0);
		glUseProgram(deferredResolveShader->mId);
		glViewport(0, 0, RES_X, RES_Y);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::checkForErrors();

		bonobo::bindTextureSampler(*deferredResolveShader, "diffuse_buffer",  0, *rtDiffuse,             *sampler);
		bonobo::bindTextureSampler(*deferredResolveShader, "specular_buffer", 1, *rtSpecular,            *sampler);
		bonobo::bindTextureSampler(*deferredResolveShader, "light_d_buffer",  2, *rtLightDContributions, *sampler);
		bonobo::bindTextureSampler(*deferredResolveShader, "light_s_buffer",  3, *rtLightSContributions, *sampler);

		GLStateInspection::CaptureSnapshot("Resolve Pass");

		bonobo::drawFullscreen(*deferredResolveShader);


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
		gSimpleDraw.Texture(v2f(-0.95f, -0.95f), v2f(-0.55f, -0.55f), *rtDiffuse,             v4i(0, 1, 2, -1));
		gSimpleDraw.Texture(v2f(-0.45f, -0.95f), v2f(-0.05f, -0.55f), *rtNormalSpecular,      v4i(0, 1, 2, -1));
		gSimpleDraw.Texture(v2f( 0.05f, -0.95f), v2f( 0.45f, -0.55f), *rtSpecular,            v4i(0, 1, 2, -1));
		gSimpleDraw.Texture(v2f( 0.55f, -0.95f), v2f( 0.95f, -0.55f), *rtDepthTexture,        v4i(0, 0, 0, -1));
		gSimpleDraw.Texture(v2f(-0.95f,  0.55f), v2f(-0.55f,  0.95f), *rtShadowMap,           v4i(0, 0, 0, -1));
		gSimpleDraw.Texture(v2f(-0.45f,  0.55f), v2f(-0.05f,  0.95f), *rtLightDContributions, v4i(0, 1, 2, -1));
		gSimpleDraw.Texture(v2f( 0.05f,  0.55f), v2f( 0.45f,  0.95f), *rtLightSContributions, v4i(0, 1, 2, -1));


		bonobo::checkForErrors();

		GLStateInspection::View::Render();
		//Log::View::Render();
		ImGui::Render();

		window->Swap();
		lastTime = nowTime;
	}

	glDeleteBuffers(1, &coneVbo);
	bonobo::checkForErrors();
	coneVbo = 0u;
	glDeleteVertexArrays(1, &coneVao);
	bonobo::checkForErrors();
	coneVao = 0u;
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
