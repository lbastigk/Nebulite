/**
 * @file Renderer.h
 *
 * This file contains the declaration of the Renderer class, which is responsible for rendering
 * the game objects and managing the rendering pipeline.
 */

#pragma once

// Global Libraries
#include <thread>
#include <random>
#include <stdint.h>

// External Libraries
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "absl/container/flat_hash_map.h"

// Nebulite
#include "Environment.h"
#include "FileManagement.h"
#include "Time.h"
#include "TimeKeeper.h"
#include "Invoke.h"




namespace Nebulite{

/**
 * @class Nebulite::Renderer
 * 
 * @brief Responsible for rendering game objects and managing the rendering pipeline.
 */
class Renderer {
public:

	
	/**
	 * @brief Initializes a Renderer with given dimensions and settings.
	 * 
	 * @param invoke Reference to the Nebulite::Invoke instance.
	 * @param global Reference to the global Nebulite::JSON instance.
	 * @param flag_headless Boolean flag for headless mode.
	 * @param X Width of the rendering area.
	 * @param Y Height of the rendering area.
	 */
	Renderer(Nebulite::Invoke& invoke, Nebulite::JSON& global, bool flag_headless = false, unsigned int X = 1080, unsigned int Y = 1080);

	//Marshalling
	std::string serialize(){
		// TODO: Serialize render-specific variables?
		return env.serialize();
	}

	void deserialize(std::string serialOrLink){
		env.deserialize(
			serialOrLink, 
			invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),0), 
			invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),0)
		);
	}
	
	void loadFonts(int scalar);

	//-----------------------------------------------------------
	// Pipeline
	void appendInvokePtr(Nebulite::Invoke* invoke);
	void append(Nebulite::RenderObject* toAppend);
	void reinsertAllObjects();
	void update();
	void setGlobalValues();
	bool isQuit(){return quit;}
	void setQuit(){quit=true;}
	bool snapshot(std::string link);

	bool attachTextureAboveLayer(Environment::Layers aboveThisLayer, std::string name, SDL_Texture* texture) {
		if(texture == nullptr) {
			return false; // Cannot attach a null texture
		}

		BetweenLayerTextures[aboveThisLayer][name] = texture;
		return true;
	}
	bool removeTextureBetweenLayer(std::string name) {
		bool found = false;
		for(auto& layer : BetweenLayerTextures) {
			if(layer.second.contains(name)) {
				layer.second.erase(name);
				found = true;
			}
		}
		return found;
	}

	//-----------------------------------------------------------
	// Special Functions
	void beep();
	
	//-----------------------------------------------------------
	// Purge
	void purgeObjects();
	void purgeObjectsAt(int x, int y);
	void purgeTextures();
	void destroy();
	
	//-----------------------------------------------------------
	// Manipulation
	void changeWindowSize(int w, int h, int scalar);
	void moveCam(int dX, int dY, bool isMiddle = false);
	void setCam(int X, int Y, bool isMiddle = false);

	void setForcedGlobalValue(const std::string& key, const std::string& value) {
		forced_global_values.emplace_back(key, value);
	}
	void clearForcedGlobalValues() {
		forced_global_values.clear();
	}
	
	//-----------------------------------------------------------
	// Rendering
	void tick();
	bool timeToRender();
	
	//-----------------------------------------------------------
	// Setting
	void toggleFps(bool show = true){
		showFPS = show;
	}
	void setFPS(int fps);
	void setThreadSize(unsigned int size);
	
	//-----------------------------------------------------------
	// Getting

	size_t getTextureAmount(){return TextureContainer.size();}
	size_t getObjectCount(){return env.getObjectCount();}
	int getResX(){return invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),0);}
	int getResY(){return invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),0);}
	int getFPS(){return fps;}
	int getPosX(){return invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.positionX.c_str(),0);}
	int getPosY(){return invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.positionY.c_str(),0);}
	bool windowExists(){return !!Renderer::window;}
	bool isConsoleMode(){return consoleMode;}

	unsigned int getTileXpos(){return tileXpos;}
	unsigned int getTileYpos(){return tileYpos;}

	SDL_Renderer* getSdlRenderer(){return renderer;}
	Nebulite::Invoke* getInvoke(){return invoke_ptr;}

	// Updated with each renderer update
	void update_rand() {invoke_ptr->getGlobalPointer()->set<Uint64>("rand",   dist(rngA));};

	// Updated with each renderer update and append
	void update_rrand(){invoke_ptr->getGlobalPointer()->set<Uint64>("rrand",  dist(rngB));};

	Nebulite::RenderObject* getObjectFromId(uint32_t id) {
		return env.getObjectFromId(id);
	}

	

