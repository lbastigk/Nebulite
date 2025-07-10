//--------------------------------------------------------------------------------------
// RenderObjectContainer
#include "RenderObjectContainer.h"

//-----------------------------------------------------------
//Constructor

Nebulite::RenderObjectContainer::RenderObjectContainer() {

}


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

			RenderObject ro;
			ro.deserialize(ro_serial);
			append(ro, dispResX, dispResY);
		}
	}
}

//-----------------------------------------------------------
//-----------------------------------------------------------
// Pipeline

std::pair<int16_t,int16_t> getTilePos(std::shared_ptr<Nebulite::RenderObject> toAppend, int dispResX, int dispResY){
    // Calculate tile position based on screen resolution
    double valget;

    // Calculate correspondingTileXpos using positionX
    valget = toAppend.get()->valueGet<double>(Nebulite::keyName.renderObject.positionX.c_str(), 0.0);
    int16_t correspondingTileXpos = (int16_t)(valget / (double)dispResX);

    // Calculate correspondingTileYpos using positionY
    valget = toAppend.get()->valueGet<double>(Nebulite::keyName.renderObject.positionY.c_str(), 0.0);
    int16_t correspondingTileYpos = (int16_t)(valget / (double)dispResY);

    // Ensure the position is valid, grow the ObjectContainer if necessary
	return std::make_pair(correspondingTileXpos,correspondingTileYpos);
}


void Nebulite::RenderObjectContainer::append(Nebulite::RenderObject& toAppend, int dispResX, int dispResY) {
	auto ptr = std::make_shared<Nebulite::RenderObject>(std::move(toAppend));

    std::pair<int16_t,int16_t> pos = getTilePos(ptr,dispResX,dispResY);
	ObjectContainer[pos].push_back(ptr);
}

void Nebulite::RenderObjectContainer::update(int16_t tileXpos, int16_t tileYpos, int dispResX, int dispResY, Nebulite::Invoke* globalInvoke, bool onlyRestructure) {
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
			pos = std::make_pair(dX, dY);
			auto& vec = ObjectContainer[pos];

			std::vector<std::shared_ptr<RenderObject>> newVec;
			newVec.reserve(vec.size());

			for (auto& obj : vec) {
				if (!onlyRestructure) {
					obj->update(globalInvoke, obj);
				}

				if (!obj->valueGet(Nebulite::keyName.renderObject.deleteFlag.c_str(), false)) {
					newPos = getTilePos(obj, dispResX, dispResY);
					if (newPos != pos) {
						toReinsert.push_back(obj);  // move to different tile
					} else {
						newVec.push_back(obj);      // stay in same tile
					}
				} else {
					// Object is marked for deletion — do NOT copy it to newVec.
					// If no one else holds a shared_ptr to it, it will be destroyed here.
					// Optional: log for debug
					// std::cout << "Deleted RenderObject: " << obj->getName() << std::endl;
				}
			}

			vec.swap(newVec); // Replace tile content with filtered list
		}
	}

	// Add objects back to renderer that are now in a different tile position
	for(const auto& obj : toReinsert){
		append(*obj.get(), dispResX, dispResY);
	}
	toReinsert.clear();
	toReinsert.shrink_to_fit();
}

void Nebulite::RenderObjectContainer::reinsertAllObjects(int dispResX, int dispResY) {
	std::vector<std::shared_ptr<RenderObject>> toReinsert;

	for (auto it = ObjectContainer.begin(); it != ObjectContainer.end(); ) {
		for (auto& obj : it->second) {
			toReinsert.push_back(obj);
		}
	}
	ObjectContainer.clear(); // Fully reset

	// Reinsert with consistent pointer ownership
	for (const auto& obj : toReinsert) {
		append(*obj.get(), dispResX, dispResY);
	}
}

bool Nebulite::RenderObjectContainer::isValidPosition(std::pair<uint16_t,uint16_t> pos) {
    // Check if ObjectContainer is not empty
	auto it = ObjectContainer.find(pos);
	return it != ObjectContainer.end();
}

std::vector<std::shared_ptr<Nebulite::RenderObject>>& Nebulite::RenderObjectContainer::getContainerAt(std::pair<uint16_t,uint16_t> pos) {
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
}
