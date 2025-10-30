#include "Core/RenderObjectContainer.hpp"

#include "Core/GlobalSpace.hpp"

//------------------------------------------
// RenderObjectContainer

//------------------------------------------
//Constructor

Nebulite::Core::RenderObjectContainer::RenderObjectContainer(Nebulite::Core::GlobalSpace* globalSpace){
	this->globalSpace = globalSpace;
}

//------------------------------------------
//Marshalling
std::string Nebulite::Core::RenderObjectContainer::serialize(){
	//------------------------------------------
	// Setup

	// Initialize RapidJSON document
	Nebulite::Utility::JSON doc(globalSpace);

	//------------------------------------------
	// Get all objects in container
	int i = 0;
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ++it){
		for (auto& batch : it->second){
			for(auto& obj : batch.objects){
				Nebulite::Utility::JSON obj_serial(globalSpace);
				obj_serial.deserialize(obj->serialize());
					
				// insert into doc
				std::string key = "objects[" + std::to_string(i) + "]";
				doc.set_subdoc(key.c_str(), &obj_serial);
				i++;
			}
		}
	}

	//------------------------------------------
	// Return as string
	return doc.serialize();
}

void Nebulite::Core::RenderObjectContainer::deserialize(std::string const& serialOrLink, uint16_t dispResX, uint16_t dispResY){
	Nebulite::Utility::JSON layer(globalSpace);
	layer.deserialize(serialOrLink);
	if(layer.memberCheck("objects") == Nebulite::Utility::JSON::KeyType::array){
		for(uint32_t i = 0; i < layer.memberSize("objects"); i++){
			std::string key = "objects[" + std::to_string(i) + "]";

			// Check if serial or not:
			std::string ro_serial = layer.get<std::string>(key.c_str());
			if(ro_serial == "{Object}"){
				Nebulite::Utility::JSON tmp(globalSpace);
				tmp = layer.get_subdoc(key);
				ro_serial = tmp.serialize();
			}

			RenderObject* ro = new RenderObject(globalSpace);
			ro->deserialize(ro_serial);
			append(ro, dispResX, dispResY);
		}
	}
}

//------------------------------------------
// Pipeline

std::pair<int16_t,int16_t> getTilePos(Nebulite::Core::RenderObject* toAppend, uint16_t dispResX, uint16_t dispResY){
    // Calculate correspondingTileXpos using positionX
    double posX = toAppend->get<double>(Nebulite::Constants::keyName.renderObject.positionX.c_str(), 0.0);
    int16_t correspondingTileXpos = static_cast<int16_t>(posX / static_cast<double>(dispResX));

    // Calculate correspondingTileYpos using positionY
    double posY = toAppend->get<double>(Nebulite::Constants::keyName.renderObject.positionY.c_str(), 0.0);
    int16_t correspondingTileYpos = static_cast<int16_t>(posY / static_cast<double>(dispResY));

    // Form pair and return
	return std::make_pair(correspondingTileXpos,correspondingTileYpos);
}

void Nebulite::Core::RenderObjectContainer::append(Nebulite::Core::RenderObject* toAppend, uint16_t dispResX, uint16_t dispResY){
    std::pair<int16_t,int16_t> pos = getTilePos(toAppend, dispResX, dispResY);

	// Try to insert into an existing batch
	auto it = std::find_if(ObjectContainer[pos].begin(), ObjectContainer[pos].end(),
		[](const batch& b){ return b.estimatedCost <= BATCH_COST_GOAL; }
	);
	if (it != ObjectContainer[pos].end()){
		it->push(toAppend);
		return;
	}

	// No existing batch could accept the object, so create a new one
	batch newBatch;
	newBatch.push(toAppend);
	ObjectContainer[pos].push_back(std::move(newBatch));
}

std::thread Nebulite::Core::RenderObjectContainer::create_batch_worker(batch& batch, std::pair<uint16_t, uint16_t> pos,uint16_t dispResX, uint16_t dispResY){
	return std::thread([&batch, pos, this, dispResX, dispResY](){
		// Every batch worker has potential objects to move or delete
		std::vector<RenderObject*> to_move_local;
		std::vector<RenderObject*> to_delete_local;

		// We update each object and check if it needs to be moved or deleted
		for (auto obj : batch.objects){
			obj->update();

			if (!obj->flag.deleteFromScene){
				std::pair<uint16_t,uint16_t> newPos = getTilePos(obj, dispResX, dispResY);
				if (newPos != pos){
					to_move_local.push_back(obj);
				}
			} else {
				to_delete_local.push_back(obj);
			}
		}

		// All objects to move are collected in queue
		for (auto ptr : to_move_local){
			batch.removeObject(ptr);
			std::lock_guard<std::mutex> lock(reinsertionProcess.reinsertMutex);
			reinsertionProcess.queue.push_back(ptr);
		}

		// All objects to delete are collected in trash
		for (auto ptr : to_delete_local){
			batch.removeObject(ptr);
			std::lock_guard<std::mutex> lock(deletionProcess.deleteMutex);
			deletionProcess.trash.push_back(ptr);
		}
	});
}

