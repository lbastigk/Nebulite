#pragma once

#include "SDL.h"		// SDL Renderer is used for some methods to calculate text
#include "SDL_ttf.h"	// Same for ttf

#include "JSON.h"
#include "Invoke.h"
#include "KeyNames.h"

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

	// Updating renderobject based on global/local invokes
	void update(Nebulite::Invoke* globalInvoke);

	// Reloads all invokes from json doc into cmds_general and cmds_internal
	void reloadInvokes();

	// Calculates Text Texture from info in doc as well as renderer position
	void calculateText(SDL_Renderer* renderer,TTF_Font* font, int renderer_X, int renderer_Y);

	// Calculate sprite destination
	void calculateDstRect();

	// Calculate sprite source
	void calculateSrcRect();

	// Estimate computationcal cost of update (based on size of cmds_internal)
	uint64_t estimateCompuationalCost();

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
	std::vector<std::shared_ptr<Nebulite::Invoke::InvokeEntry>> cmds_general;	// Global commands, intended for self-other-global interaction
	std::vector<std::shared_ptr<Nebulite::Invoke::InvokeEntry>> cmds_internal;	// Internal commands, intended for self-global interaction
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


