/**
 * @file Renderer.h
 *
 * This file contains the declaration of the Nebulite::Core::Renderer class, which is responsible for rendering
 * the game objects and managing the rendering pipeline.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include <thread>
#include <random>
#include <stdint.h>

// External
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "absl/container/flat_hash_map.h"

// Nebulite
#include "Core/Environment.h"
#include "Utility/FileManagement.h"
#include "Utility/Time.h"
#include "Utility/TimeKeeper.h"
#include "Interaction/Invoke.h"

//------------------------------------------
namespace Nebulite{
namespace Core {
/**
 * @class Nebulite::Core::Renderer
 * 
 * @brief Responsible for rendering game objects and managing the rendering pipeline.
 */
class Renderer {
public:
	/**
	 * @brief Initializes a Renderer with given dimensions and settings.
	 * 
	 * @param invoke Reference to the Nebulite::Interaction::Invoke instance.
	 * @param global Reference to the global Nebulite::Utility::JSON instance.
	 * @param flag_headless Boolean flag for headless mode.
	 * @param X Width of the rendering area.
	 * @param Y Height of the rendering area.
	 */
	Renderer(Nebulite::Interaction::Invoke& invoke, Nebulite::Utility::JSON& global, bool flag_headless = false, unsigned int X = 1080, unsigned int Y = 1080);

	//------------------------------------------
	// Serialization / Deserialization

	/**
	 * @brief Serializes the current state of the Renderer.
	 * 
	 * @return A JSON string representing the Renderer state.
	 * 
	 * @todo Consider serializing render-specific variables if needed.
	 * Currently, this only serializes the environment state.
	 * In actuality, we would need to also serialize the global document state.
	 */
	std::string serialize(){
		return env.serialize();
	}

