/**
 * @file RenderObject.h
 * @brief Declaration of the RenderObject class.
 * 
 * Defines the Nebulite::Core::RenderObject class, which represents a renderable entity
 * in the Nebulite engine. This class encapsulates all data and logic needed to
 * display, update, and interact with a single object on the screen.
 */

#pragma once

#include "SDL.h"		// SDL Renderer is used for some methods to calculate text
#include "SDL_ttf.h"	// Same for ttf

#include "Constants/KeyNames.h"
#include "Interaction/Invoke.h"
#include "Interaction/Execution/RenderObjectTree.h"
#include "Utility/JSON.h"

namespace Nebulite {
namespace Core {
/**
 * @class Nebulite::Core::RenderObject
 * @brief Represents a renderable object in the Nebulite engine.
 *
 * Key Features:
 * 
 *   - Stores all render-related data in an internal JSON document for flexible property management.
 * 
 *   - Provides methods for serialization and deserialization to and from strings or files.
 * 
 *   - Supports dynamic property access and modification via valueSet/valueGet templates.
 * 
 *   - Manages SDL_Rect structures for sprite and text positioning, and caches these for performance.
 * 
 *   - Handles text rendering using SDL_ttf, including texture creation and caching.
 * 
 *   - Supports invoke command lists for both general (global) and internal (local) interactions.
 * 
 *   - Offers update and reload mechanisms to synchronize state with global invokes and JSON data.
 * 
 *   - Calculates computational cost estimates for update operations.
 * 
 *   - allows for flexible additions through the `RenderObjectTree` for local function calls.
 *
 * Interaction with Invoke:
 * 
 *   - Invokes are loaded in from JSON files and categorized into global and local commands.
 * 
 *   - Global commands are based on a self-other-global interaction model, while local commands are focused on self-global interactions.
 *     They are broadcasted to the Invoke class on defined topics to find matching objects that are listening to those topics.
 * 
 *   - Local commands are defined by an empty topic and are based on a self-global interaction model.
 *
 * Usage:
 *
 *   - Instantiate a RenderObject to represent a sprite, text, or other visual entity. Create a JSON document to hold its properties.
 *
 *   - Append the RenderObject to the Renderer via `./bin/Nebulite spawn myObject.json`
 *
 * All resource management (textures, surfaces) is handled internally. 
 * Copy and move operations are disabled to prevent accidental resource duplication.
 */
class RenderObject {
public:
	//-----------------------------------------------------------
	// Special member Functions

	/**
	 * @brief Constructs a new RenderObject.
	 * 
	 * RenderObjects are initialized with reference to the Global JSON document.
	 * This allows the object to compile any Nebulite::Interaction::Logic::Expression contained within,
	 * as they require remanent references for `self` and `global`.
	 */
	RenderObject(Nebulite::Utility::JSON* global);

	/**
	 * @brief Destroys the RenderObject.
	 * 
	 * Cleans up any resources used by the RenderObject, including
	 * textures and surfaces.
	 */
	~RenderObject();

	// Suppress copy/move operators
	RenderObject(const RenderObject& other) = delete;
	RenderObject(RenderObject&& other) = delete;
	RenderObject& operator=(RenderObject&& other) = delete;


	//-----------------------------------------------------------
	//Marshalling

	/**
	 * @brief Serializes the RenderObject to a JSON string.
	 * 
	 * @return A string representation of the RenderObject's JSON document.
	 */
	std::string serialize();

	/**
	 * @brief Deserializes the RenderObject from a JSON string.
	 * 
	 * @param serialOrLink The JSON string to deserialize.
	 */
	void deserialize(std::string serialOrLink);
	
	//-----------------------------------------------------------
	// Setting/Getting specific values

	/**
	 * @brief Sets a value in the RenderObject's JSON document.
	 * 
	 * @tparam T The type of the value to set.
	 * @param key The key of the value to set.
	 * @param data The value to set.
	 */
	template <typename T> void valueSet(const char* key, const T data);

	/**
	 * @brief Gets a value from the RenderObject's JSON document.
	 * 
	 * @tparam T The type of the value to get.
	 * @param key The key of the value to get.
	 * @param defaultValue The default value to return if the key is not found.
	 * @return The value from the JSON document, or the default value if not found.
	 */
	template <typename T> T valueGet(const char* key, const T& defaultValue = T());

	/**
	 * @brief Gets a pointer to the internal JSON document.
	 * 
	 * @return A pointer to the internal JSON document.
	 */
	Nebulite::Utility::JSON* getDoc() { return &json; }

