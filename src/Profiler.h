/*
 * CPU performance profiler
 */

#pragma once
#include <string>
#include <unordered_map>
#include <stack>
#include <vector>
#include "BuildSettings.h"
#include "Misc.h"

// Settings
#define PROFILER_SAMPLES		1024

namespace Profiler {

void Init();
void Destroy();

// Pause and resume profiling
void Enable(bool enable);
void Toggle();

// Describes one profiling point in the code.
// If the same location is encountered from different nested profiling regions,
// multiple Location objects will be created (i.e., they are treated as different locations)
struct Location {
	Location(std::string id);
	~Location();

	std::string mIdentifier;			// LOWPRIO: Replace with hash
	float *mSamples;					// Ring buffer of timing samples (in seconds)
	int mSamplePtr;						// Sample ring buffer head pointer
	double mRunningMean;				// Running mean of all samples (note: longer runs => higher error)
	double mRunningMeanOverhead;		// Running mean of profiling code overhead
	double mNSamples;					// Total number of samples
	std::unordered_map<std::string, Location *> mChildren; // Nested profiling regions
};

// Threads are handled separately.
struct Thread {
	std::stack<Location *> mProfileStack;
	Location *mRoot;
	std::thread::id mThreadId;
};

// Populate a list of all threads. Use this to inspect profiling results.
void GetThreads(std::vector<Thread *> &threads);

// This produces one sample for a particular location
// The profiling region starts with the constructor and ends when the destructor is called
class Instance {
public:
	Instance(std::string uniqueIdentifier);
	~Instance();

private:
	double mOverheadTime;
	std::chrono::high_resolution_clock::time_point mStartTime;
	void *mProfileLocation;
	void *mProfileThread;
	bool mEnabled;
};

};

#if defined ENABLE_PROFILING && ENABLE_PROFILING != 0
	#define PROFILE(a)		Profiler::Instance profilerInstance(a)
#else
	#define PROFILE(a)
#endif
