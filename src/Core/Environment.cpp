//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

// Nebulite
#include "Constants/Event.hpp"
#include "Core/Environment.hpp"
#include "Core/RenderObject.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/KeyType.hpp"
#include "Data/RenderObjectContainer.hpp"
#include "Data/RendererProcessor.hpp"
#include "Data/Tiling.hpp"
#include "Module/Domain/Initializer.hpp"
#include "Nebulite.hpp"
#include "Utility/Generate.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Core {

Environment::Environment(Data::JsonScope& documentReference, Utility::IO::Capture& parentCapture)
    : Domain("Environment", documentReference, parentCapture)
    , roc(Utility::Generate::array<Data::RenderObjectContainer, allLayers.size()>([](std::size_t) {
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
    parseTaskQueues(true);
    return Constants::Event::Success;
}

//------------------------------------------
// Marshalling

std::string Environment::serialize() {
    Data::JSON doc;

    // Serialize each container and add to the document
    for (unsigned int i = 0; i < allLayers.size(); i++) {
        std::string const key = "containerLayer" + std::to_string(i);
        std::string const serializedContainer = roc[i].serialize();

        // Add the container JSON object to the main document
        Data::JSON layer;
        layer.deserialize(serializedContainer);
        doc.setSubDoc(key, layer);
    }
    return doc.serialize();
}

void Environment::deserialize(std::string const& serialOrLink, Data::TilingInformation const& tilingInformation) {
    Data::JSON file;
    file.deserialize(serialOrLink);

    // Getting all layers
    for (unsigned int i = 0; i < allLayers.size(); i++) {
        // Check if the key exists in the document
        if (std::string const key = "containerLayer" + std::to_string(i); file.memberType(key) != Data::KeyType::null) {
            // Extract the value corresponding to the key
            Data::JSON const layer = file.getSubDoc(key);

            // Convert the JSON object to a pretty-printed string
            std::string const str = layer.serialize();

            // Serialize container layer
            roc[i].deserialize(str, tilingInformation, capture);
        }
    }
    reinitModules();
}

//------------------------------------------
// Object Management

void Environment::append(RenderObject* toAppend, Data::TilingInformation const& tilingInformation, std::uint8_t  const layer) {
    // Add domain id to map
    indexToIdMap[indexCounter] = toAppend->getId();
    indexCounter++;

    if (layer == 0 || layer >= allLayers.size()) {
        if (toAppend->estimateComputationalCost() != 0) {
            capture.log.println("Warning: Appending object with non-zero computational cost to background layer, which isn't updated. Consider moving the object to a higher layer so its rulesets are properly executed.");
        }
        roc[0].append(toAppend, tilingInformation);
    }
    roc[layer].append(toAppend, tilingInformation);
}

void Environment::updateObjects(std::vector<Data::TileCoordinate> const& tiles, Data::TilingInformation const& tilingInformation, Data::RendererProcessor const& rendererProcessor) {
    // Do not update lowest layer (background), as it is only for static tiles that do not need to be updated
    for (unsigned int i = 1; i < allLayers.size(); i++) {
        rendererProcessor.prepareForNewLayer(&roc[i]);
        roc[i].update(tiles, tilingInformation, rendererProcessor);
    }
}

void Environment::reinsertAllObjects(Data::TilingInformation const& tilingInformation) {
    for (unsigned int i = 0; i < allLayers.size(); i++) {
        roc[i].reinsertAllObjects(tilingInformation);
    }
}

RenderObject* Environment::getObjectFromId(std::size_t const& domainId) {
    // Go through all layers
    for (unsigned int i = 0; i < allLayers.size(); ++i) {
        if (auto* const obj = roc[i].getObjectFromId(domainId); obj != nullptr) {
            return obj;
        }
    }
    return nullptr;
}

//------------------------------------------
// Get object

std::optional<size_t> Environment::getIdFromIndex(std::size_t const& index) const {
    if (!indexToIdMap.contains(index)) {
        return std::nullopt; // No object with this index
    }
    return indexToIdMap.at(index);
}

std::optional<size_t> Environment::getIndexFromId(std::size_t const& domainId) const {
    for (const auto& [objIndex, objId] : indexToIdMap) {
        if (objId == domainId) {
            return objIndex; // Return the index associated with the given ID
        }
    }
    return std::nullopt; // No index found for the given ID
}

std::optional<std::pair<RenderObject*, Data::JsonScope*>> Environment::getObjectFromIndex(std::size_t const& searchIndex) {
    if (!indexToIdMap.contains(searchIndex)) {
        return std::nullopt; // No object with this index
    }
    auto const domainId = indexToIdMap[searchIndex];
    if (auto* ro = getObjectFromId(domainId); ro) {
        EnvironmentToken constexpr token;
        return std::make_pair(ro, &ro->getDocument(token));
    }
    return std::nullopt; // Object retrieval failed somehow
}

//------------------------------------------
// Container Management

Data::Tile& Environment::getContainerAt(Data::TileCoordinate const& pos, Layer layer) {
    return roc[static_cast<uint8_t>(layer)].getContainerAt(pos);
}

bool Environment::isValidPosition(Data::TileCoordinate const& pos, Layer layer) const {
    return roc[static_cast<uint8_t>(layer)].isValidPosition(pos);
}

void Environment::purgeObjects() {
    // Release resources for ObjectContainer
    for (unsigned int i = 0; i < allLayers.size(); i++) {
        roc[i].purgeObjects();
    }
}

size_t Environment::getObjectCount() const {
    return std::accumulate(
        roc.begin(), roc.end(), 0u,
        [](std::size_t const acc, Data::RenderObjectContainer const& container) {
            return acc + container.getObjectCount();
        }
    );
}

} // namespace Nebulite::Core
