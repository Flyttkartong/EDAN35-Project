#include "Profiler.h"
#include "Misc.h"
#include <mutex>
#include <cstring>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace Profiler {

/*----------------------------------------------------------------------------*/

std::mutex stackMutex;
bool profileEnabled = true;
std::unordered_map<std::thread::id, Thread *> rootMap;

Location::Location(std::string id) : mIdentifier(id), mNSamples(0.0), mRunningMean(0.0), mRunningMeanOverhead(0.0), mSamplePtr(0)
{
	mSamples = new float[PROFILER_SAMPLES];
	memset(mSamples, 0, sizeof(float) * PROFILER_SAMPLES);
}

Location::~Location()
{
	delete [] mSamples;
	for (auto it : mChildren)
		delete it.second;
}

/*----------------------------------------------------------------------------*/

void Init()
{
}

void Destroy()
{
	for (auto it : rootMap) {
		delete it.second->mRoot;
		while (!it.second->mProfileStack.empty())
			it.second->mProfileStack.pop();
		delete it.second;
	}
}

void Enable(bool enable)
{
	profileEnabled = enable;
}

void Toggle()
{
	profileEnabled = !profileEnabled;
}

void GetThreads(std::vector<Thread *> &threads)
{
	threads.clear();
	stackMutex.lock();
	for (auto it : rootMap)
		threads.push_back(it.second);
	stackMutex.unlock();
}

/*----------------------------------------------------------------------------*/

Instance::Instance(std::string uniqueIdentifier)
{
	mEnabled = profileEnabled;
	if (!profileEnabled)
		return;
	auto overheadStart = StartTimer();

	Thread *pt;
	std::thread::id tid = GetThreadID();
	stackMutex.lock();
	auto elem = rootMap.find(tid);
	if (elem == rootMap.end()) {
		pt = new Thread();
		rootMap[tid] = pt;
		Location *root = new Location("__root");
		pt->mRoot = root;
		pt->mProfileStack.push(root);
		pt->mThreadId = tid;
	} else
		pt = elem->second;
	stackMutex.unlock();

	Location *parent = pt->mProfileStack.top();
	Location *pl;
	auto loc = parent->mChildren.find(uniqueIdentifier);
	if (loc == parent->mChildren.end()) {
		pl = new Location(uniqueIdentifier);
		parent->mChildren[uniqueIdentifier] = pl;
	} else
		pl = loc->second;
	pt->mProfileStack.push(pl);

	mProfileThread = (void *) pt;
	mProfileLocation = (void *) pl;
	mOverheadTime = EndTimerSeconds(overheadStart);
	mStartTime = StartTimer();
}

/*----------------------------------------------------------------------------*/

Instance::~Instance()
{
	if (!mEnabled)
		return;
	double timeDiffd = EndTimerSeconds(mStartTime);
	float timeDifff = float(timeDiffd);
	Location *pl = (Location *) mProfileLocation;
	Thread *pt = (Thread *) mProfileThread;

	/*
		PrevMean = sum[1, n](s_i) / n
		NewMean  = (sum[1, n](s_i) + s_(n + 1)) / (n + 1)
			==>
		NewMean = (n PrevMean + smp) / (n + 1)
	*/
	pl->mRunningMeanOverhead = (pl->mNSamples * pl->mRunningMeanOverhead + mOverheadTime) / (pl->mNSamples + 1.0);
	pl->mRunningMean = (pl->mNSamples * pl->mRunningMean + timeDiffd) / (pl->mNSamples + 1.0);
	pl->mNSamples += 1.0;

	pl->mSamples[pl->mSamplePtr] = timeDifff;
	pl->mSamplePtr = (pl->mSamplePtr + 1) % PROFILER_SAMPLES;

	pt->mProfileStack.pop();
}

/*----------------------------------------------------------------------------*/

};
