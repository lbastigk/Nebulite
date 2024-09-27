#include "JSONHandler.h"
#include "Renderer.h"

class GameLoop {
public:
	GameLoop();

	bool timeToRender();

	void renderFrame();

	void handleEvent();

	//value to get, without $$
	double getNumber(std::string toGet);

	std::string getString(std::string toGet);

private:
	bool renderFPS = true;
	int event = 0;
	Renderer Renderer;
};