//--------------------------------------------------------------------------------------
// RenderObjectContainer
#include "RenderObjectContainer.h"

//-----------------------------------------------------------
//Constructor

RenderObjectContainer::RenderObjectContainer() {

}


//-----------------------------------------------------------
//Marshalling
std::string RenderObjectContainer::serialize() {
	//---------------------------------------
	// Setup

	// Initialize RapidJSON document
	Nebulite::JSON doc;

	//---------------------------------------
	// Get all objects in container
	int i = 0;
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ) {
		for (auto& obj : it->second) {
			Nebulite::JSON obj_serial;
			obj_serial.deserialize(obj->serialize());
				
			// insert into doc
			std::string key = "objects[" + std::to_string(i) + "]";
			doc.set_subdoc(key.c_str(),obj_serial);
			i++;
		}
	}

	//---------------------------------------
	// Return as string
	return doc.serialize();
}

void RenderObjectContainer::deserialize(const std::string& serialOrLink, int dispResX, int dispResY) {
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

			RenderObject ro;
			ro.deserialize(ro_serial);

			auto ptr = std::make_shared<RenderObject>(std::move(ro));
			append(ptr, dispResX, dispResY);
		}
	}
}

//-----------------------------------------------------------
//-----------------------------------------------------------
// Pipeline

std::pair<int16_t,int16_t> getTilePos(std::shared_ptr<RenderObject> toAppend, int dispResX, int dispResY){
    // Calculate tile position based on screen resolution
    double valget;

    // Calculate correspondingTileXpos using positionX
    valget = toAppend.get()->valueGet<double>(namenKonvention.renderObject.positionX.c_str(), 0.0);
    int16_t correspondingTileXpos = (int16_t)(valget / (double)dispResX);

    // Calculate correspondingTileYpos using positionY
    valget = toAppend.get()->valueGet<double>(namenKonvention.renderObject.positionY.c_str(), 0.0);
    int16_t correspondingTileYpos = (int16_t)(valget / (double)dispResY);

    // Ensure the position is valid, grow the ObjectContainer if necessary
	return std::make_pair(correspondingTileXpos,correspondingTileYpos);
}


void RenderObjectContainer::append(std::shared_ptr<RenderObject> toAppend, int dispResX, int dispResY) {
    std::pair<int16_t,int16_t> pos = getTilePos(toAppend,dispResX,dispResY);
	ObjectContainer[pos].push_back(toAppend);
}

void RenderObjectContainer::update(int16_t tileXpos, int16_t tileYpos, int dispResX, int dispResY, Invoke* globalInvoke, bool onlyRestructure) {
	std::pair<uint16_t,uint16_t> pos;
	std::pair<uint16_t,uint16_t> newPos;
	std::vector<std::shared_ptr<RenderObject>> toReinsert;

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
			pos = std::make_pair(dX,dY);
			std::vector<std::shared_ptr<RenderObject>> newBatch;
			for (auto& obj : ObjectContainer[pos]) {
				if(!onlyRestructure){
					obj->update(globalInvoke,obj);
				}
				
				//-----------------------------------------
				// Check delete flag
				if (!obj->valueGet(namenKonvention.renderObject.deleteFlag.c_str(),false)){
					// Check if it's in a new tile
					newPos = getTilePos(obj,dispResX,dispResY);
					if (newPos != pos) {
						toReinsert.push_back(obj);
					}
					else{
						newBatch.push_back(obj);
					}
				}
				else{
					//dont reinsert: gets deleted
				}
			}
			// Give new batch of objects still in same tile
			//batch = std::move(newBatch);
			ObjectContainer[pos].swap(newBatch);

			newBatch.clear();
			newBatch.shrink_to_fit();
		}
	}

	// Add objects back to renderer that are now in a different tile position
	for(const auto& obj : toReinsert){
		append(obj, dispResX, dispResY);
	}
	toReinsert.clear();
	toReinsert.shrink_to_fit();
}


void RenderObjectContainer::reinsertAllObjects(int dispResX, int dispResY) {
	std::vector<std::shared_ptr<RenderObject>> toReinsert;

	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ) {
		for (auto& obj : it->second) {
			toReinsert.push_back(obj);
		}
	}
	ObjectContainer.clear(); // Fully reset

	// Reinsert with consistent pointer ownership
	for (const auto& obj : toReinsert) {
		append(obj, dispResX, dispResY);
	}
}

bool RenderObjectContainer::isValidPosition(std::pair<uint16_t,uint16_t> pos) {
    // Check if ObjectContainer is not empty
	auto it = ObjectContainer.find(pos);
	return it != ObjectContainer.end();
}

std::vector<std::shared_ptr<RenderObject>>& RenderObjectContainer::getContainerAt(std::pair<uint16_t,uint16_t> pos) {
	return ObjectContainer[pos];
}

void RenderObjectContainer::purgeObjects() {
	// Release resources for ObjectContainer
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ) {
        ObjectContainer.erase(it++);
    }
}

void RenderObjectContainer::purgeObjectsAt(int x, int y, int dispResX, int dispResY){
	// Calculate correspondingTileXpos using positionX
    int16_t correspondingTileXpos = (int16_t)(x / (double)dispResX);
    int16_t correspondingTileYpos = (int16_t)(y / (double)dispResY);

    // Ensure the position is valid, grow the ObjectContainer if necessary
	std::pair<int16_t,int16_t> pos = std::make_pair(correspondingTileXpos,correspondingTileYpos);

	if (isValidPosition(pos)) {
		ObjectContainer[pos].clear();
	}	
}

size_t RenderObjectContainer::getObjectCount() {
	// Calculate the total item count
	size_t totalCount = 0;

	int i = 0;
	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ) {
		totalCount += it->second.size();
	}
}
