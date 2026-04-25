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

#include "Core/RenderObject.hpp"
#include "Data/RenderObjectContainer.hpp"
#include "Data/Document/JSON.hpp"
#include "Module/Domain/Initializer.hpp"
#include "Utility/Generate.hpp"

//------------------------------------------

namespace Nebulite::Core {

Environment::Environment(Data::JsonScope& documentReference, Utility::IO::Capture& parentCapture)
    : Domain("Environment", documentReference, parentCapture),
      roc(Utility::Generate::array<Data::RenderObjectContainer, LayerCount>([](std::size_t) {
          return Data::RenderObjectContainer{};
      }))
{
    Module::Domain::Initializer::initEnvironment(this);
}

Environment::~Environment() = default;

//------------------------------------------
// Domain-related

Constants::Event Environment::update() {
    updateModules();
    return Constants::Event::Success;
}

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
            roc[i].deserialize(str, dispResX, dispResY, capture);
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

void Environment::updateObjects(int16_t const& tilePositionX, int16_t const& tilePositionY, uint16_t const& dispResX, uint16_t const& dispResY, Data::RendererProcessor const& rendererProcessor) {
    for (unsigned int i = 0; i < LayerCount; i++) {
        rendererProcessor.prepareForNewLayer(&roc[i]);
        roc[i].update(tilePositionX, tilePositionY, dispResX, dispResY, rendererProcessor);
    }
}

void Environment::reinsertAllObjects(uint16_t const& dispResX, uint16_t const& dispResY) {
    for (unsigned int i = 0; i < LayerCount; i++) {
        roc[i].reinsertAllObjects(dispResX, dispResY);
    }
}

RenderObject* Environment::getObjectFromId(size_t const& domainId) {
    // Go through all layers
    for (unsigned int i = 0; i < LayerCount; ++i) {
        if (auto const obj = roc[i].getObjectFromId(domainId); obj != nullptr) {
            return obj;
        }
    }
    return nullptr;
}

//------------------------------------------
// Container Management

std::vector<Data::Batch>& Environment::getContainerAt(int16_t x, int16_t y, Layer layer) {
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
        [](size_t const acc, Data::RenderObjectContainer const& container) {
            return acc + container.getObjectCount();
        }
    );
}

} // namespace Nebulite::Core
