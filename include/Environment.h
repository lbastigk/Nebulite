#pragma once

#include <iostream>
#include <string>
#include "absl/container/flat_hash_map.h"
#include <utility>
#include <vector>


#include "RenderObjectContainer.h"
#include "Invoke.h"
#include "JSON.h"



#define RENDEROBJECTCONTAINER_COUNT 5

namespace Nebulite{
class Environment {
public:
	//-----------------------------------------------------------
	//Constructor

	Environment();
	Environment(const Environment& other);

	// Suppress copy/move operators
	Environment(Environment&& other) = delete;
	Environment& operator=(Environment&& other) = delete;
	Environment& operator=(const Environment& other) = delete;

	//-----------------------------------------------------------
	//Marshalling
	std::string serialize();


	void deserialize(std::string serialOrLink, int dispResX,int dispResY);
	
	std::string serializeGlobal(){
		return global->serialize();
	}

	void linkGlobal(Nebulite::JSON& global_ptr){global = &global_ptr;};
	
	void append(Nebulite::RenderObject* toAppend,int dispResX, int dispResY, int layer = 0);
	void update(int16_t tileXpos, int16_t tileYpos,int dispResX,int dispResY,Nebulite::Invoke* globalInvoke=nullptr);
	void reinsertAllObjects(int dispResX,int dispResY);

	std::vector<Nebulite::RenderObjectContainer::batch>& getContainerAt(int16_t x, int16_t y, int layer);
	bool isValidPosition(int x, int y, int layer);

	void purgeObjects();
	void purgeObjectsAt(int x, int y, int dispResX, int dispResY);
	void purgeLayer(int layer);

	size_t getObjectCount(bool excludeTopLayer = true);

	enum RenderObjectLayers {
			background,
			general,
			foreground,
			effects,
			menue
	};

	Nebulite::JSON& getGlobal() {
		return *global;
	}

	Nebulite::RenderObject* getObjectFromId(uint32_t id) {
		// Go through all layers
		for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; ++i) {
			auto obj = roc[i].getObjectFromId(id);
			if (obj != nullptr) {
				return obj;
			}
		}
		return nullptr;
	}

private:
	// Link to Global Values
    Nebulite::JSON* global;
	
	Nebulite::RenderObjectContainer roc[RENDEROBJECTCONTAINER_COUNT];
};
}