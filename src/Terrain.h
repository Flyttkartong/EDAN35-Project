#pragma once



class InputHandler;
class Window;


class Terrain {
public:
	Terrain(int argc, const char* argv[]);
	~Terrain();
	void run();
	float * createLookupTable();
private:
	InputHandler *inputHandler;
	Window       *window;
};

