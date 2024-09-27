#include "GameLoop.h"

GameLoop::GameLoop() {
	//TODO, setting up Renderer etc....
}

bool GameLoop::timeToRender() {
	//TODO...
}

void GameLoop::renderFrame() {
	//Render Current instances
	Renderer.renderFrame();

	//Render FPS
	Renderer.renderFPS();

	// Present the renderer
	Renderer.showFrame();
}

void GameLoop::handleEvent() {
	event = Renderer.handleEvent();
}

//value to get, without $$
double GameLoop::getNumber(std::string toGet) {
	//first part of string is where to look (what doc to use)

	//rest is structure inside that doc
}

std::string GameLoop::getString(std::string toGet) {
	//first part of string is where to look (what doc to use)

	//rest is structure inside that doc
}