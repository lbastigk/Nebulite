#include "Core/Environment.hpp"
#include "Core/GlobalSpace.hpp"

/**
 * @brief Helper functions to create an array of RenderObjectContainer instances.
 */
namespace {
	/**
	 * @brief Creates an array of RenderObjectContainer instances.
	 * 
	 * @tparam LayerCount The number of layers (size of the array).
	 * @param globalSpace Pointer to the GlobalSpace instance.
	 * @return An array of RenderObjectContainer instances.
	 */
	template<std::size_t LayerCount>
	std::array<Nebulite::Core::RenderObjectContainer, LayerCount>
	make_roc_array(Nebulite::Core::GlobalSpace* globalSpace){
		return []<std::size_t... Is>(Nebulite::Core::GlobalSpace* gs, std::index_sequence<Is...>){
			return std::array<Nebulite::Core::RenderObjectContainer, sizeof...(Is)>{
				{(static_cast<void>(Is), Nebulite::Core::RenderObjectContainer(gs))...}
			};
		}(globalSpace, std::make_index_sequence<LayerCount>{});
	}
}

Nebulite::Core::Environment::Environment(Nebulite::Core::GlobalSpace* globalSpace)
	: roc(make_roc_array<Nebulite::Core::Environment::LayerCount>(globalSpace))
{
	this->globalSpace = globalSpace;

	// Storing pointer copy for easy access of global document
	global = globalSpace->getDoc();
}

//------------------------------------------
// Marshalling

std::string Nebulite::Core::Environment::serialize(){
	Nebulite::Utility::JSON doc(globalSpace);

	// Serialize each container and add to the document
	for (unsigned int i = 0; i < Nebulite::Core::Environment::LayerCount; i++){
		std::string key = "containerLayer" + std::to_string(i);
		std::string serializedContainer = roc[i].serialize();

		// Add the container JSON object to the main document
		Nebulite::Utility::JSON layer(globalSpace);
		layer.deserialize(serializedContainer);
		doc.set_subdoc(key.c_str(), &layer);
	}
	return doc.serialize();
}

void Nebulite::Core::Environment::deserialize(std::string const& serialOrLink, uint16_t dispResX,uint16_t dispResY){
	Nebulite::Utility::JSON file(globalSpace);
	file.deserialize(serialOrLink);

	// Getting all layers
	for (unsigned int i = 0; i < Nebulite::Core::Environment::LayerCount; i++){
		// Key name
		std::string key = "containerLayer" + std::to_string(i) ;

		// Check if the key exists in the document
		if (file.memberCheck(key) != Nebulite::Utility::JSON::KeyType::null){
			// Extract the value corresponding to the key
			Nebulite::Utility::JSON layer = file.get_subdoc(key);

			// Convert the JSON object to a pretty-printed string
			std::string str = layer.serialize();

			// Serialize container layer
			roc[i].deserialize(str,dispResX,dispResY);
		}
	}
}

//------------------------------------------
// Object Management

void Nebulite::Core::Environment::append(Nebulite::Core::RenderObject* toAppend,uint16_t dispResX, uint16_t dispResY, uint8_t layer){
	if (layer < Nebulite::Core::Environment::LayerCount){
		roc[layer].append(toAppend, dispResX, dispResY);
	}
	else {
		roc[0].append(toAppend, dispResX, dispResY);
	}
}

void Nebulite::Core::Environment::update(int16_t tileXposition, int16_t tileYposition, uint16_t dispResX, uint16_t dispResY){
	for (unsigned int i = 0; i < Nebulite::Core::Environment::LayerCount; i++){
		roc[i].update(tileXposition, tileYposition, dispResX, dispResY);
	}
}

void Nebulite::Core::Environment::reinsertAllObjects(uint16_t dispResX,uint16_t dispResY){
	for (unsigned int i = 0; i < Nebulite::Core::Environment::LayerCount; i++){
		roc[i].reinsertAllObjects(dispResX,dispResY);
	}
}

Nebulite::Core::RenderObject* Nebulite::Core::Environment::getObjectFromId(uint32_t id){
	// Go through all layers
	for (unsigned int i = 0; i < Nebulite::Core::Environment::LayerCount; ++i){
		auto obj = roc[i].getObjectFromId(id);
		if (obj != nullptr){
			return obj;
		}
	}
	return nullptr;
}

//------------------------------------------
// Container Management

std::vector<Nebulite::Core::RenderObjectContainer::batch>& Nebulite::Core::Environment::getContainerAt(int16_t x, int16_t y, Environment::Layer layer){
	auto pos = std::make_pair(x,y);
	if (layer < Nebulite::Core::Environment::LayerCount && layer >= 0){
		return roc[layer].getContainerAt(pos);
	}
	else {
		return roc[0].getContainerAt(pos);
	}
}

bool Nebulite::Core::Environment::isValidPosition(int x, int y, Environment::Layer layer){
	auto pos = std::make_pair(x,y);
	if (layer < Nebulite::Core::Environment::LayerCount && layer >= 0){
		return roc[layer].isValidPosition(pos);
	}
	else {
		return roc[0].isValidPosition(pos);
	}
}

void Nebulite::Core::Environment::purgeObjects(){
	// Release resources for ObjectContainer
	for (unsigned int i = 0; i < Nebulite::Core::Environment::LayerCount; i++){
		roc[i].purgeObjects();
	}
}

size_t Nebulite::Core::Environment::getObjectCount() const {
	return std::accumulate(
		roc.begin(), roc.end(), 0u,
		[](size_t acc, Nebulite::Core::RenderObjectContainer const& container){
			return acc + container.getObjectCount();
		}
	);
}