private:
	//-------------------------------------------------------------------------------------
	// Boolean Status Variables
	bool reset_delta = false; 		// Reset delta values on next update
	bool audioInitialized = false;
	bool quit = false;
	bool consoleMode = false;
	bool showFPS = true;			// Set default to false later on

	//-------------------------------------------------------------------------------------
	// Audio
	SDL_AudioDeviceID audioDevice = 0;
	SDL_AudioSpec desired, obtained;
	const int frequency = 440;  // 440 Hz beep
	const int duration = 200;   // 200ms
	const int sampleRate = 44100;
	const int samples = (sampleRate * duration) / 1000;
	std::vector<Sint16>* sineBuffer = nullptr;
	std::vector<Sint16>* squareBuffer = nullptr;
	std::vector<Sint16>* triangleBuffer = nullptr;

	//-------------------------------------------------------------------------------------
	//General Variables
	std::vector<std::pair<std::string, std::string>> forced_global_values; // Key-Value pairs to set in global JSON

	std::string directory;
	uint32_t id_counter = 1;

	// Positions
	uint16_t tileXpos;
	uint16_t tileYpos;
    
	// Time
	TimeKeeper RendererLoopTime;	// Used for Simulation timing
	TimeKeeper RendererPollTime;	// Used for Polling timing
	TimeKeeper RendererFullTime;	// While Polling timer technically never stops, we use an extra timer just for full application time


	// Subclasses and pointers
	Environment env;
	Nebulite::Invoke* invoke_ptr = nullptr;	

	// Rendering
	unsigned int WindowScale = 1;
	SDL_Window* window;
	SDL_Renderer* renderer;
	TTF_Font* font;
	TTF_Font* consoleFont;
	absl::flat_hash_map<std::string, SDL_Texture*> TextureContainer;

	//SDL_Rect rect;
	SDL_Rect textRect;
	SDL_Rect consoleRect;
	SDL_Rect DstRect;

	// Events etc
	SDL_Event event;
	int MousePosX = 0;
	int MousePosY = 0;
	int lastMousePosX = 0;
	int lastMousePosY = 0;
	Uint32 lastMouseState;
	Uint32 mouseState;
	std::vector<Uint8> prevKeyState;
	

	//-------------------------------------------------------------------------------------
	// RNG
	std::mt19937 rngA;
    std::mt19937 rngB;
    std::size_t hashString(const std::string& str);
	std::uniform_int_distribution<int> dist;
    
	//-------------------------------------------------------------------------------------
	// Update Functions
	void clear();
	void renderFrame();
	void renderFrameNoThreads();
	void renderFPS(double scalar = 1.0);
	void showFrame();
	void pollEvent();
	SDL_Event getEventHandle();

	//-------------------------------------------------------------------------------------
	//For FPS Count

	// Define font properties
	SDL_Color textColor = { 255, 255, 255, 255 }; // White color
	int SCREEN_FPS = 500; // Target framerate (e.g., 60 FPS)
	int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS; // Milliseconds per frame
	Uint64 prevTicks = SDL_GetTicks64();
	Uint64 lastFPSRender = SDL_GetTicks64();
	Uint64 totalframes = 0;
	int fpsCount = 0;
	int fps = 0;
	

	//-------------------------------------------------------------------------------------
	// Console
	std::string consoleInputBuffer;                  // What the user is typing
	std::deque<std::string> consoleOutput;           // Optional: Past output log

	//-------------------------------------------------------------------------------------
	// Texture-Related

	// Function to load texture from file
	void loadTexture(std::string link);

	absl::flat_hash_map<Environment::Layers, absl::flat_hash_map<std::string, SDL_Texture*>> BetweenLayerTextures; // Textures that are attached between layers

};
}   // namespace Nebulite
