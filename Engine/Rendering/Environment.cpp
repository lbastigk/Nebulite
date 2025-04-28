#include "Environment.h"

Environment::Environment() {
	JSONHandler::Set::Any<double>(global,"dt",0);
    JSONHandler::Set::Any<double>(global,"G",0.1 * 100);
}

Environment::Environment(const Environment& other) {
	//doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
	//for (const auto& entry : other.roc)
}

//-----------------------------------------------------------
//Destructor
Environment::~Environment() {
	//TODO
};

//-----------------------------------------------------------
//Marshalling

std::string Environment::serialize() {
	// Initialize RapidJSON document
	rapidjson::Document doc;
	doc.SetObject();

	JSONHandler::Set::subDoc(doc,"global",global);

	// Set up allocator
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	// Serialize each container and add to the document
	std::string serializedContainers[RENDEROBJECTCONTAINER_COUNT];
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		serializedContainers[i] = roc[i].serialize();

		// Parse the serialized string to a JSON object
		rapidjson::Document containerDoc;
		containerDoc.Parse(serializedContainers[i].c_str());

		if (containerDoc.HasParseError()) {
			std::cerr << "JSON parse error: " << containerDoc.GetParseError() << std::endl;
			continue; // Skip this container if there's an error
		}

		// Create a Value to hold the containerDoc contents
		rapidjson::Value containerValue;
		containerValue.CopyFrom(containerDoc, allocator);

		// Add the container JSON object to the main document
		std::string key = "containerLayer" + std::to_string(i);
		doc.AddMember(rapidjson::Value(key.c_str(), allocator).Move(), containerValue, allocator);
	}

	// Convert the document to a string
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	return buffer.GetString();
}

void Environment::deserialize(std::string serialOrLink, int dispResX,int dispResY,int THREADSIZE) {
	rapidjson::Document doc;
	doc = JSONHandler::deserialize(serialOrLink);

	JSONHandler::Get::subDoc(doc,"global",global);

	// doc has values for containerLayer0 to containerLayer4
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		std::string key = "containerLayer" + std::to_string(i);

		// Check if the key exists in the document
		if (doc.HasMember(key.c_str())) {
			// Extract the value corresponding to the key
			const rapidjson::Value& layer = doc[key.c_str()];

			// Convert the JSON object to a pretty-printed string
			rapidjson::StringBuffer buffer;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
			layer.Accept(writer);

			
			std::string str = buffer.GetString();

			roc[i].deserialize(str,dispResX,dispResY,THREADSIZE);
		}
		else {
			std::cerr << "Key " << key << " not found in the document!" << std::endl;
		}
	}
}

void Environment::append(RenderObject toAttach,int dispResX, int dispResY,int THREADSIZE, int layer) {
	if (layer < RENDEROBJECTCONTAINER_COUNT && layer >= 0) {
		roc[layer].append(toAttach, dispResX, dispResY, THREADSIZE);
	}
	else {
		roc[0].append(toAttach, dispResX, dispResY, THREADSIZE);
	}
}

/*
// Old version, no threads
void Environment::update(int tileXpos,int tileYpos,int dispResX,int dispResY, int THREADSIZE,Invoke* globalInvoke) {
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		roc[i].update(tileXpos,tileYpos,dispResX,dispResY,THREADSIZE,globalInvoke);
	}
}
*/

void Environment::update(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE, Invoke* globalInvoke) {
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



void Environment::reinsertAllObjects(int dispResX,int dispResY, int THREADSIZE){
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		roc[i].reinsertAllObjects(dispResX,dispResY,THREADSIZE);
	}
}

void Environment::update_withThreads(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE,Invoke* globalInvoke) {
	// Update all objects in each layer

	// TODO:
	// IDEA: A runner for each roc, telling the env/renderer once done and being triggered by env/renderer 
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		roc[i].update_withThreads(tileXpos, tileYpos, dispResX, dispResY, THREADSIZE);
	}
}

std::vector<std::vector<std::shared_ptr<RenderObject>>>& Environment::getContainerAt(int x, int y, int layer) {
	if (layer < RENDEROBJECTCONTAINER_COUNT && layer >= 0) {
		return roc[layer].getContainerAt(x,y);
	}
	else {
		return roc[0].getContainerAt(x, y);
	}
}

bool Environment::isValidPosition(int x, int y, int layer) {
	if (layer < RENDEROBJECTCONTAINER_COUNT && layer >= 0) {
		return roc[layer].isValidPosition(x, y);
	}
	else {
		return roc[0].isValidPosition(x, y);
	}
}

void Environment::purgeObjects() {
	// Release resources for ObjectContainer
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		roc[i].purgeObjects();
	}
}

void Environment::purgeObjectsAt(int x, int y, int dispResX, int dispResY){
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		roc[i].purgeObjectsAt(x, y, dispResX, dispResY);
	}
}

void Environment::purgeLayer(int layer) {
	if (layer >= 0 && layer < RENDEROBJECTCONTAINER_COUNT) {
		roc[layer].purgeObjects();
	}
}

size_t Environment::getObjectCount(bool excludeTopLayer) {
	// Calculate the total item count
	size_t totalCount = 0;

	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT - (int)excludeTopLayer; i++) {
		totalCount += roc[i].getObjectCount();
	}
	return totalCount;
}

size_t Environment::getObjectCountAtTile(int x, int y,bool excludeTopLayer) {
	// Calculate the total item count
	size_t totalCount = 0;

	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT - (int)excludeTopLayer; i++) {
		totalCount += roc[i].getObjectCountAtTile(x,y);
	}
	return totalCount;
}

