#include <imgui.h>

#include "Profiler.h"
#include "ProfilerView.h"

namespace Profiler {

#pragma warning(disable : 4996)

/*----------------------------------------------------------------------------*/

std::vector<Thread *> roots;
int currentItem = 0;
int currentProfile = 0;
char **threadString;
char **profileString;

/*----------------------------------------------------------------------------*/

void View::Init()
{
	threadString = new char*[PROFILER_MAX_THREADS];
	for (int i = 0; i < PROFILER_MAX_THREADS; i++)
		threadString[i] = new char[16];
	sprintf(threadString[0], "All");
	profileString = new char*[PROFILER_MAX_PROFILES];
	for (int i = 0; i < PROFILER_MAX_PROFILES; i++)
		profileString[i] = new char[256];
}

/*----------------------------------------------------------------------------*/

void View::Destroy()
{
	for (int i = 0; i < PROFILER_MAX_THREADS; i++)
		delete [] threadString[i];
	delete [] threadString;
	for (int i = 0; i < PROFILER_MAX_PROFILES; i++)
		delete [] profileString[i];
	delete [] profileString;
}

/*----------------------------------------------------------------------------*/

static void IncludeLocationRec(Location *loc, int &n)
{
	if (n >= PROFILER_MAX_PROFILES)
		return;
	n++;
	if (ImGui::TreeNode((void *) loc, loc->mIdentifier.c_str())) {
		ImGui::Text("Samples: %d", int(loc->mNSamples));
		ImGui::Text("Running mean: %.6f ms", float(loc->mRunningMean * 1000.0));

		ImGui::PlotLines("", loc->mSamples, PROFILER_SAMPLES, loc->mSamplePtr, 0, FLT_MAX, FLT_MAX, ImVec2(1100, 100));
		float mmin = FLT_MAX;
		float mmax = -FLT_MAX;
		float total = 0.0f;
		for (int i = 0; i < PROFILER_SAMPLES; i++) {
			mmax = loc->mSamples[i] > mmax ? loc->mSamples[i] : mmax;
			mmin = loc->mSamples[i] < mmin ? loc->mSamples[i] : mmin;
			total += loc->mSamples[i];
		}
		total /= float(PROFILER_SAMPLES);
		ImGui::Text("Window  min: %.6f ms", mmin * 1000.0f);
		ImGui::Text("Window  max: %.6f ms", mmax * 1000.0f);
		ImGui::Text("Window mean: %.6f ms", total * 1000.0f);
		ImGui::Text("Profiler overhead: %.6f ms", float(loc->mRunningMeanOverhead * 1000.0));

		for (auto c : loc->mChildren)
			IncludeLocationRec(c.second, n);
		ImGui::TreePop();
	}
}

/*----------------------------------------------------------------------------*/

void View::Render()
{
	Profiler::GetThreads(roots);

	bool opened = false;
	ImGui::Begin("Profiler", &opened, ImVec2(600, 400), -1.0f, 0);

	if (ImGui::Button("Enable/disable")) {
		Profiler::Toggle();
	}


	int n = 0;
	for (auto it : roots) {
		char buf[256];
		sprintf(buf, "Thread: %d", it->mThreadId);
		if (ImGui::TreeNode(buf)) {
			for (auto c : it->mRoot->mChildren)
				IncludeLocationRec(c.second, n);
			ImGui::TreePop();
		}
	}

	ImGui::End();

}

/*----------------------------------------------------------------------------*/

};
