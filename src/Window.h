#pragma once

#include <string>

#include <GLFW/glfw3.h>

#include "VectorMath.h"


class InputHandler;


class Window
{
public:
	enum SwapStrategy {
		DISABLE_VSYNC = 0,
		ENABLE_VSYNC = 1,
		LATE_SWAP_TEARING = -1
	};
public:
	static void Init();
	static Window *Create(std::string title, unsigned int w, unsigned int h, unsigned int msaa = 4, bool fullscreen = false, SwapStrategy swap = ENABLE_VSYNC);
	static bool Destroy(Window *window);
	static void Destroy();

protected:
	Window(std::string title, unsigned int w, unsigned int h, unsigned int msaa, bool fullscreen_, SwapStrategy swap_);
	~Window();
public:
	void SetFullscreen(bool state);
	std::string GetTitle() const;
	void Swap() const;
	vec2i GetDimensions() const;
	GLFWwindow *GetGLFW_Window() const;
	void SetInputHandler(InputHandler *inputHandler);
private:
	bool Show();
	static void ErrorCallback(int error, char const* description);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void MouseCallback(GLFWwindow* window, int button, int action, int mods);
	static void CursorCallback(GLFWwindow* window, double x, double y);
private:
	std::string mTitle;
	unsigned int mWidth, mHeight;
	unsigned int mMSAA;
	bool mFullscreen;
	SwapStrategy mSwap;
	GLFWwindow *mWindowGLFW;
	InputHandler *mInputHandler;
};

