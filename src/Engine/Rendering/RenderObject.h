#pragma once

#include "SDL.h"		// SDL Renderer is used for some methods to calculate text
#include <SDL_ttf.h>	// Same for ttf

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
	//Destructor
	~RenderObject();

	//-----------------------------------------------------------
	//Marshalling
	std::string serialize();
	void deserialize(std::string serialOrLink);
	
	//-----------------------------------------------------------
	// Setting/Getting specific values
	template <typename T> void valueSet(std::string key, const T data);
	template <typename T> T valueGet(std::string key, const T& defaultValue = T());

	void appendInvoke(InvokeCommand toAppend);

	rapidjson::Document* getDoc() const;
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
	rapidjson::Document doc;
	SDL_Rect dstRect;
	SDL_Rect srcRect;

	SDL_Surface* textSurface;
	SDL_Texture* textTexture;
	SDL_Rect textRect;

	std::vector<std::shared_ptr<InvokeEntry>> cmds_general;
	std::vector<std::shared_ptr<InvokeEntry>> cmds_internal;
};

//-----------------------------------------------------------
// Setting/Getting specific values
template <typename T> void RenderObject::valueSet(std::string key, const T data) {
	JSONHandler::Set::Any(doc, key, data);
	calculateDstRect();
	calculateSrcRect();
}

template <typename T> T RenderObject::valueGet(std::string key, const T& defaultValue){
	return JSONHandler::Get::Any<T>(doc, key, defaultValue);
}


