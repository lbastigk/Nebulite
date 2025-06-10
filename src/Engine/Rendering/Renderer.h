#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "Environment.h"
#include "FileManagement.h"
#include "Time.h"
#include "Invoke.h"

#include <thread>
#include <random>
#include <stdint.h>
#include "absl/container/flat_hash_map.h"


#define WINDOWNAME "coolgame"

class Renderer {
public:
	Renderer(Invoke& invoke, Nebulite::JSON& global, bool flag_hidden = false, unsigned int zoom = 1, unsigned int X = 1080, unsigned int Y = 1080);

	//Marshalling
	std::string serialize();
	std::string serializeGlobal(){
		return env.serializeGlobal();
	}
	void deserialize(std::string serialOrLink);
	
	//-----------------------------------------------------------
	// Pipeline
	void appendInvokePtr(Invoke* invoke);
	void append(std::shared_ptr<RenderObject> toAppend);
	void reinsertAllObjects();
	void update();
	void setGlobalValues();
	bool isQuit(){return quit;}
	void setQuit(){quit=true;}
	
	//-----------------------------------------------------------
	// Purge
	void purgeObjects();
	void purgeObjectsAt(int x, int y);
	void purgeLayer(int layer);
	void purgeTextures();
	void destroy();
	
	//-----------------------------------------------------------
	// Manipulation
	void changeWindowSize(int w, int h, int scalar);
	void moveCam(int dX, int dY, bool isMiddle = false);
	void setCam(int X, int Y, bool isMiddle = false);
	
	//-----------------------------------------------------------
	// Rendering
	bool timeToRender();
	void clear();
	void renderFrame();
	void renderFrameNoThreads();
	void renderFPS(float scalar = 1.0);
	void showFrame();
	void pollEvent();
	SDL_Event getEventHandle();
	
	//-----------------------------------------------------------
	// Setting
	void setFPS(int fps);
	void setThreadSize(unsigned int size);
	
	//-----------------------------------------------------------
	// Getting
	Nebulite::JSON& getGlobal(){return env.getGlobal();}
	int getEps(){return epsillon;}
	size_t getTextureAmount(){return TextureContainer.size();}
	size_t getObjectCount(){return env.getObjectCount();}
	int getResX(){return invoke_ptr->getGlobalPointer()->get<int>("Display.Resolution.X",0);}
	int getResY(){return invoke_ptr->getGlobalPointer()->get<int>("Display.Resolution.Y",0);}
	int getFPS(){return fps;}
	int getPosX(){return invoke_ptr->getGlobalPointer()->get<int>("Display.Position.X",0);};
	int getPosY(){return invoke_ptr->getGlobalPointer()->get<int>("Display.Position.Y",0);};
	bool windowExists(){return !!Renderer::window;}

	unsigned int getTileXpos(){return tileXpos;}
	unsigned int getTileYpos(){return tileYpos;}

	SDL_Renderer* getSdlRenderer(){return renderer;}
	Invoke* getInvoke(){return invoke_ptr;}

	// Updated with each renderer update
	void update_rand() {invoke_ptr->getGlobalPointer()->set<Uint64>("rand",   dist(rngA));};

	// Updated with each renderer update and append
	void update_rrand(){invoke_ptr->getGlobalPointer()->set<Uint64>("rrand",  dist(rngB));};

private:
	//-------------------------------------------------------------------------------------
	//General Variables
	uint32_t id_counter = 1;
	uint64_t starttime;
    uint64_t currentTime;
    uint64_t lastTime;
	Invoke* invoke_ptr = nullptr;

	// Subclasses
	Environment env;

	//Settings
	std::string directory;

	// Positions
	uint16_t tileXpos;
	uint16_t tileYpos;

	// Rendering
	unsigned int RenderZoom = 1;
	unsigned int RenderScalar = 1;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Rect rect;
	TTF_Font* font;
	absl::flat_hash_map<std::string, SDL_Texture*> TextureContainer;

	// Events etc
	SDL_Event event;
	bool quit = false;
	int MousePosX = 0;
	int MousePosY = 0;
	int lastMousePosX = 0;
	int lastMousePosY = 0;
	Uint32 lastMouseState;
	Uint32 mouseState;
	std::vector<Uint8> prevKeyState;

	uint64_t last_poll;

	//--------------------------------------------
	// RNG
	std::mt19937 rngA;
    std::mt19937 rngB;
    std::size_t hashString(const std::string& str);
	std::uniform_int_distribution<int> dist;
    

	//-------------------------------------------------------------------------------------
	//For FPS Count

	// Define font properties
	SDL_Color textColor = { 255, 255, 255, 255 }; // White color

	bool control_fps = false;
	int SCREEN_FPS = 500; // Target framerate (e.g., 60 FPS)
	int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS; // Milliseconds per frame
	Uint64 prevTicks = SDL_GetTicks64();
	Uint64 lastFPSRender = SDL_GetTicks64();
	Uint64 totalframes = 0;
	int fpsCount = 0;
	int fps = 0;

	//Extra delay in microseconds
	//Start value based on experience?
	int64_t epsillon = 0;
	int kp = 2;
	int ki = 10;
	int kd = 1;
	int64_t integral = 0;
	int64_t prevError = 0;
	

	//-------------------------------------------------------------------------------------
	//Other

	// Function to load texture from file
	void loadTexture(std::string link);

	
};