	/**
	 * @brief Deserializes the Renderer state from a JSON string or link.
	 * 
	 * @param serialOrLink The JSON string or link to deserialize.
	 */
	void deserialize(std::string serialOrLink){
		env.deserialize(
			serialOrLink, 
			invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0), 
			invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0)
		);
	}

	//------------------------------------------
	// Pipeline

	/**
	 * @brief Updates the renderer for the next frame.
	 * 
	 * - clears the screen
	 * 
	 * - calls the general update function
	 * 
	 * - renders frame
	 * 
	 * - renders fps, if enabled
	 * 
	 * - presents the frame
	 */
	void tick();

	/**
	 * @brief Checks if it's time to render the next frame based on the target FPS.
	 * 
	 * @todo use custom TimeKeeper class instead for FPS target tracking
	 */
	bool timeToRender();

	/**
	 * @brief Appends a RenderObject to the Renderer.
	 * 
	 * This function adds a RenderObject to the rendering pipeline.
	 * 
	 * - Sets id of the RenderObject.
	 * 
	 * - Increases the id counter.
	 * 
	 * - Appends the RenderObject to the environment.
	 * 
	 * - Loads its texture
	 * 
	 * - updates the rolling random number generator.
	 * 
	 * @param toAppend Pointer to the RenderObject to append.
	 */
	void append(Nebulite::Core::RenderObject* toAppend);

	/**
	 * @brief Reinserts all objects into the rendering pipeline.
	 * 
	 * Does not change the id counter.
	 */
	void reinsertAllObjects();

	/**
	 * @brief Sets global values in the Nebulite::Utility::JSON global document
	 * 
	 * - time
	 * 
	 * - framecount
	 * 
	 * - runtime
	 * 
	 * - RNG
	 * 
	 * Does NOT set keyboard/mouse values
	 */
	void setGlobalValues();

	/**
	 * @brief Checks if the Renderer is in a quit state.
	 * 
	 * @return True if the Renderer is set to quit, false otherwise.
	 */
	bool isQuit(){return quit;}

	/**
	 * @brief Sets the quit state of the Renderer.
	 */
	void setQuit(){quit=true;}

	/**
	 * @brief Attaches a texture above a specific layer.
	 * 
	 * @param aboveThisLayer The layer above which to attach the texture.
	 * @param name The name of the texture.
	 * @param texture The SDL_Texture to attach.
	 * @return True if the texture was successfully attached, false otherwise.
	 */
	bool attachTextureAboveLayer(Environment::Layers aboveThisLayer, std::string name, SDL_Texture* texture) {
		if(texture == nullptr) {
			return false; // Cannot attach a null texture
		}

		BetweenLayerTextures[aboveThisLayer][name] = texture;
		return true;
	}

	/**
	 * @brief Removes a texture from between layers.
	 * 
	 * @param name The name of the texture to remove.
	 * @return True if the texture was successfully removed, false otherwise.
	 */
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

	//------------------------------------------
	// Special Functions

	/**
	 * @brief Beeps the system speaker.
	 */
	void beep();

	/**
	 * @brief Takes a snapshot of the current Renderer state.
	 * 
	 * @param link The link to save the snapshot to.
	 * 
	 * @return True if the snapshot was successful, false otherwise.
	 * 
	 * @todo Snapshot not working on windows via wine.
	 * Image has right dimensions, but is black.
	 * Nebulite Linux Release and Debug work fine.
	 * Nebulite Windows Release and Debug are broken.
	 */
	bool snapshot(std::string link);
	
	//------------------------------------------
	// Purge

	/**
	 * @brief Purges all objects from the Renderer.
	 */
	void purgeObjects();

	/**
	 * @brief Purges all textures from the Renderer.
	 */
	void purgeTextures();

	/**
	 * @brief Destroys the Renderer and all associated resources.
	 */
	void destroy();
	
	//------------------------------------------
	// Manipulation

	/**
	 * @brief Changes the window size.
	 * 
	 * @param w The new width of the window.
	 * @param h The new height of the window.
	 * @param scalar The scaling factor to apply.
	 */
	void changeWindowSize(int w, int h, int scalar);

	/**
	 * @brief Moves the camera by a certain amount.
	 * 
	 * @param dX The amount to move the camera in the X direction.
	 * @param dY The amount to move the camera in the Y direction.
	 */
	void moveCam(int dX, int dY);

	/**
	 * @brief Sets the camera position.
	 * 
	 * @param X The new X position of the camera.
	 * @param Y The new Y position of the camera.
	 * @param isMiddle If true, the (x,y) coordinates relate to the middle of the screen.
	 * If false, they relate to the top left corner.
	 */
	void setCam(int X, int Y, bool isMiddle = false);

	/**
	 * @brief Sets a forced global value.
	 * 
	 * All forced global values overwrite any other actions on that value.
	 * 
	 * @param key The key of the global value to set.
	 * @param value The value to set.
	 */
	void setForcedGlobalValue(const std::string& key, const std::string& value) {
		forced_global_values.emplace_back(key, value);
	}

	/**
	 * @brief Clears all forced global values.
	 */
	void clearForcedGlobalValues() {
		forced_global_values.clear();
	}
	
	//------------------------------------------
	// Setting

	/**
	 * @brief Toggles the display of the FPS counter.
	 */
	void toggleFps(bool show = true){
		showFPS = show;
	}

	/**
	 * @brief Sets the target FPS for the renderer.
	 */
	void setTargetFPS(int fps);

	
	//------------------------------------------
	// Getting

	/**
	 * @brief Gets the amount of textures currently loaded.
	 * 
	 * @return The number of textures.
	 */
	size_t getTextureAmount(){return TextureContainer.size();}

	/**
	 * @brief Gets the amount of RenderObjects currently loaded.
	 * 
	 * @return The number of RenderObjects in the environment.
	 */
	size_t getObjectCount(){return env.getObjectCount();}

	/**
	 * @brief Gets the current resolution in the X direction.
	 * 
	 * @return The current resolution in the X direction.
	 */
	int getResX(){return invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0);}

	/**
	 * @brief Gets the current resolution in the Y direction.
	 * 
	 * @return The current resolution in the Y direction.
	 */
	int getResY(){return invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0);}

	/**
	 * @brief Gets the current FPS.
	 * 
	 * @return The current FPS.
	 */
	int getFPS(){return REAL_FPS;}

	/**
	 * @brief Gets the current position of the camera in the X direction.
	 * 
	 * The position is considered to be the top left corner of the screen.
	 * 
	 * @return The current position of the camera in the X direction.
	 */
	int getPosX(){return invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.positionX.c_str(),0);}

	/**
	 * @brief Gets the current position of the camera in the Y direction.
	 * 
	 * The position is considered to be the top left corner of the screen.
	 * 
	 * @return The current position of the camera in the Y direction.
	 */
	int getPosY(){return invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.positionY.c_str(),0);}

	/**
	 * @brief Checks if the console is currently in use.
	 * 
	 * @return True if the console is in use, false otherwise.
	 */
	bool isConsoleMode(){return consoleMode;}

	/**
	 * @brief Gets the current tile position of the camera in the X direction.
	 * 
	 * The position to check for tile position is considered to be the top left corner of the screen.
	 * 
	 * @return The current tile position of the camera in the X direction.
	 */
	unsigned int getTileXpos(){return tileXpos;}

	/**
	 * @brief Gets the current tile position of the camera in the Y direction.
	 * 
	 * The position to check for tile position is considered to be the top left corner of the screen.
	 * 
	 * @return The current tile position of the camera in the Y direction.
	 */
	unsigned int getTileYpos(){return tileYpos;}

	/**
	 * @brief Gets the SDL_Renderer instance.
	 * 
	 * Allows for access to the underlying SDL renderer for custom rendering operations.
	 * 
	 * @return The SDL_Renderer instance.
	 */
	SDL_Renderer* getSdlRenderer(){return renderer;}

	/**
	 * @brief Gets the Invoke instance.
	 * 
	 * Allows for access to the underlying Invoke instance for custom operations.
	 * 
	 */
	Nebulite::Interaction::Invoke* getInvoke(){return invoke_ptr;}

	// Updated with each renderer update
	/**
	 * @brief Updates the random number generator state.
	 */
	void update_rand() {invoke_ptr->getGlobalPointer()->set<Uint64>("rand",   dist(rngA));};

	/**
	 * @brief Updates the rolling random number generator state.
	 * 
	 * Is updated with each renderer update and append.
	 */
	void update_rrand(){invoke_ptr->getGlobalPointer()->set<Uint64>("rrand",  dist(rngB));};

	/**
	 * @brief Gets the RenderObject from its ID.
	 * 
	 * @param id The ID of the RenderObject to retrieve.
	 * @return A pointer to the RenderObject, or nullptr if not found.
	 */
	Nebulite::Core::RenderObject* getObjectFromId(uint32_t id) {
		return env.getObjectFromId(id);
	}

