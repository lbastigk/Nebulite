#pragma once

#include <algorithm> //for std::remove_if
#include <thread>
#include "SDL.h"

#include "Invoke.h"
#include "NamenKonventionen.h"
#include "JSONHandler.h"

class MoveRuleSet;
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
	void subDocSet(std::string key,rapidjson::Document& subDoc);

	void appendInvoke(InvokeCommand toAppend);

	rapidjson::Document* getDoc() const;
	SDL_Rect& getDstRect();
	void calculateDstRect();
	SDL_Rect* getSrcRect();
	void calculateSrcRect();
	//-----------------------------------------------------------
	void update(Invoke* globalInvoke=nullptr);
	void exampleMoveSet(std::string val = namenKonvention.renderObject.positionX);
	
	//TODO
	bool hasMoveSet();

	void loadMoveSet(MoveRuleSet mrs);

private:
	rapidjson::Document doc;
	SDL_Rect dstRect;
	SDL_Rect srcRect;
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


class RenderObjectContainer {
public:
	//-----------------------------------------------------------
	//Constructor

	RenderObjectContainer();

	//-----------------------------------------------------------
	//Marshalling
	std::string serialize();
	void deserialize(const std::string& serialOrLink, int dispResX, int dispResY, int THREADSIZE);

	//-----------------------------------------------------------
	//-----------------------------------------------------------
	// Pipeline

	// Append objects
	void appendPtr(std::shared_ptr<RenderObject> ptr, int dispResX, int dispResY, int THREADSIZE);
	void append(RenderObject toAppend, int dispResX, int dispResY, int THREADSIZE);

	// Reinsert all objects into container, useful for new tile size declaration
	// e.g. new rendering size
	void reinsertAllObjects(int dispResX, int dispResY, int THREADSIZE);

	// Checks if given tile position contains objects
	bool isValidPosition(int x, int y) const;

	// removes all objects
	void purgeObjects();
	void purgeObjectsAt(int x, int y, int dispResX, int dispResY);	//at tile x,y

	// returns amount of objects
	size_t getObjectCount();
	size_t getObjectCountAtTile(int x, int y); //at tile x,y

	// Updating all objects in 3x3 Tile viewport
	// [ ][ ][ ]
	// [ ][P][ ] <- Player in Tile
	// [ ][ ][ ]
	void update_withThreads(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE,Invoke* globalInvoke=nullptr);
	void update(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE,Invoke* globalInvoke=nullptr,bool onlyRestructure = false);

	
	// Used to get a container Tile
	std::vector<std::vector<std::shared_ptr<RenderObject>>>& getContainerAt(int x, int y);
	
	// Returns a texture stored inside the container
	SDL_Texture* getTexture(int screenSizeX, int screenSizeY, SDL_Renderer *renderer, int tileXpos, int tileYpos, int Xpos, int Ypos, auto& TextureContainer);

private:
	//tileY		tileX		#			Batches		of RenderObject pointer
	std::vector<std::vector<std::vector<std::vector<std::shared_ptr<RenderObject>>>>> ObjectContainer;

};
