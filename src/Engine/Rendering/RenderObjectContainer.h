#include "RenderObject.h"

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
	void append(std::shared_ptr<RenderObject> toAppend, int dispResX, int dispResY, int THREADSIZE);

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
	void update_withThreads(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE,Invoke* globalInvoke=nullptr);
	void update(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE,Invoke* globalInvoke=nullptr,bool onlyRestructure = false);

	
	// Used to get a container Tile
	std::vector<std::vector<std::shared_ptr<RenderObject>>>& getContainerAt(int x, int y);
	
	// Returns a texture stored inside the container
	SDL_Texture* getTexture(int screenSizeX, int screenSizeY, SDL_Renderer *renderer, int tileXpos, int tileYpos, int Xpos, int Ypos, auto& TextureContainer);

private:
	// TODO: Change to hashmap for better usage of negative x/y values!
	//tileY		tileX		#			Batches		of RenderObject pointer
	//absl::flat_hash_map<int16_t,absl::flat_hash_map<int16_t,std::vector<std::shared_ptr<RenderObject>>>> ObjectContainer;
	std::vector<std::vector<std::vector<std::vector<std::shared_ptr<RenderObject>>>>> ObjectContainer;

};
