#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

//#include "OptionsLoader.h"

#include "Environment.h"

#include "FileManagement.h"
#include "Time.h"

#include "Invoke.h"
#include <thread>

#include <stdint.h>


#define WINDOWNAME "coolgame"

class Renderer {
public:
	Renderer(std::deque<std::string>& tasks, Invoke& invoke, bool flag_hidden = false, unsigned int zoom = 1, unsigned int X = 1080, unsigned int Y = 1080);

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
	void update_withThreads();
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
	void changeWindowSize(int w, int h);
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
	rapidjson::Document& getGlobal(){return env.getGlobal();}
	int getEps(){return epsillon;}
	size_t getTextureAmount(){return TextureContainer.size();}
	size_t getObjectCount(){return env.getObjectCount();}
	int getResX(){return JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"Display.Resolution.X",0);}
	int getResY(){return JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"Display.Resolution.Y",0);}
	int getThreadSize(){return THREADSIZE;}
	int getFPS(){return fps;}
	int getPosX(){return JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"Display.Position.X",0);};
	int getPosY(){return JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"Display.Position.Y",0);};
	bool windowExists(){return !!Renderer::window;}

	unsigned int getTileXpos(){return tileXpos;}
	unsigned int getTileYpos(){return tileYpos;}

	SDL_Renderer* getSdlRenderer(){return renderer;}
	Invoke* getInvoke(){return invoke_ptr;}

	class SDL {
	public:
		const static int KEY_ESC = 27;
		const static int KEY_SPACE = 32;
		const static int KEY_ENTER = 13;

		const static int KEY_Q = 113;
		const static int KEY_W = 119;
		const static int KEY_E = 101;
		const static int KEY_R = 114;
		const static int KEY_T = 116;
		const static int KEY_Z = 122;
		const static int KEY_U = 117;
		const static int KEY_I = 105;
		const static int KEY_O = 111;
		const static int KEY_P = 112;
		//const static int KEY_� = 252;

		const static int KEY_A = 97;
		const static int KEY_S = 115;
		const static int KEY_D = 100;
		const static int KEY_F = 102;
		const static int KEY_G = 103;
		const static int KEY_H = 104;
		const static int KEY_J = 106;
		const static int KEY_K = 107;
		const static int KEY_L = 108;
		//const static int KEY_� = 246;
		//const static int KEY_� = 228;

		const static int KEY_Y = 121;
		const static int KEY_X = 120;
		const static int KEY_C = 99;
		const static int KEY_V = 118;
		const static int KEY_B = 98;
		const static int KEY_N = 110;
		const static int KEY_M = 109;
	};

	

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
	unsigned int THREADSIZE = 2;
	std::string directory;

	// Positions
	unsigned int tileXpos;
	unsigned int tileYpos;

	// Rendering
	unsigned int RenderZoom = 1;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Rect rect;
	TTF_Font* font;
	std::map<std::string, SDL_Texture*> TextureContainer;

	// Events etc
	SDL_Event event;
	bool quit = false;
	int MousePosX = 0;
	int MousePosY = 0;
	int lastMousePosX = 0;
	int lastMousePosY = 0;
	Uint32 lastMouseState;
	Uint32 mouseState;

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
