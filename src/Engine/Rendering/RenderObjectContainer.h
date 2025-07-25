#include "RenderObject.h"
#include <thread>

namespace Nebulite{
class RenderObjectContainer {
public:
	//-----------------------------------------------------------
	//Constructor

	RenderObjectContainer();

	//-----------------------------------------------------------
	// Batch of objects in container
	struct batch{
		std::vector<RenderObject*> objects;
		uint64_t estimatedCost = 0;

		RenderObject* pop() {
			if (objects.empty()) return nullptr;

			RenderObject* obj = objects.back(); // Get last element
			estimatedCost -= obj->estimateComputationalCost(); // Adjust cost
			objects.pop_back(); // Remove from vector

			return obj;
		}

		void push(RenderObject* obj){
			estimatedCost += obj->estimateComputationalCost();
			objects.push_back(obj);
		}

		bool removeObject(RenderObject* obj) {
			auto it = std::find(objects.begin(), objects.end(), obj);
			if (it != objects.end()) {
				estimatedCost -= obj->estimateComputationalCost();
				objects.erase(it);
				return true;
			}
			return false;
		}
	};

	//-----------------------------------------------------------
	//Marshalling
	std::string serialize();
	void deserialize(const std::string& serialOrLink, int dispResX, int dispResY);

	//-----------------------------------------------------------
	//-----------------------------------------------------------
	// Pipeline

	// Append objects
	void append(Nebulite::RenderObject* toAppend, int dispResX, int dispResY);

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
	void update(int16_t tileXpos, int16_t tileYpos, int dispResX, int dispResY,Nebulite::Invoke* globalInvoke=nullptr,bool onlyRestructure = false);

	
	// Used to get a container Tile
	std::vector<batch>& getContainerAt(std::pair<uint16_t,uint16_t> pos);

private:
	const uint64_t BATCH_COST_GOAL = 5000;

	

	absl::flat_hash_map<std::pair<int16_t,int16_t>,std::vector<batch>> ObjectContainer;
	std::vector<std::thread> workers;
	std::mutex reinsertMutex;
	std::mutex deleteMutex;

	// 3-Step reinsertion
	// 1.) remove from batch
	// 2.) reinsert
	std::vector<Nebulite::RenderObject*> to_move;
	std::vector<Nebulite::RenderObject*> await_reinsert;

	// 4-Step deletion
	// 1.) remove from batch
	// 2.) store in trash
	// 3.) store in purgatory
	// 4.) delete
	std::vector<Nebulite::RenderObject*> to_delete;
	std::vector<Nebulite::RenderObject*> trash;		// Moving objects, marking for deletion
	std::vector<Nebulite::RenderObject*> purgatory;	// Deleted each frame
};
}
