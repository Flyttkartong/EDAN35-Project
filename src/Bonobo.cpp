#include "Bonobo.h"
#include "GLB.h"
#include "Log.h"
#include "Window.h"

#ifdef _WIN32
#pragma warning(disable : 4996)
#endif

void Bonobo::Init()
{
	Log::Init();
	LogInfo("Running Bonobo v 0.2");

	LogInfo("Initiating window management system...");
	Window::Init();

	LogInfo("Done");
}

void Bonobo::Destroy()
{
	Window::Destroy();
	Log::Destroy();
}

