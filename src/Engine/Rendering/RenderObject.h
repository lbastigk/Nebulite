#pragma once

#include "SDL.h"		// SDL Renderer is used for some methods to calculate text
#include <SDL_ttf.h>	// Same for ttf

#include "JSON.h"
#include "Invoke.h"
#include "KeyNames.h"

class RenderObject {
public:
	//-----------------------------------------------------------
	//Constructor
	RenderObject();
	RenderObject(const RenderObject& other);
	RenderObject& operator=(RenderObject& other);



	//-----------------------------------------------------------
	//Marshalling
	std::string serialize();
	void deserialize(std::string serialOrLink);
	
	//-----------------------------------------------------------
	// Setting/Getting specific values
	template <typename T> void valueSet(const char* key, const T data);
	template <typename T> T valueGet(const char* key, const T& defaultValue = T());
	Nebulite::JSON* getDoc(){return &json;};
	const Nebulite::JSON* getDoc() const { return &json; }
	SDL_Rect& getDstRect();
	SDL_Rect* getSrcRect();
	SDL_Texture* getTextTexture();
	SDL_Rect* getTextRect();
	//-----------------------------------------------------------
	void update(Nebulite::Invoke* globalInvoke,std::shared_ptr<RenderObject> this_shared);
	void reloadInvokes(std::shared_ptr<RenderObject> this_shared);

	void calculateText(SDL_Renderer* renderer,TTF_Font* font, int renderer_X, int renderer_Y);
	void calculateDstRect();
	void calculateSrcRect();

private:
	// Main doc holding values
	Nebulite::JSON json;

	// for caching of SDL Positions
	SDL_Rect dstRect;
	SDL_Rect srcRect;
	SDL_Rect textRect;

	// Surface and Texture of Text
	SDL_Surface* textSurface;
	SDL_Texture* textTexture;

	// Invoke Commands
	std::vector<std::shared_ptr<Nebulite::Invoke::InvokeEntry>> cmds_general;		// Global
	std::vector<std::shared_ptr<Nebulite::Invoke::InvokeEntry>> cmds_internal;	// Internal
};

//-----------------------------------------------------------
// Setting/Getting specific values
template <typename T> void RenderObject::valueSet(const char* key, const T data) {
	//JSONHandler::Set::Any(doc, key, data);
	json.set(key,data);
	calculateDstRect();
	calculateSrcRect();
}

template <typename T> T RenderObject::valueGet(const char* key, const T& defaultValue){
	return json.get<T>(key,defaultValue);
}


