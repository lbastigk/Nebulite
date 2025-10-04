#include "Core/Environment.hpp"
#include <utility>

#include "Core/GlobalSpace.hpp"


// Helper function to build the RenderObjectContainer array
namespace Nebulite {
    template<std::size_t... Is>
    std::array<Nebulite::Core::RenderObjectContainer, sizeof...(Is)> 
    make_roc_array(Nebulite::Core::GlobalSpace* globalSpace, std::index_sequence<Is...>) {
        return {{(static_cast<void>(Is), Nebulite::Core::RenderObjectContainer(globalSpace))...}};
    }
}   // namespace Nebulite

Nebulite::Core::Environment::Environment(Nebulite::Core::GlobalSpace* globalSpace)
	: roc(make_roc_array(globalSpace, std::make_index_sequence<Nebulite::Core::Environment::LayerCount>{}))
{
	// Storing pointer copy for easy access of global document
	global = globalSpace->getDoc();
}



//------------------------------------------
// Marshalling

std::string Nebulite::Core::Environment::serialize() {
	Nebulite::Utility::JSON doc;

	// Serialize each container and add to the document
	for (int i = 0; i < Nebulite::Core::Environment::LayerCount; i++) {
		std::string key = "containerLayer" + std::to_string(i);
		std::string serializedContainer = roc[i].serialize();

		// Add the container JSON object to the main document
		Nebulite::Utility::JSON layer;
		layer.deserialize(serializedContainer);
		doc.set_subdoc(key.c_str(), &layer);
	}
	return doc.serialize();
}

void Nebulite::Core::Environment::deserialize(std::string serialOrLink, int dispResX,int dispResY) {
	Nebulite::Utility::JSON file;
	file.deserialize(serialOrLink);
	global->deserialize(file.get_subdoc("global").serialize());

	// Getting all layers
	for (int i = 0; i < Nebulite::Core::Environment::LayerCount; i++) {
		// Key name
		std::string key = "containerLayer" + std::to_string(i) ;

		// Check if the key exists in the document
		if (file.memberCheck(key) != Nebulite::Utility::JSON::KeyType::null) {
			// Extract the value corresponding to the key
			Nebulite::Utility::JSON layer = file.get_subdoc(key.c_str());

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

//------------------------------------------
// Object Management

void Nebulite::Core::Environment::append(Nebulite::Core::RenderObject* toAppend,int dispResX, int dispResY, int layer) {
	if (layer < Nebulite::Core::Environment::LayerCount && layer >= 0) {
		roc[layer].append(toAppend, dispResX, dispResY);
	}
	else {
		roc[0].append(toAppend, dispResX, dispResY);
	}
}

void Nebulite::Core::Environment::update(int16_t tileXpos, int16_t tileYpos,int dispResX,int dispResY,Nebulite::Interaction::Invoke* globalInvoke) {
	for (int i = 0; i < Nebulite::Core::Environment::LayerCount; i++) {
		roc[i].update(tileXpos,tileYpos,dispResX,dispResY,globalInvoke);
	}
}

void Nebulite::Core::Environment::reinsertAllObjects(int dispResX,int dispResY){
	for (int i = 0; i < Nebulite::Core::Environment::LayerCount; i++) {
		roc[i].reinsertAllObjects(dispResX,dispResY);
	}
}

Nebulite::Core::RenderObject* Nebulite::Core::Environment::getObjectFromId(uint32_t id) {
	// Go through all layers
	for (int i = 0; i < Nebulite::Core::Environment::LayerCount; ++i) {
		auto obj = roc[i].getObjectFromId(id);
		if (obj != nullptr) {
			return obj;
		}
	}
	return nullptr;
}

//------------------------------------------
// Container Management

std::vector<Nebulite::Core::RenderObjectContainer::batch>& Nebulite::Core::Environment::getContainerAt(int16_t x, int16_t y, Environment::Layer layer) {
	auto pos = std::make_pair(x,y);
	if (layer < Nebulite::Core::Environment::LayerCount && layer >= 0) {
		return roc[layer].getContainerAt(pos);
	}
	else {
		return roc[0].getContainerAt(pos);
	}
}

bool Nebulite::Core::Environment::isValidPosition(int x, int y, Environment::Layer layer) {
	auto pos = std::make_pair(x,y);
	if (layer < Nebulite::Core::Environment::LayerCount && layer >= 0) {
		return roc[layer].isValidPosition(pos);
	}
	else {
		return roc[0].isValidPosition(pos);
	}
}

void Nebulite::Core::Environment::purgeObjects() {
	// Release resources for ObjectContainer
	for (int i = 0; i < Nebulite::Core::Environment::LayerCount; i++) {
		roc[i].purgeObjects();
	}
}

uint32_t Nebulite::Core::Environment::getObjectCount() {
	// Calculate the total item count
	uint32_t totalCount = 0;

	for (int i = 0; i < Nebulite::Core::Environment::LayerCount; i++) {
		totalCount += roc[i].getObjectCount();
	}
	return totalCount;
}

