#pragma once

#include <unordered_map>

#include <GLFW/glfw3.h>

#include "VectorMath.h"

/*----------------------------------------------------------------------------*/

#define PRESSED						(1 << 0)
#define RELEASED					(1 << 1)
#define JUST_PRESSED				(1 << 2)
#define JUST_RELEASED				(1 << 3)

#define MAX_MOUSE_BUTTONS			8

class InputHandler
{
public:
	struct IState {
		IState() : mDownTick(-1), mUpTick(-1), mIsDown(false) {}
		u64		mDownTick;
		u64		mUpTick;
		bool	mIsDown;
	};

public:
	InputHandler();

public:
	void FeedKeyboard(int key, int scancode, int action, int mods);
	void FeedMouseButtons(int button, int action, int mods);
	void FeedMouseMotion(vec2d const& position);
	void Advance();
	u32 GetScancodeState(int scancode);
	u32 GetKeycodeState(int key);
	u32 GetMouseState(u32 button);
	vec2d GetMousePositionAtStateShift(u32 button);
	vec2d GetMousePosition();

private:
	void DownEvent(std::unordered_map<size_t, IState> &map, size_t loc);
	void DownModEvent(std::unordered_map<size_t, IState> &map, u32 mods);
	void UpEvent(std::unordered_map<size_t, IState> &map, size_t loc);
	void UpModEvent(std::unordered_map<size_t, IState> &map, u32 mods);

	u32 GetState(std::unordered_map<size_t, IState> &map, size_t loc);

	std::unordered_map<size_t, IState> mScancodeMap;
	std::unordered_map<size_t, IState> mKeycodeMap;
	std::unordered_map<size_t, IState> mMouseMap;

	vec2d mMousePosition;
	vec2d mMousePositionSwitched[MAX_MOUSE_BUTTONS];

	u64 mTick;

};

