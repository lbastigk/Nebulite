/*
 * RenderObject.h
 * --------------
 * Defines the Nebulite::RenderObject class, which represents a renderable entity
 * in the Nebulite engine. This class encapsulates all data and logic needed to
 * display, update, and interact with a single object on the screen.
 *
 * Key Features:
 *   - Stores all render-related data in an internal JSON document for flexible property management.
 *   - Provides methods for serialization and deserialization to and from strings or files.
 *   - Supports dynamic property access and modification via valueSet/valueGet templates.
 *   - Manages SDL_Rect structures for sprite and text positioning, and caches these for performance.
 *   - Handles text rendering using SDL_ttf, including texture creation and caching.
 *   - Supports invoke command lists for both general (global) and internal (local) interactions.
 *   - Offers update and reload mechanisms to synchronize state with global invokes and JSON data.
 *   - Calculates computational cost estimates for update operations.
 *   - allows for flexible additions through the RenderObjectTree for local function calls.
 *
 * Interaction with Invoke:
 *   - Invokes are loaded in from JSON files and categorized into global and local commands.
 *   - Global commands are based on a self-other-global interaction model, while local commands are focused on self-global interactions.
 *     They are broadcasted to the Invoke class on defined topics to find matching objects that are listening to those topics.
 *   - Local commands are defined by an empty topic and are based on a self-global interaction model.
 *
 * Usage:
 *   - Instantiate a RenderObject to represent a sprite, text, or other visual entity.
 *   - Use valueSet/valueGet to modify or query properties.
 *   - Call update() and reloadInvokes() to synchronize with global state or after property changes.
 *   - Use calculateText() to update text textures when text or font properties change.
 *   - Access SDL_Rect and SDL_Texture pointers for integration with SDL rendering routines.
 *   - See RenderObjectTree for managing local function calls and interactions.
 *
 * All resource management (textures, surfaces) is handled internally. Copy and move operations
 * are disabled to prevent accidental resource duplication.
 */

#pragma once

#include "SDL.h"		// SDL Renderer is used for some methods to calculate text
#include "SDL_ttf.h"	// Same for ttf

#include "JSON.h"
#include "Invoke.h"
#include "KeyNames.h"
#include "RenderObjectTree.h"

namespace Nebulite{
class RenderObject {
public:
	//-----------------------------------------------------------
	// Special member Functions

	RenderObject();
	~RenderObject();

	// Suppress copy/move operators
	RenderObject(const RenderObject& other) = delete;
	RenderObject(RenderObject&& other) = delete;
	RenderObject& operator=(RenderObject&& other) = delete;


	//-----------------------------------------------------------
	//Marshalling

	// Serialize object to string
	std::string serialize();

	// Deserialize object from string
	void deserialize(std::string serialOrLink);
	
	//-----------------------------------------------------------
	// Setting/Getting specific values

	// Setting any value into json doc
	template <typename T> void valueSet(const char* key, const T data);

	// Getting any value from json doc
	template <typename T> T valueGet(const char* key, const T& defaultValue = T());

	// Returns pointer to internal json doc
	Nebulite::JSON* getDoc(){return &json;};

	// Returns pointer to internal json doc as const
	const Nebulite::JSON* getDoc() const { return &json; }

	// Get SDL_Rect describing destination of sprite
	SDL_Rect* getDstRect();

	// Get SDL rect describing source of sprite
	SDL_Rect* getSrcRect();

	// Get SDL_Rect describing destination of text
	SDL_Rect* getTextRect();

	// Get texture of text
	SDL_Texture* getTextTexture();
	
	//-----------------------------------------------------------
	// Update-Oriented functions

	// Updating renderobject based on global/local invokes
	void update(Nebulite::Invoke* globalInvoke);

	// Calculates Text Texture from info in doc as well as renderer position
	void calculateText(SDL_Renderer* renderer,TTF_Font* font, int renderer_X, int renderer_Y);

	// Calculate sprite destination
	void calculateDstRect();

	// Calculate sprite source
	void calculateSrcRect();

	// Estimate computationcal cost of update (based on size of cmds_internal)
	uint64_t estimateComputationalCost();

	// For internal string parsing in RenderObjectTree
	// IMPORTANT: Make sure the first arg is a name and not the function itself!
	// e.g. parseStr("set text.str Hello World") -> does not work
	// e.g. parseStr("<someName> set text.str Hello World") -> works
	Nebulite::ERROR_TYPE parseStr(const std::string& str);

	//-----------------------------------------------------------
	// Management Flags for Renderer-Interaction
	struct flag{
		bool deleteFromScene = false;	// If true, delete this object from scene on next update
		bool calculateText = false;		// If true, calculate text texture on next update
		bool reloadInvokes = false;		// If true, reload invokes on next update
	} flag;
private:
	// Main doc holding values
	Nebulite::JSON json;

	// for caching of SDL Positions
	SDL_Rect dstRect;	// destination of sprite
	SDL_Rect srcRect;	// source of sprite from spritesheet
	SDL_Rect textRect;	// destination of text texture

	// Surface and Texture of Text
	SDL_Surface* textSurface;
	SDL_Texture* textTexture;

	// Invoke Commands
	std::vector<std::shared_ptr<Nebulite::Invoke::Entry>> entries_global;	// Global commands, intended for self-other-global interaction
	std::vector<std::shared_ptr<Nebulite::Invoke::Entry>> entries_local;	// Internal commands, intended for self-global interaction

	// RenderObjectTree for local function calls
	Nebulite::RenderObjectTree renderObjectTree;
};
}

//-----------------------------------------------------------
// Setting/Getting specific values

template <typename T> void Nebulite::RenderObject::valueSet(const char* key, const T data) {
	//JSONHandler::Set::Any(doc, key, data);
	json.set(key,data);
	calculateDstRect();
	calculateSrcRect();
}

template <typename T> T Nebulite::RenderObject::valueGet(const char* key, const T& defaultValue){
	return json.get<T>(key,defaultValue);
}


