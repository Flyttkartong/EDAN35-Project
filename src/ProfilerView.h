#pragma once

#include "Profiler.h"

#define PROFILER_MAX_THREADS		1024
#define PROFILER_MAX_PROFILES		4096

namespace Profiler {

class View {
public:
	static void Init();
	static void Destroy();
public:
	static void Render();
};

};
