#pragma once

#include "SDL.h"		// SDL Renderer is used for some methods to calculate text
#include <SDL_ttf.h>	// Same for ttf

#include "JSON.h"
#include "Invoke.h"
#include "NamenKonventionen.h"
#include "JSONHandler.h"

class RenderObject {
public:
	//-----------------------------------------------------------
	//Constructor
	RenderObject();
	RenderObject(const RenderObject& other);
	RenderObject& operator=(const RenderObject& other);

	//-----------------------------------------------------------
	//Marshalling
	std::string serialize();
	void deserialize(std::string serialOrLink);
	
	//-----------------------------------------------------------
	// Setting/Getting specific values
	template <typename T> void valueSet(const char* key, const T data);
	template <typename T> T valueGet(const char* key, const T& defaultValue = T());


	Nebulite::JSON* getDoc(){return &json;};
	rapidjson::Document* _getDoc() const;
	SDL_Rect& getDstRect();
	void calculateDstRect();
	SDL_Rect* getSrcRect();
	void calculateSrcRect();
	SDL_Texture& getTextTexture();
	void calculateText(SDL_Renderer* renderer,TTF_Font* font, int renderer_X, int renderer_Y);
	SDL_Rect* getTextRect();
	//-----------------------------------------------------------
	void update(Invoke* globalInvoke,std::shared_ptr<RenderObject> this_shared);
	void exampleMoveSet(std::string val = namenKonvention.renderObject.positionX);

	void reloadInvokes(std::shared_ptr<RenderObject> this_shared);

private:
	Nebulite::JSON json;
	SDL_Rect dstRect;
	SDL_Rect srcRect;

	SDL_Surface* textSurface;
	SDL_Texture* textTexture;
	SDL_Rect textRect;

	std::vector<std::shared_ptr<Invoke::InvokeEntry>> cmds_general;
	std::vector<std::shared_ptr<Invoke::InvokeEntry>> cmds_internal;
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
	//return JSONHandler::Get::Any<T>(doc, key, defaultValue);
}


