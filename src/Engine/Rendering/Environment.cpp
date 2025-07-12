#include "Environment.h"

Nebulite::Environment::Environment() {}

Nebulite::Environment::Environment(const Environment& other) {
	//doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
	//for (const auto& entry : other.roc)
}

//-----------------------------------------------------------
//Destructor
Nebulite::Environment::~Environment() {
	//TODO
};

//-----------------------------------------------------------
//Marshalling

std::string Nebulite::Environment::serialize() {

	Nebulite::JSON doc;

	// Serialize each container and add to the document
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		std::string key = "containerLayer" + std::to_string(i);
		std::string serializedContainer = roc[i].serialize();

		// Add the container JSON object to the main document
		Nebulite::JSON layer;
		layer.deserialize(serializedContainer);
		doc.set_subdoc(key.c_str(),layer);
	}
	return doc.serialize();
}

void Nebulite::Environment::deserialize(std::string serialOrLink, int dispResX,int dispResY) {
	Nebulite::JSON file;
	file.deserialize(serialOrLink);
	global->deserialize(file.get_subdoc("global").serialize());

	// Getting all layers
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		// Key name
		std::string key = "containerLayer" + std::to_string(i) ;

		// Check if the key exists in the document
		if (file.memberCheck(key) != Nebulite::JSON::KeyType::null) {
			// Extract the value corresponding to the key
			Nebulite::JSON layer = file.get_subdoc(key.c_str());

			// Convert the JSON object to a pretty-printed string
			std::string str = layer.serialize();

			// Serialize container layer
			roc[i].deserialize(str,dispResX,dispResY);
		}
		else {
			std::cerr << "Layer Key " << key << " not found in the document or uncomparible" << std::endl;
		}
	}
}

void Nebulite::Environment::append(Nebulite::RenderObject* toAppend,int dispResX, int dispResY, int layer) {
	if (layer < RENDEROBJECTCONTAINER_COUNT && layer >= 0) {
		roc[layer].append(toAppend, dispResX, dispResY);
	}
	else {
		roc[0].append(toAppend, dispResX, dispResY);
	}
}

// Before activating env update: might not work with direct invoke-manipulation of $(other.var)
//#define UPDATE_THREADED 1

#ifdef UPDATE_THREADED
void Nebulite::Environment::update(int16_t tileXpos, int16_t tileYpos, int dispResX, int dispResY, int THREADSIZE, Invoke* globalInvoke) {
    std::vector<std::thread> threads;

    for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; ++i) {
        threads.emplace_back([=]() {
            roc[i].update(tileXpos, tileYpos, dispResX, dispResY, THREADSIZE, globalInvoke);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}
#else
// no threads
void Nebulite::Environment::update(int16_t tileXpos, int16_t tileYpos,int dispResX,int dispResY,Nebulite::Invoke* globalInvoke) {
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		roc[i].update(tileXpos,tileYpos,dispResX,dispResY,globalInvoke);
	}
}
#endif




void Nebulite::Environment::reinsertAllObjects(int dispResX,int dispResY){
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		roc[i].reinsertAllObjects(dispResX,dispResY);
	}
}


std::vector<Nebulite::RenderObject*>& Nebulite::Environment::getContainerAt(int16_t x, int16_t y, int layer) {
	auto pos = std::make_pair(x,y);
	if (layer < RENDEROBJECTCONTAINER_COUNT && layer >= 0) {
		return roc[layer].getContainerAt(pos);
	}
	else {
		return roc[0].getContainerAt(pos);
	}
}

bool Nebulite::Environment::isValidPosition(int x, int y, int layer) {
	auto pos = std::make_pair(x,y);
	if (layer < RENDEROBJECTCONTAINER_COUNT && layer >= 0) {
		return roc[layer].isValidPosition(pos);
	}
	else {
		return roc[0].isValidPosition(pos);
	}
}

void Nebulite::Environment::purgeObjects() {
	// Release resources for ObjectContainer
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		roc[i].purgeObjects();
	}
}

void Nebulite::Environment::purgeObjectsAt(int x, int y, int dispResX, int dispResY){
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		roc[i].purgeObjectsAt(x, y, dispResX, dispResY);
	}
}

void Nebulite::Environment::purgeLayer(int layer) {
	if (layer >= 0 && layer < RENDEROBJECTCONTAINER_COUNT) {
		roc[layer].purgeObjects();
	}
}

size_t Nebulite::Environment::getObjectCount(bool excludeTopLayer) {
	// Calculate the total item count
	size_t totalCount = 0;

	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT - (int)excludeTopLayer; i++) {
		totalCount += roc[i].getObjectCount();
	}
	return totalCount;
}


