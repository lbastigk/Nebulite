//--------------------------------------------------------------------------------------
// RenderObjectContainer
#include "RenderObjectContainer.h"

//-----------------------------------------------------------
//Constructor

Nebulite::RenderObjectContainer::RenderObjectContainer(Nebulite::Invoke* globalInvoke) : globalInvoke(globalInvoke) {}

//-----------------------------------------------------------
//Marshalling
std::string Nebulite::RenderObjectContainer::serialize() {
	//---------------------------------------
	// Setup

	// Initialize RapidJSON document
	Nebulite::JSON doc;

	//---------------------------------------
	// Get all objects in container
	int i = 0;
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ++it) {
		for (auto& batch : it->second) {
			for(auto& obj : batch.objects){
				Nebulite::JSON obj_serial;
				obj_serial.deserialize(obj->serialize());
					
				// insert into doc
				std::string key = "objects[" + std::to_string(i) + "]";
				doc.set_subdoc(key.c_str(),obj_serial);
				i++;
			}
		}
	}

	//---------------------------------------
	// Return as string
	return doc.serialize();
}

void Nebulite::RenderObjectContainer::deserialize(const std::string& serialOrLink, int dispResX, int dispResY) {
	Nebulite::JSON layer;
	layer.deserialize(serialOrLink);
	if(layer.memberCheck("objects") == Nebulite::JSON::KeyType::array){
		for(int i = 0; i < layer.memberSize("objects"); i++){
			std::string key = "objects[" + std::to_string(i) + "]";

			// Check if serial or not:
			std::string ro_serial = layer.get<std::string>(key.c_str());
			if(ro_serial == "{Object}"){
				Nebulite::JSON tmp;
				tmp = layer.get_subdoc(key.c_str());
				ro_serial = tmp.serialize();
			}

			RenderObject* ro = new RenderObject;
			ro->deserialize(ro_serial);
			append(ro, dispResX, dispResY);
		}
	}
}

//-----------------------------------------------------------
//-----------------------------------------------------------
// Pipeline

std::pair<int16_t,int16_t> getTilePos(Nebulite::RenderObject* toAppend, int dispResX, int dispResY){

    // Calculate correspondingTileXpos using positionX
    double posX = toAppend->valueGet<double>(Nebulite::keyName.renderObject.positionX.c_str(), 0.0);
    int16_t correspondingTileXpos = (int16_t)(posX / (double)dispResX);

    // Calculate correspondingTileYpos using positionY
    double posY = toAppend->valueGet<double>(Nebulite::keyName.renderObject.positionY.c_str(), 0.0);
    int16_t correspondingTileYpos = (int16_t)(posY / (double)dispResY);

    // Ensure the position is valid, grow the ObjectContainer if necessary
	return std::make_pair(correspondingTileXpos,correspondingTileYpos);
}

void Nebulite::RenderObjectContainer::append(Nebulite::RenderObject* toAppend, int dispResX, int dispResY) {
	uint64_t objectCost = toAppend->estimateComputationalCost(globalInvoke);
	//std::cout << "Appending object with estimated cost: " << objectCost << std::endl;

    std::pair<int16_t,int16_t> pos = getTilePos(toAppend, dispResX, dispResY);

	// Try to insert into an existing batch
	//std::cerr << "Current batch size in tile: " << ObjectContainer[pos].size() << std::endl;
	for (auto& batch : ObjectContainer[pos]) {
		if (batch.estimatedCost <= BATCH_COST_GOAL) {
			batch.push(toAppend, globalInvoke);
			return;
		}
	}

	// No existing batch could accept the object, so create a new one
	batch newBatch;
	newBatch.push(toAppend, globalInvoke);
	ObjectContainer[pos].push_back(std::move(newBatch));
}

