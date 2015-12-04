#pragma once


class InputHandler;
class Window;


class Deferred {
public:
	Deferred(int argc, const char* argv[]);
	~Deferred();
	void run();

private:
	InputHandler *inputHandler;
	Window       *window;
};

