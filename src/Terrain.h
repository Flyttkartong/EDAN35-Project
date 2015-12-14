#pragma once



class InputHandler;
class Window;


class Terrain {
public:
	Terrain(int argc, const char* argv[]);
	~Terrain();
	void run();
	float turbulence(int x, int y, int z);
	float smoothNoise(int x, int y, int z);
	int * createLookupTable();
private:
	InputHandler *inputHandler;
	Window       *window;
};