void Nebulite::RenderObjectContainer::update(int16_t tileXpos, int16_t tileYpos, int dispResX, int dispResY, Nebulite::Invoke* globalInvoke, bool onlyRestructure) {
	

	//---------------------------------------------
	// 2-Step Deletion

	// Deleteflag--->Trash--->Purgatory-->Destructor
	// This way, any invokes previously send are safe to never access nullpointers

	// Finalize deletion of objects in purgatory
	if (!purgatory.empty()) {
		for(auto& ptr : purgatory){
			delete ptr;
		}
		purgatory.clear();
	}

	// Move trash into purgatory
	purgatory.swap(trash);

	//--------------------------------------------------------------------------------
	// Update only tiles that might be visible

	// since one tile is size of screen, a max of 9 tiles
	// [P] - Tile with Player
	// [#] - loaded Tiles
	// [ ] - inactive Tiles
	//
	// [ ][ ][ ][ ][ ][ ][ ][ ][ ]
	// [ ][ ][ ][ ][ ][ ][ ][ ][ ]
	// [ ][ ][ ][ ][ ][ ][ ][ ][ ]
	// [ ][ ][ ][#][#][#][ ][ ][ ]
	// [ ][ ][ ][#][P][#][ ][ ][ ]
	// [ ][ ][ ][#][#][#][ ][ ][ ]
	// [ ][ ][ ][ ][ ][ ][ ][ ][ ]
	// [ ][ ][ ][ ][ ][ ][ ][ ][ ]
	// [ ][ ][ ][ ][ ][ ][ ][ ][ ]
	for (int16_t dX = tileXpos - 1; dX <= tileXpos + 1; dX++) {
		for (int16_t dY = tileYpos - 1; dY <= tileYpos + 1; dY++) {
			std::pair<uint16_t,uint16_t> pos = std::make_pair(dX, dY);
			auto& tile = ObjectContainer[pos];

			for (auto& batch : tile) {
				workers.emplace_back([&batch, pos, this, globalInvoke, onlyRestructure, dispResX, dispResY]() {
					std::vector<RenderObject*> to_move_local;
					std::vector<RenderObject*> to_delete_local;

					for (auto obj : batch.objects) {
						if (!onlyRestructure) {
							obj->update(globalInvoke);
						}

						if (!obj->flag.deleteFromScene) {
							std::pair<uint16_t,uint16_t> newPos = getTilePos(obj, dispResX, dispResY);
							if (newPos != pos) {
								to_move_local.push_back(obj);
							}
						} else {
							to_delete_local.push_back(obj);
						}
					}

					// Lock to safely remove and collect
					for (auto ptr : to_move_local) {
						batch.removeObject(ptr, globalInvoke);
						std::lock_guard<std::mutex> lock(reinsertMutex);
						await_reinsert.push_back(ptr);
					}
					for (auto ptr : to_delete_local) {
						batch.removeObject(ptr, globalInvoke);
						std::lock_guard<std::mutex> lock(deleteMutex);
						trash.push_back(ptr);
					}
				});
			}
		}
	}

	// Wait for threads to be finished
	for (auto& worker : workers) {
		if(worker.joinable()) worker.join();
	}


	// Objects to move
	for (auto ptr : await_reinsert) {
		append(ptr, dispResX, dispResY);
	}
	await_reinsert.clear();
}

void Nebulite::RenderObjectContainer::reinsertAllObjects(int dispResX, int dispResY) {
	// Collect all objects
	std::vector<RenderObject*> toReinsert;
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); it++) {
		for (auto batch : it->second) {
			for(auto obj : batch.objects){
				toReinsert.push_back(obj);
			}
		}
	}

	// Fully reset container
	ObjectContainer.clear(); 

	// Reinsert
	for (const auto& ptr : toReinsert) {
		append(ptr,dispResX,dispResY);
	}
}

bool Nebulite::RenderObjectContainer::isValidPosition(std::pair<uint16_t,uint16_t> pos) {
    // Check if ObjectContainer is not empty
	auto it = ObjectContainer.find(pos);
	return it != ObjectContainer.end();
}

std::vector<Nebulite::RenderObjectContainer::batch>& Nebulite::RenderObjectContainer::getContainerAt(std::pair<uint16_t,uint16_t> pos) {
	return ObjectContainer[pos];
}

void Nebulite::RenderObjectContainer::purgeObjects() {
	// Release resources for ObjectContainer
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ) {
        ObjectContainer.erase(it++);
    }
}

void Nebulite::RenderObjectContainer::purgeObjectsAt(int x, int y, int dispResX, int dispResY){
	// Calculate correspondingTileXpos using positionX
    int16_t correspondingTileXpos = (int16_t)(x / (double)dispResX);
    int16_t correspondingTileYpos = (int16_t)(y / (double)dispResY);

    // Ensure the position is valid, grow the ObjectContainer if necessary
	std::pair<int16_t,int16_t> pos = std::make_pair(correspondingTileXpos,correspondingTileYpos);

	if (isValidPosition(pos)) {
		ObjectContainer[pos].clear();
	}	
}

size_t Nebulite::RenderObjectContainer::getObjectCount() {
	// Calculate the total item count
	size_t totalCount = 0;

	int i = 0;
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ) {
		totalCount += it->second.size();
	}
	return totalCount;
}
