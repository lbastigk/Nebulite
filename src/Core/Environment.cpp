//------------------------------------------
// Includes

// Standard library
#include <string>
#include <vector>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite.hpp"
#include "Core/Environment.hpp"
#include "Core/RenderObjectContainer.hpp"
#include "Data/Document/JSON.hpp"

//------------------------------------------

/**
 * @brief Helper functions to create an array of RenderObjectContainer instances.
 */
namespace {
/**
 * @brief Creates an array of RenderObjectContainer instances.
 *
 * @tparam LayerCount The number of layers (size of the array).
 * @return An array of RenderObjectContainer instances.
 */
template <std::size_t LayerCount>
std::array<Nebulite::Core::RenderObjectContainer, LayerCount>
make_roc_array() {
    return []<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::array<Nebulite::Core::RenderObjectContainer, sizeof...(Is)>{
            {(static_cast<void>(Is), Nebulite::Core::RenderObjectContainer{})...}
        };
    }(std::make_index_sequence<LayerCount>{});
}
} // anonymous namespace

namespace Nebulite::Core {

Environment::Environment(Data::JsonScope& documentReference)
    : Domain("Environment", *this, documentReference),
      roc(make_roc_array<LayerCount>()) {
}

Environment::~Environment() = default;

//------------------------------------------
// Marshalling

std::string Environment::serialize() {
    Data::JSON doc;

    // Serialize each container and add to the document
    for (unsigned int i = 0; i < LayerCount; i++) {
        std::string key = "containerLayer" + std::to_string(i);
        std::string serializedContainer = roc[i].serialize();

        // Add the container JSON object to the main document
        Data::JSON layer;
        layer.deserialize(serializedContainer);
        doc.setSubDoc(key.c_str(), layer);
    }
    return doc.serialize();
}

void Environment::deserialize(std::string const& serialOrLink, uint16_t const& dispResX, uint16_t const& dispResY) {
    Data::JSON file;
    file.deserialize(serialOrLink);

    // Getting all layers
    for (unsigned int i = 0; i < LayerCount; i++) {
        // Check if the key exists in the document
        if (std::string key = "containerLayer" + std::to_string(i); file.memberType(key) != Data::KeyType::null) {
            // Extract the value corresponding to the key
            Data::JSON layer = file.getSubDoc(key);

            // Convert the JSON object to a pretty-printed string
            std::string str = layer.serialize();

            // Serialize container layer
            roc[i].deserialize(str, dispResX, dispResY);
        }
    }
    reinitModules();
}

//------------------------------------------
// Object Management

void Environment::append(RenderObject* toAppend, uint16_t const& dispResX, uint16_t const& dispResY, uint8_t const& layer) {
    if (layer < LayerCount) {
        roc[layer].append(toAppend, dispResX, dispResY);
    } else {
        roc[0].append(toAppend, dispResX, dispResY);
    }
}

void Environment::updateObjects(int16_t const& tilePositionX, int16_t const& tilePositionY, uint16_t const& dispResX, uint16_t const& dispResY) {
    for (unsigned int i = 0; i < LayerCount; i++) {
        roc[i].update(tilePositionX, tilePositionY, dispResX, dispResY);
    }
}

void Environment::reinsertAllObjects(uint16_t const& dispResX, uint16_t const& dispResY) {
    for (unsigned int i = 0; i < LayerCount; i++) {
        roc[i].reinsertAllObjects(dispResX, dispResY);
    }
}

RenderObject* Environment::getObjectFromId(uint32_t const& id) {
    // Go through all layers
    for (unsigned int i = 0; i < LayerCount; ++i) {
        if (auto const obj = roc[i].getObjectFromId(id); obj != nullptr) {
            return obj;
        }
    }
    return nullptr;
}

//------------------------------------------
// Container Management

std::vector<RenderObjectContainer::Batch>& Environment::getContainerAt(int16_t x, int16_t y, Layer layer) {
    auto const pos = std::make_pair(x, y);
    if (static_cast<uint8_t>(layer) < LayerCount) {
        return roc[static_cast<uint8_t>(layer)].getContainerAt(pos);
    }
    return roc[0].getContainerAt(pos);
}

bool Environment::isValidPosition(int x, int y, Layer layer) {
    auto const pos = std::make_pair(x, y);
    if (static_cast<uint8_t>(layer) < LayerCount) {
        return roc[static_cast<uint8_t>(layer)].isValidPosition(pos);
    }
    return roc[0].isValidPosition(pos);
}

void Environment::purgeObjects() {
    // Release resources for ObjectContainer
    for (unsigned int i = 0; i < LayerCount; i++) {
        roc[i].purgeObjects();
    }
}

size_t Environment::getObjectCount() const {
    return std::accumulate(
        roc.begin(), roc.end(), 0u,
        [](size_t const acc, RenderObjectContainer const& container) {
            return acc + container.getObjectCount();
        }
        );
}

} // namespace Nebulite::Core