void Nebulite::Core::RenderObjectContainer::update(int16_t tileXpos, int16_t tileYpos, uint16_t dispResX, uint16_t dispResY){
	//------------------------------------------
	// 2-Step Deletion

	// Deleteflag--->Trash--->Purgatory-->Destructor
	// This way, any invokes previously send are safe to never access nullpointers

	// Finalize deletion of objects in purgatory
	if (!deletionProcess.purgatory.empty()){
		for(auto& ptr : deletionProcess.purgatory){
			delete ptr;
		}
		deletionProcess.purgatory.clear();
	}

	// Move trash into purgatory
	deletionProcess.purgatory.swap(deletionProcess.trash);

	//------------------------------------------
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
	for (int16_t dX = tileXpos - 1; dX <= tileXpos + 1; dX++){
		for (int16_t dY = tileYpos - 1; dY <= tileYpos + 1; dY++){
			std::pair<uint16_t,uint16_t> pos = std::make_pair(dX, dY);
			auto& tile = ObjectContainer[pos];

			// Create batch workers for each batch in the tile
			std::transform(
				tile.begin(), tile.end(),
				std::back_inserter(batchWorkers),
				[&](auto& batch){ return create_batch_worker(batch, pos, dispResX, dispResY); }
			);
		}
	}

	// Wait for threads to be finished
	for (auto& batchWorker : batchWorkers){
		if(batchWorker.joinable()) batchWorker.join();
	}

	// Objects to move
	for (auto obj_ptr : reinsertionProcess.queue){
		append(obj_ptr, dispResX, dispResY);
	}
	reinsertionProcess.queue.clear();
}

void Nebulite::Core::RenderObjectContainer::reinsertAllObjects(uint16_t dispResX, uint16_t dispResY){
	// Collect all objects
	std::vector<RenderObject*> toReinsert;
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); it++){
		for (auto batch : it->second){
			// Collect all objects from the batch
			std::copy(batch.objects.begin(), batch.objects.end(), std::back_inserter(toReinsert));
		}
	}

	// Fully reset container
	ObjectContainer.clear(); 

	// Reinsert
	for (auto const& ptr : toReinsert){
		append(ptr,dispResX,dispResY);
	}
}

bool Nebulite::Core::RenderObjectContainer::isValidPosition(std::pair<uint16_t,uint16_t> pos){
    // Check if ObjectContainer is not empty
	auto it = ObjectContainer.find(pos);
	return it != ObjectContainer.end();
}

std::vector<Nebulite::Core::RenderObjectContainer::batch>& Nebulite::Core::RenderObjectContainer::getContainerAt(std::pair<uint16_t,uint16_t> pos){
	return ObjectContainer[pos];
}

void Nebulite::Core::RenderObjectContainer::purgeObjects(){
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ){
		for (auto& batch : it->second){
			// Move all objects to trash
			std::move(batch.objects.begin(), batch.objects.end(), std::back_inserter(deletionProcess.trash));
			batch.objects.clear(); // Remove all objects from the batch
		}
		++it;
	}
}

size_t Nebulite::Core::RenderObjectContainer::getObjectCount() const {
	// Calculate the total item count
	size_t totalCount = 0;
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ){
		totalCount += it->second.size();
	}
	return totalCount;
}

//------------------------------------------
// Batch

Nebulite::Core::RenderObject* Nebulite::Core::RenderObjectContainer::batch::pop(){
	if (objects.empty()) return nullptr;

	RenderObject* obj = objects.back(); // Get last element
	estimatedCost -= obj->estimateComputationalCost(); // Adjust cost
	objects.pop_back(); // Remove from vector

	return obj;
}

void Nebulite::Core::RenderObjectContainer::batch::push(RenderObject* obj){
	estimatedCost += obj->estimateComputationalCost();
	objects.push_back(obj);
}

bool Nebulite::Core::RenderObjectContainer::batch::removeObject(RenderObject* obj){
	auto it = std::find(objects.begin(), objects.end(), obj);
	if (it != objects.end()){
		estimatedCost -= obj->estimateComputationalCost();
		objects.erase(it);
		return true;
	}
	return false;
}