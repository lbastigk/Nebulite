#include "Core/RenderObjectContainer.h"

//--------------------------------------------------------------------------------------
// RenderObjectContainer

//-----------------------------------------------------------
//Constructor

Nebulite::Core::RenderObjectContainer::RenderObjectContainer(Nebulite::Interaction::Invoke* globalInvoke) : globalInvoke(globalInvoke) {}

//-----------------------------------------------------------
//Marshalling
std::string Nebulite::Core::RenderObjectContainer::serialize() {
	//---------------------------------------
	// Setup

	// Initialize RapidJSON document
	Nebulite::Utility::JSON doc;

	//---------------------------------------
	// Get all objects in container
	int i = 0;
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ++it) {
		for (auto& batch : it->second) {
			for(auto& obj : batch.objects){
				Nebulite::Utility::JSON obj_serial;
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

void Nebulite::Core::RenderObjectContainer::deserialize(const std::string& serialOrLink, int dispResX, int dispResY) {
	Nebulite::Utility::JSON layer;
	layer.deserialize(serialOrLink);
	if(layer.memberCheck("objects") == Nebulite::Utility::JSON::KeyType::array){
		for(int i = 0; i < layer.memberSize("objects"); i++){
			std::string key = "objects[" + std::to_string(i) + "]";

			// Check if serial or not:
			std::string ro_serial = layer.get<std::string>(key.c_str());
			if(ro_serial == "{Object}"){
				Nebulite::Utility::JSON tmp;
				tmp = layer.get_subdoc(key.c_str());
				ro_serial = tmp.serialize();
			}

			RenderObject* ro = new RenderObject(globalInvoke->getGlobalPointer());
			ro->deserialize(ro_serial);
			append(ro, dispResX, dispResY);
		}
	}
}

//-----------------------------------------------------------
// Pipeline

std::pair<int16_t,int16_t> getTilePos(Nebulite::Core::RenderObject* toAppend, int dispResX, int dispResY){

    // Calculate correspondingTileXpos using positionX
    double posX = toAppend->valueGet<double>(Nebulite::Constants::keyName.renderObject.positionX.c_str(), 0.0);
    int16_t correspondingTileXpos = (int16_t)(posX / (double)dispResX);

    // Calculate correspondingTileYpos using positionY
    double posY = toAppend->valueGet<double>(Nebulite::Constants::keyName.renderObject.positionY.c_str(), 0.0);
    int16_t correspondingTileYpos = (int16_t)(posY / (double)dispResY);

    // Ensure the position is valid, grow the ObjectContainer if necessary
	return std::make_pair(correspondingTileXpos,correspondingTileYpos);
}

void Nebulite::Core::RenderObjectContainer::append(Nebulite::Core::RenderObject* toAppend, int dispResX, int dispResY) {
	uint64_t objectCost = toAppend->estimateComputationalCost(globalInvoke);
    std::pair<int16_t,int16_t> pos = getTilePos(toAppend, dispResX, dispResY);

	// Try to insert into an existing batch
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

void Nebulite::Core::RenderObjectContainer::update(int16_t tileXpos, int16_t tileYpos, int dispResX, int dispResY, Nebulite::Interaction::Invoke* globalInvoke) {
	//---------------------------------------------
	// 2-Step Deletion

	// Deleteflag--->Trash--->Purgatory-->Destructor
	// This way, any invokes previously send are safe to never access nullpointers

	// Finalize deletion of objects in purgatory
	if (!deletionProcess.purgatory.empty()) {
		for(auto& ptr : deletionProcess.purgatory){
			delete ptr;
		}
		deletionProcess.purgatory.clear();
	}

	// Move trash into purgatory
	deletionProcess.purgatory.swap(deletionProcess.trash);

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
				batchWorkers.emplace_back([&batch, pos, this, globalInvoke, dispResX, dispResY]() {
					// Every batch worker has potential objects to move or delete
					std::vector<RenderObject*> to_move_local;
					std::vector<RenderObject*> to_delete_local;

					// We update each object and check if it needs to be moved or deleted
					for (auto obj : batch.objects) {
						obj->update(globalInvoke);

						if (!obj->flag.deleteFromScene) {
							std::pair<uint16_t,uint16_t> newPos = getTilePos(obj, dispResX, dispResY);
							if (newPos != pos) {
								to_move_local.push_back(obj);
							}
						} else {
							to_delete_local.push_back(obj);
						}
					}

					// All objects to move are collected in objects_awaiting_reinsertion
					for (auto ptr : to_move_local) {
						batch.removeObject(ptr, globalInvoke);
						std::lock_guard<std::mutex> lock(reinsertionProcess.reinsertMutex);
						reinsertionProcess.objects_awaiting_reinsertion.push_back(ptr);
					}

					// All objects to delete are collected in trash
					for (auto ptr : to_delete_local) {
						batch.removeObject(ptr, globalInvoke);
						std::lock_guard<std::mutex> lock(deletionProcess.deleteMutex);
						deletionProcess.trash.push_back(ptr);
					}
				});
			}
		}
	}

	// Wait for threads to be finished
	for (auto& batchWorker : batchWorkers) {
		if(batchWorker.joinable()) batchWorker.join();
	}

	// Objects to move
	for (auto obj_ptr : reinsertionProcess.objects_awaiting_reinsertion) {
		append(obj_ptr, dispResX, dispResY);
	}
	reinsertionProcess.objects_awaiting_reinsertion.clear();
}

void Nebulite::Core::RenderObjectContainer::reinsertAllObjects(int dispResX, int dispResY) {
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

bool Nebulite::Core::RenderObjectContainer::isValidPosition(std::pair<uint16_t,uint16_t> pos) {
    // Check if ObjectContainer is not empty
	auto it = ObjectContainer.find(pos);
	return it != ObjectContainer.end();
}

std::vector<Nebulite::Core::RenderObjectContainer::batch>& Nebulite::Core::RenderObjectContainer::getContainerAt(std::pair<uint16_t,uint16_t> pos) {
	return ObjectContainer[pos];
}

void Nebulite::Core::RenderObjectContainer::purgeObjects() {
	// Release resources for ObjectContainer
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ) {
        ObjectContainer.erase(it++);
    }
}

size_t Nebulite::Core::RenderObjectContainer::getObjectCount() {
	// Calculate the total item count
	size_t totalCount = 0;

	int i = 0;
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ) {
		totalCount += it->second.size();
	}
	return totalCount;
}

//--------------------------------------------------------------------------------------
// Batch

Nebulite::Core::RenderObject* Nebulite::Core::RenderObjectContainer::batch::pop(Nebulite::Interaction::Invoke* globalInvoke) {
	if (objects.empty()) return nullptr;

	RenderObject* obj = objects.back(); // Get last element
	estimatedCost -= obj->estimateComputationalCost(globalInvoke); // Adjust cost
	objects.pop_back(); // Remove from vector

	return obj;
}

void Nebulite::Core::RenderObjectContainer::batch::push(RenderObject* obj, Nebulite::Interaction::Invoke* globalInvoke){
	estimatedCost += obj->estimateComputationalCost(globalInvoke);
	objects.push_back(obj);
}

bool Nebulite::Core::RenderObjectContainer::batch::removeObject(RenderObject* obj, Nebulite::Interaction::Invoke* globalInvoke) {
	auto it = std::find(objects.begin(), objects.end(), obj);
	if (it != objects.end()) {
		estimatedCost -= obj->estimateComputationalCost(globalInvoke);
		objects.erase(it);
		return true;
	}
	return false;
}