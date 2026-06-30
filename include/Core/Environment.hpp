#ifndef CORE_ENVIRONMENT_HPP
#define CORE_ENVIRONMENT_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cstddef> // NOLINT
#include <cstdint> // NOLINT
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Constants/Event.hpp"
#include "Data/RenderObjectContainer.hpp"
#include "Data/Tiling.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class RendererProcessor;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Core {
/**
 * @class Nebulite::Core::Environment
 * @brief Represents the container of all render objects loaded in memory.
 * @details This class is responsible for containing all RenderObject instances.
 *          Lifecycle management is handled inside the RenderObjectContainer.
 *          The environment is split into multiple layers, each containing a grid of render objects.
 *          The grid size depends on the display resolution:
 *          `<display_resolution_x> * <display_resolution_y>`
 */
class Environment final : public Interaction::Execution::Domain {
public:
    /**
     * @enum Nebulite::Core::Environment::Layer
     * @brief Enum representing the different rendering layers.
     * @details Each layer is technically responsible for a specific type of rendering.
     *          However, there is no real distinction in how the layers are processed.
     *          The only difference is the order in which they are rendered.
     *          Each layer can be thought of as a separate "pass" over the render objects.
     *          Starting with the lowest layer (background) and ending with the highest layer (menu).
     *          *IMPORTANT:* New layers must be added to private variable `allLayers` in the correct order.
     */
    enum class Layer : std::uint8_t {
        background = 0,
        general,
        foreground,
        effects
    };

    static auto constexpr FinalLayer = Layer::effects;

private:

    // All layers in rendering order
    static std::array constexpr allLayers = {
        Layer::background, // Special layer: uses pre-calculated textures. Only updated on object removal/insertion
        Layer::general,
        Layer::foreground,
        Layer::effects
    };

    static_assert(allLayers.back() == FinalLayer, "Layer ordering changed, please review code.");

    // Inner RenderObject container layers
    std::array<Data::RenderObjectContainer, allLayers.size()> roc;

    //------------------------------------------
    // Append index to domain id

    absl::flat_hash_map<std::size_t, std::size_t> indexToIdMap;
    std::size_t indexCounter = 1; // Start at 1 to avoid confusion with default value of 0
public:

    //------------------------------------------
    // Layers

    /**
     * @brief Retrieves all layers in rendering order.
     * @return the vector of all layers.
     */
    static auto constexpr& getAllLayerTypes() {
        return allLayers;
    }

    auto const& getAllLayers() {
        return roc;
    }

    static auto constexpr layerCount = allLayers.size();

    //------------------------------------------
    // Special Members

    explicit Environment(Data::JsonScope& documentReference, Utility::IO::Capture& parentCapture);

    ~Environment() override ;

    // Suppress copy/move operators
    Environment(Environment const&) = delete;
    Environment& operator=(Environment const&) = delete;
    Environment(Environment&&) = delete;
    Environment& operator=(Environment&&) = delete;

    //------------------------------------------
    // Domain-related

    [[nodiscard]] Constants::Event update() override;

    //------------------------------------------
    // Marshalling

    /**
     * @brief Serializes the Environment to a JSON string.
     * @details The serialized JSON string consists of n many arrays, one for each layer.
     *          Each array contains the serialized representation of the RenderObjects in that layer.
     * @return A JSON string representation of the Environment.
     */
    std::string serialize();

    /**
     * @brief Deserializes the Environment from a JSON string.
     * @details The deserialized JSON string is expected to have the same structure as the serialized format.
     *          See `serialize()` for more details.
     * @param serialOrLink The JSON string to deserialize or a link to the JSON file.
     * @param tilingInformation Width and height of each tile
     */
    void deserialize(std::string const& serialOrLink, Data::TilingInformation const& tilingInformation);

    //------------------------------------------
    // Object Management

    /**
     * @brief Appends a new RenderObject to the specified layer of the environment.
     * @param toAppend Pointer to the RenderObject to append.
     * @param tilingInformation Width and height of each tile
     * @param layer Layer index to append the object to (default is 0).
     */
    void append(RenderObject* toAppend, Data::TilingInformation const& tilingInformation, std::uint8_t layer = 0);

