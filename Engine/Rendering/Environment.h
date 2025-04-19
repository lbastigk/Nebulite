#pragma once

#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <vector>

#include "JSONHandler.h"
#include "RenderObject.h"
#include "Invoke.h"



#define RENDEROBJECTCONTAINER_COUNT 5

class Environment {
public:
	//-----------------------------------------------------------
	//Constructor

	Environment();
	Environment(const Environment& other);

	//-----------------------------------------------------------
	//Destructor
	~Environment();

	//-----------------------------------------------------------
	//Marshalling
	std::string serializeOld();

	std::string serialize();


	void deserialize(std::string serialOrLink, int dispResX,int dispResY,int THREADSIZE);
	
	
	void append(RenderObject toAttach,int dispResX, int dispResY,int THREADSIZE, int layer = 0);
	void update(int tileXpos,int tileYpos,int dispResX,int dispResY, int THREADSIZE,Invoke* globalInvoke=nullptr);
	void update_withThreads(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE, Invoke* globalInvoke=nullptr);
	void reinsertAllObjects(int dispResX,int dispResY, int THREADSIZE);

	std::vector<std::vector<std::shared_ptr<RenderObject>>>& getContainerAt(int x, int y, int layer);
	bool isValidPosition(int x, int y, int layer);

	void purgeObjects();
	void purgeObjectsAt(int x, int y, int dispResX, int dispResY);
	void purgeLayer(int layer);

	size_t getObjectCount(bool excludeTopLayer = true);
	size_t getObjectCountAtTile(int x, int y,bool excludeTopLayer = true);

	enum RenderObjectLayers {
			background,
			general,
			foreground,
			effects,
			menue
	};
private:
	
	RenderObjectContainer roc[RENDEROBJECTCONTAINER_COUNT];
};