	/**
	 * @brief Gets a pointer to the SDL_Rect describing the destination of the sprite.
	 * 
	 * @return A pointer to the SDL_Rect describing the destination of the sprite.
	 */
	SDL_Rect* getDstRect();

	/**
	 * @brief Gets a pointer to the SDL_Rect describing the source of the sprite.
	 * 
	 * @return A pointer to the SDL_Rect describing the source of the sprite.
	 */
	SDL_Rect* getSrcRect();

	/**
	 * @brief Gets a pointer to the SDL_Rect describing the destination of the text.
	 * 
	 * @return A pointer to the SDL_Rect describing the destination of the text.
	 */
	SDL_Rect* getTextRect();

	/**
	 * @brief Gets the texture of the text.
	 * 
	 * @return A pointer to the SDL_Texture representing the text.
	 */
	SDL_Texture* getTextTexture();
	
	//-----------------------------------------------------------
	// Update-Oriented functions

	// Updating renderobject based on global/local invokes
	/**
	 * @brief Updates the RenderObject based on the given global invoke.
	 * 
	 * - updates the internal FuncTrees
	 * 
	 * - reloads invokes if needed
	 * 
	 * - updates local invokes
	 * 
	 * - listens to global invokes
	 * 
	 * - broadcasts its own global invokes
	 * 
	 * - calculates source and destination rects
	 */
	void update(Nebulite::Interaction::Invoke* globalInvoke);

	/**
	 * @brief Calculates the text texture for the RenderObject.
	 * 
	 * @param renderer The SDL_Renderer to use for rendering.
	 * @param font The TTF_Font to use for rendering the text.
	 * @param renderer_X The X position of the renderer used for text offset.
	 * @param renderer_Y The Y position of the renderer used for text offset.
	 */
	void calculateText(SDL_Renderer* renderer,TTF_Font* font, int renderer_X, int renderer_Y);

	/**
	 * @brief Calculates the destination rectangle for the sprite.
	 */
	void calculateDstRect();

	// Calculate sprite source
	/**
	 * @brief Calculates the source rectangle for the sprite.
	 */
	void calculateSrcRect();

	// Estimate computationcal cost of update (based on size of cmds_internal)
	/**
	 * @brief Estimates the computational cost of updating the RenderObject.
	 * 
	 * Based on the amount of evaluations and variables in the invoke entries.
	 */
	uint64_t estimateComputationalCost(Nebulite::Interaction::Invoke* globalInvoke);

	// For internal string parsing in RenderObjectTree
	// IMPORTANT: Make sure the first arg is a name and not the function itself!
	// e.g. parseStr("set text.str Hello World") -> does not work
	// e.g. parseStr("<someName> set text.str Hello World") -> works
	/**
	 * @brief Parses a string into a Nebulite command.
	 * 
	 * @param str The string to parse.
	 * @return Potential errors that occured on command execution
	 */
	Nebulite::Constants::ERROR_TYPE parseStr(const std::string& str);

	//-----------------------------------------------------------
	// Management Flags for Renderer-Interaction

	/**
	 * @struct flag
	 * @brief Flags for managing RenderObject behavior
	 */
	struct flag{
		bool deleteFromScene = false;	// If true, delete this object from scene on next update
		bool calculateText = false;		// If true, calculate text texture on next update
		bool reloadInvokes = false;		// If true, reload invokes on next update
	} flag;
private:
	// Main doc holding values
	Nebulite::Utility::JSON json;

	// for caching of SDL Positions
	SDL_Rect dstRect;	// destination of sprite
	SDL_Rect srcRect;	// source of sprite from spritesheet
	SDL_Rect textRect;	// destination of text texture

	// Surface and Texture of Text
	SDL_Surface* textSurface;	// Surface for the text
	SDL_Texture* textTexture;	// Texture for the text

	// Invoke Commands
	std::vector<std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry>> entries_global;		// Global commands, intended for self-other-global interaction
	std::vector<std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry>> entries_local;		// Internal commands, intended for self-global interaction

	// RenderObjectTree for local function calls
	Nebulite::Interaction::Execution::RenderObjectTree renderObjectTree;

	// Each RenderObject with linkage to the global document for copy/move instructions in the renderObjectTree
	Nebulite::Utility::JSON* global;
};
} // namespace Core
} // namespace Nebulite

//-----------------------------------------------------------
// Setting/Getting specific values

template <typename T> void Nebulite::Core::RenderObject::valueSet(const char* key, const T data) {
	//JSONHandler::Set::Any(doc, key, data);
	json.set(key,data);
	calculateDstRect();
	calculateSrcRect();
}

template <typename T> T Nebulite::Core::RenderObject::valueGet(const char* key, const T& defaultValue){
	return json.get<T>(key,defaultValue);
}


