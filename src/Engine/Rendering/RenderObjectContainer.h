#include "RenderObject.h"

class RenderObjectContainer {
public:
	//-----------------------------------------------------------
	//Constructor

	RenderObjectContainer();

	//-----------------------------------------------------------
	//Marshalling
	std::string serialize();
	void deserialize(const std::string& serialOrLink, int dispResX, int dispResY);

	//-----------------------------------------------------------
	//-----------------------------------------------------------
	// Pipeline

	// Append objects
	void append(std::shared_ptr<RenderObject> toAppend, int dispResX, int dispResY);

	// Reinsert all objects into container, useful for new tile size declaration
	// e.g. new rendering size
	void reinsertAllObjects(int dispResX, int dispResY);

	// Checks if given tile position contains objects
	bool isValidPosition(std::pair<uint16_t,uint16_t> pos);

	// removes all objects
	void purgeObjects();
	void purgeObjectsAt(int x, int y, int dispResX, int dispResY);	//at tile x,y

	// returns amount of objects
	size_t getObjectCount();

	// Updating all objects in 3x3 Tile viewport
	void update(int16_t tileXpos, int16_t tileYpos, int dispResX, int dispResY,Invoke* globalInvoke=nullptr,bool onlyRestructure = false);

	
	// Used to get a container Tile
	std::vector<std::shared_ptr<RenderObject>>& getContainerAt(std::pair<uint16_t,uint16_t> pos);

private:
	// TODO: Change to hashmap for better usage of negative x/y values!
	//tileY		tileX		#			Batches		of RenderObject pointer
	absl::flat_hash_map<std::pair<int16_t,int16_t>,std::vector<std::shared_ptr<RenderObject>>> ObjectContainer;
	//std::vector<std::vector<std::vector<std::vector<std::shared_ptr<RenderObject>>>>> ObjectContainer;

};
