#include "Bonobo.h"
#include "Deferred.h"
#include "Terrain.h"
#include "Testing.h"

#ifdef main
#	undef main
#endif
int main(int argc, const char* argv[])
{
	Bonobo::Init();

	/*Deferred *deferred = new Deferred(argc, argv);
	deferred->run();
    delete deferred;
    deferred = nullptr;*/

	Terrain *terrain = new Terrain(argc, argv);
	terrain->run();
	delete terrain;
	terrain = nullptr;

	/*Testing *testing = new Testing(argc, argv);
	testing->run();
	delete testing;
	testing = nullptr;*/

	Bonobo::Destroy();
}