private:

	//------------------------------------------
	// Boolean Status Variables
	bool reset_delta = false; 		// Reset delta values on next update
	bool audioInitialized = false;
	bool quit = false;
	bool consoleMode = false;
	bool showFPS = true;			// Set default to false later on

	//------------------------------------------
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

	//------------------------------------------
	//General Variables
	std::vector<std::pair<std::string, std::string>> forced_global_values; // Key-Value pairs to set in global JSON
	std::string baseDirectory;
	uint32_t renderobject_id_counter = 1;

	// Positions
	uint16_t tileXpos;
	uint16_t tileYpos;

	// Timekeeper
	Nebulite::Utility::TimeKeeper RendererLoopTime;	// Used for Simulation timing
	Nebulite::Utility::TimeKeeper RendererPollTime;	// Used for Polling timing
	Nebulite::Utility::TimeKeeper RendererFullTime;	// While Polling timer technically never stops, we use an extra timer just for full application time

	// Custom Subclasses
	Environment env;
	Nebulite::Interaction::Invoke* invoke_ptr = nullptr;	

	// Rendering
	unsigned int WindowScale = 1;
	SDL_Window* window;
	SDL_Renderer* renderer;

	//SDL_Rect rect;
	SDL_Rect textRect;
	SDL_Rect consoleRect;
	SDL_Rect DstRect;

	//------------------------------------------
	// Event Handling

	SDL_Event event;
	int MousePosX = 0;
	int MousePosY = 0;
	int lastMousePosX = 0;
	int lastMousePosY = 0;
	Uint32 lastMouseState;
	Uint32 mouseState;
	std::vector<Uint8> prevKeyState;

	/**
	 * @brief Gets the current SDL event.
	 * 
	 * @return The current SDL event.
	 * 
	 * @todo this function is garbage, just using SDL_PollEvent on a global variable would be cleaner.
	 */
	SDL_Event getEventHandle();

	/**
	 * @brief Polls Keyboard and mouse states, sets forced global values.
	 * 
	 * @todo Move to separate GTE-File 
	 * hashmap for key names instead of constant polling?
	 * This also ensures cross-platform stability, note that SDL_GetScancodeName is not cross-platform stable!!!
	 * Manual map is therefore necessary
	 */
	void pollEvent();
	
	//------------------------------------------
	// RNG
	std::mt19937 rngA;
    std::mt19937 rngB;

	/**
	 * @brief Hashes a string to produce a size_t value.
	 * 
	 * Used for RNG seeding.
	 */
    std::size_t hashString(const std::string& str){return std::hash<std::string>{}(str);};

	/**
	 * @brief Used to limit RNG output to fit uint16_t.
	 */
	std::uniform_int_distribution<int> dist;
    
	//------------------------------------------
	// Renderer::tick Functions

	/**
	 * @brief Clears the Renderer screen
	 * 
	 * This function clears renderer to an all black screen.
	 */
	void clear();

	/**
	 * @brief Updates the Renderer state.
	 * 
	 * - updates timer
	 * 
	 * - polls SDL events
	 * 
	 * - polls mouse and keyboard state
	 * 
	 * - checks for console mode and updates its state
	 * 
	 * - sets global values
	 * 
	 * - updates the invoke instance
	 * 
	 * - updates the environment
	 */
	void updateState();

	/**
	 * @brief Renders the current frame.
	 * 
	 * Takes all RenderObjects potentially visible in the current frame and renders them.
	 * See the Environment and RenderObjectContainer class for more details on how objects 
	 * are managed in the tile-based-container
	 */
	void renderFrame();

	/**
	 * @brief Renders the current FPS on screen
	 * 
	 * @param scalar Scaling factor for the FPS text size.
	 */
	void renderFPS(double scalar = 1.0);

	/**
	 * @brief Presents the rendered frame to the screen.
	 */
	void showFrame();

	//------------------------------------------
	//For FPS Count

	uint64_t prevTicks = SDL_GetTicks64();
	uint64_t lastFPSRender = SDL_GetTicks64();
	uint64_t totalframes = 0;

	uint16_t TARGET_FPS = 500; 								// Target framerate (e.g., 60 FPS)
	uint16_t TARGET_TICKS_PER_FRAME = 1000 / TARGET_FPS; 	// Milliseconds per frame
	uint16_t REAL_FPS_COUNTER = 0;							// Counts fps in a 1-second-interval
	uint16_t REAL_FPS = 0;									// Actual fps this past second
	
	//------------------------------------------
	// Console
	std::string consoleInputBuffer;                  // What the user is typing
	std::deque<std::string> consoleOutput;           // Optional: Past output log

	//------------------------------------------
	// Texture-Related

	// Function to load texture from file
	/**
	 * @brief Loads a texture into the TextureContainer
	 * 
	 * Creates the necessary surface and texture object from a given file path.
	 * 
	 * @param link The file path to load the texture from.
	 */
	void loadTexture(std::string link);

	/**
	 * @brief Texture container for the Renderer
	 * 
	 * This container holds all loaded textures from RenderObject sprites for the renderer, allowing for easy access and management.
	 * 
	 * `TextureContainer[link] -> SDL_Texture*`
	 */
	absl::flat_hash_map<std::string, SDL_Texture*> TextureContainer;

	/**
	 * @brief Contains textures the renderer needs to render between layers
	 * 
	 * `BetweenLayerTextures[layer][link] -> SDL_Texture*`
	 */
	absl::flat_hash_map<Environment::Layers, absl::flat_hash_map<std::string, SDL_Texture*>> BetweenLayerTextures;

	//------------------------------------------
	// Font-Related

	// Define font properties
	SDL_Color textColor = { 255, 255, 255, 255 }; // White color

	// General font
	TTF_Font* font;

	// Font for console text
	TTF_Font* consoleFont;

	/**
	 * @brief Loads fonts for the Renderer.
	 */
	void loadFonts();
};
} 	// namespace Core
}   // namespace Nebulite
