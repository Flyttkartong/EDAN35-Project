#pragma once


class InputHandler;
class Window;


class Testing {
public:
	Testing(int argc, const char* argv[]);
	~Testing();
	void run();

private:
	InputHandler *inputHandler;
	Window       *window;
};

