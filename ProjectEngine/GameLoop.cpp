#include "JSONHandler.cpp"
#include "Renderer.cpp"

class GameLoop {
	GameLoop() {
		//TODO, setting up Renderer etc....
	}
public:
	


	bool timeToRender() {
		//TODO...
	}

	void renderFrame() {
		//Render Current instances
		Renderer.renderFrame();

		//Render FPS
		Renderer.renderFPS();

		// Present the renderer
		Renderer.showFrame();
	}

	void handleEvent() {
		event = Renderer.handleEvent();
	}

	//value to get, without $$
	double getNumber(std::string toGet) {
		//first part of string is where to look (what doc to use)

		//rest is structure inside that doc
	}

	std::string getString(std::string toGet) {
		//first part of string is where to look (what doc to use)

		//rest is structure inside that doc
	}

private:
	bool renderFPS = true;
	int event = 0;
	Renderer Renderer;
};