    /**
     * @brief Updates the environment's state.
     * @param tiles The tiles to update
     * @param tilingInformation Width and height of each tile
     * @param rendererProcessor the RendererProcessor instance to use for parallel processing of batches.
     */
    void updateObjects(std::vector<Data::TileCoordinate> const& tiles, Data::TilingInformation const& tilingInformation, Data::RendererProcessor& rendererProcessor);

    /**
     * @brief Rebuilds the Container structure.
     * @details Responsible for reinserting all render objects into their respective containers.
     * @param tilingInformation Width and height of each tile
     */
    void reinsertAllObjects(Data::TilingInformation const& tilingInformation);

    /**
     * @brief Retrieves a RenderObject by its ID.
     * @param domainId The ID of the RenderObject to retrieve.
     * @return A pointer to the RenderObject if found, nullptr otherwise.
     */
    RenderObject* getObjectFromId(std::size_t domainId);

    //------------------------------------------
    // Get object

    /**
     * @brief Gets the RenderObject ID from its index
     * @param index The index of the RenderObject
     * @return An optional containing the ID of the RenderObject if found, or std::nullopt if no object is associated with the given index.
     */
    [[nodiscard]] std::optional<std::size_t> getIdFromIndex(std::size_t index) const ;

    /**
     * @brief Gets the RenderObject index from its ID.
     * @param domainId The domain ID of the RenderObject to search for.
     * @return An optional containing the index of the RenderObject if found, or std::nullopt if no object is associated with the given ID.
     */
    [[nodiscard]] std::optional<std::size_t> getIndexFromId(std::size_t domainId) const ;

    /**
     * @brief Gets the RenderObject from its ID.
     * @param searchIndex The Index of the RenderObject to retrieve. In chronological order based on when they were appended to the Renderer.
     *        Does not change when objects are removed or purged.
     * @return A pointer to the RenderObject, or nullptr if not found.
     */
    std::optional<std::pair<RenderObject*, Data::JsonScope*>> getObjectFromIndex(std::size_t searchIndex) ;

    //------------------------------------------
    // Container Management

    /**
     * @brief Retrieves the RenderObjectContainer at the specified position and layer.
     * @param pos The tile position
     * @param layer The layer index.
     * @return A reference to the RenderObjectContainer at the specified position and layer.
     */
    Data::Tile& getContainerAt(Data::TileCoordinate const& pos, Layer layer);

    /**
     * @brief Checks if the specified position and layer are valid, meaning they are within the bounds of the environment.
     * @param pos The tile position
     * @param layer The layer index.
     * @return True if the position and layer are valid, false otherwise.
     */
    [[nodiscard]] bool isValidPosition(Data::TileCoordinate const& pos, Layer layer) const ;

    /**
     * @brief Purges all objects from the environment by placing them in the deletion process.
     */
    void purgeObjects();

    /**
     * @brief Retrieves the total number of render objects in the environment.
     * @return The total number of render objects in the environment.
     */
    [[nodiscard]] std::size_t getObjectCount() const;

    /**
     * @brief Iterate over all layers, providing access to the tile position, layer, and batches of render objects in each container.
     * @param function iterator function
     */
    void containerIteration(Data::RenderObjectContainer::IteratorFunction<Layer> const& function) {
        for (auto const [index, container] : roc | std::views::enumerate) {
            container.containerIteration(function, static_cast<Layer>(index));
        }
    }

    //------------------------------------------
    // Viewport

    struct TileAndCoordinate {
        Data::Tile* tile;
        Data::TileCoordinate coordinate;
    };

    auto viewport(std::vector<Data::TileCoordinate> const& visibleTiles, Layer const layer) {
        std::vector<TileAndCoordinate> result;
        result.reserve(visibleTiles.size());
        for (auto const& tileCoordinate : visibleTiles) {
            if (isValidPosition(tileCoordinate, layer)) {
                result.emplace_back(TileAndCoordinate{
                    .tile = &getContainerAt(tileCoordinate, layer),
                    .coordinate = tileCoordinate
                });
            }
        }
        return result;
    }
};
} // namespace Nebulite::Core

#endif // CORE_ENVIRONMENT_HPP
