#pragma once

#include <thread>

#include "NamenKonventionen.h"
#include "JSONHandler.h"
#include "SDL.h"
#include "MoveRuleSet.h"

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
	rapidjson::Document* getDoc() const;
	SDL_Rect& getDstRect();
	void calculateDstRect();
	SDL_Rect* getSrcRect();
	void calculateSrcRect();
	//-----------------------------------------------------------
	void update();
	void loadMoveSet(MoveRuleSet mrs);
	void exampleMoveSet(std::string val = namenKonvention.renderObject.positionX);
	//TODO
	bool hasMoveSet();
private:
	rapidjson::Document doc;
	SDL_Rect dstRect;
	SDL_Rect srcRect;
};

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
	void append(RenderObject toAppend, int dispResX, int dispResY, int THREADSIZE);
	void update_withThreads(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE);
	void update(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE);
	bool isValidPosition(int x, int y) const;
	auto& getContainerAt(int x, int y);
	void purgeObjects();
	size_t getObjectCount();

private:
	//tileY		tileX		#			Batches		of Objects
	std::vector<std::vector<std::vector<std::vector<RenderObject>>>> ObjectContainer;
};

/*
COPY TEMPLATE FROM ROC

class copyTemplates {

		void forEachObject(std::vector<std::vector<std::vector<std::vector<RenderObject>>>> ObjectContainer) {
			for (auto& vec1 : ObjectContainer) {
				for (auto& vec2 : vec1) {
					for (auto& batch : vec2) {
						batch.clear(); // Assuming RenderObject doesn't need explicit cleanup
					}
				}
			}
		}
	};
*/