#include <unordered_map>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "BuildSettings.h"
#include "GLB.h"
#include "imgui_impl_glfw_gl3.h"
#include "InputHandler.h"
#include "Log.h"
#include "Window.h"

std::unordered_map<std::string, Window *> *windowMap = new std::unordered_map<std::string, Window *>();

void Window::ErrorCallback(int error, char const* description)
{
  if (error == 65545 || error == 65543)
    LogInfo("Couldn't setup OpenGL, trying a lower version\n");
  else
    LogError("GLFW error %d was thrown:\n\t%s\n", error, description);
}

void Window::KeyCallback( GLFWwindow* window, int key, int scancode
                            , int action, int mods)
{
  Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
  instance->mInputHandler->FeedKeyboard(key, scancode, action, mods);

#ifdef _WIN32
  bool should_close = (key == GLFW_KEY_F4) && (mods == GLFW_MOD_ALT);
#elif defined __APPLE__
  bool should_close = (key == GLFW_KEY_Q) && (mods == GLFW_MOD_SUPER);
#elif defined __linux__
  bool should_close = (key == GLFW_KEY_Q) && (mods == GLFW_MOD_CONTROL);
#else
  bool should_close = false;
#endif
  should_close |= (key == GLFW_KEY_ESCAPE);
  if (should_close)
    glfwSetWindowShouldClose(window, true);

  ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
}

void Window::MouseCallback( GLFWwindow* window, int button, int action
                              , int mods
                              )
{
  Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
  instance->mInputHandler->FeedMouseButtons(button, action, mods);
}

void Window::CursorCallback(GLFWwindow* window, double x, double y)
{
  Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
  instance->mInputHandler->FeedMouseMotion(v2d(x, y));
}


Window *Window::Create(std::string mTitle, unsigned int w, unsigned int h, unsigned int msaa, bool fullscreen, SwapStrategy swap)
{
	bool ok = Param(w > 0 && h > 0) ||
		Param(!mTitle.empty());
	if (!ok) {
		LogWarning("No window created! (%s)", mTitle.c_str());
		return nullptr;
	}
	auto elem = windowMap->find(mTitle);
	if (elem != windowMap->end()) {
		LogWarning("A window named %s already exists", mTitle.c_str());
		return nullptr;
	}
	Window *window = new Window(mTitle, w, h, msaa, fullscreen, swap);
	(*windowMap)[mTitle] = window;
	return window;
}

bool Window::Destroy(Window *window)
{
	auto elem = windowMap->find(window->GetTitle());
	if (elem == windowMap->end()) {
		LogWarning("This window isn't mine to destroy");
		return false;
	}
	windowMap->erase(window->GetTitle());
	delete window;
	return true;
}

std::string Window::GetTitle() const
{
	return mTitle;
}

Window::Window(std::string mTitle_, unsigned w_, unsigned h_, unsigned int msaa_, bool fullscreen_, SwapStrategy swap_) :
	mTitle(mTitle_), mWidth(w_), mHeight(h_), mMSAA(msaa_), mFullscreen(fullscreen_), mWindowGLFW(nullptr), mSwap(swap_), mInputHandler(nullptr)
{
	Show();
}

Window::~Window()
{
}

bool Window::Show()
{
	if (mWindowGLFW == nullptr) {
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		glfwWindowHint(GLFW_SAMPLES, mMSAA);

		GLFWmonitor* const monitor = mFullscreen ? glfwGetPrimaryMonitor()
                                                 : nullptr;
		mWindowGLFW = glfwCreateWindow(mWidth, mHeight
                                ,mTitle.c_str() ,monitor, nullptr);
    if (mWindowGLFW == nullptr) {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      
      mWindowGLFW = glfwCreateWindow(mWidth, mHeight
                                     ,mTitle.c_str() ,monitor, nullptr);
      
      if (mWindowGLFW == nullptr)
        return false;
    }

        glfwMakeContextCurrent(mWindowGLFW);

        gl3wInit();
        ImGui_ImplGlfwGL3_Init(mWindowGLFW, false);

		glfwSetKeyCallback(mWindowGLFW, Window::KeyCallback);
		glfwSetInputMode(mWindowGLFW, GLFW_STICKY_KEYS, 1);
		glfwSetMouseButtonCallback(mWindowGLFW, Window::MouseCallback);
		glfwSetCursorPosCallback(mWindowGLFW, Window::CursorCallback);
		//glfwSetInputMode(mWindowGLFW, GLFW_CURSOR, GLFW_CURSOR_disable()D);
		glfwSetWindowUserPointer(mWindowGLFW, static_cast<void*>(this));

		glfwSetScrollCallback(mWindowGLFW, ImGui_ImplGlfwGL3_ScrollCallback);
		glfwSetCharCallback(mWindowGLFW, ImGui_ImplGlfwGL3_CharCallback);

		GLint major_version = 0, minor_version = 0, context_flags = 0, profile_mask = 0;
		glGetIntegerv(GL_MAJOR_VERSION, &major_version);
		bonobo::checkForErrors();
		glGetIntegerv(GL_MINOR_VERSION, &minor_version);
		bonobo::checkForErrors();
		glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
		bonobo::checkForErrors();
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile_mask);
		bonobo::checkForErrors();
		LogInfo("Using OpenGL %d.%d with context options: profile=%s, debug=%s, robust access=%s, forward compatible=%s.", major_version, minor_version
                       , (profile_mask & GL_CONTEXT_CORE_PROFILE_BIT) ? "core" : (profile_mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) ? "compatibility" : "unknown"
                       , (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) ? "true" : "false"
                       , (context_flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT) ? "true" : "false"
                       , (context_flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) ? "true" : "false"
                       );

		if (major_version >= 4 && minor_version >= 3)
		{
#if DEBUG_LEVEL >= 2
			glEnable(GL_DEBUG_OUTPUT);
			glDebugMessageCallback(bonobo::openglErrorCallback, nullptr);
#endif
#if DEBUG_LEVEL == 2
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
#elif DEBUG_LEVEL == 3
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif
		}
		else
		{
			LogInfo("DebugCallback is not core in OpenGL %d.%d, but might be enabled through an extension. However this is currently not supported by bonobo.", major_version, minor_version);
		}
	}

	glfwSwapInterval(mSwap);
	// TODO: Reinitiate renderer
	return true;
}

void Window::SetFullscreen(bool state)
{
	if (mFullscreen == state)
		return;
	if (mWindowGLFW)
		glfwDestroyWindow(mWindowGLFW);
	mFullscreen = state;
	Show();
}

void Window::Init()
{
	glfwSetErrorCallback(Window::ErrorCallback);
	glfwInit();
}

void Window::Destroy()
{
	if (windowMap == nullptr)
		return;
	for (auto it = windowMap->begin(); it != windowMap->end(); ++it)
		delete it->second;
	windowMap->clear();
	delete windowMap;
	windowMap = nullptr;
	glfwTerminate();
}

void Window::Swap() const
{
	glfwSwapBuffers(mWindowGLFW);
}

vec2i Window::GetDimensions() const
{
	return vec2i::Make(mWidth, mHeight);
}

GLFWwindow *Window::GetGLFW_Window() const
{
	return mWindowGLFW;
}

void Window::SetInputHandler(InputHandler *inputHandler)
{
	mInputHandler = inputHandler;
}
