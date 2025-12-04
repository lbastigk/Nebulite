/**
 * @file Environment.hpp
 * @brief Contains the definition of the Nebulite::Core::Environment class.
 */

#ifndef NEBULITE_CORE_ENVIRONMENT_HPP
#define NEBULITE_CORE_ENVIRONMENT_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <vector>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Core/RenderObjectContainer.hpp"

//------------------------------------------
namespace Nebulite::Core {
/**
 * @class Nebulite::Core::Environment
 * @brief Represents the container of all render objects loaded in memory.
 *
 * This class is responsible for containing all RenderObject instances.
 * Lifecycle management is handled inside the RenderObjectContainer.
 * The environment is split into multiple layers, each containing a grid of render objects.
 * The grid size depends on the display resolution:
 * `<display_resolution_x> * <display_resolution_y>`
 */
NEBULITE_DOMAIN(Environment) {
public:
    /**
     * @enum Nebulite::Core::Environment::Layer
     * @brief Enum representing the different rendering layers.
     *
     * Each layer is technically responsible for a specific type of rendering.
     * However, there is no real distinction in how the layers are processed.
     * The only difference is the order in which they are rendered.
     * Each layer can be thought of as a separate "pass" over the render objects.
     * Starting with the lowest layer (background) and ending with the highest layer (menu).
     *
     * *IMPORTANT:* New layers must be added to private variable `allLayers` in the correct order.
     *
     * @todo Once GUI DomainModule and renderer texture queuing is properly implemented,
     *       the layer size may be reduced and layer names reworked.
     */
    enum class Layer : uint8_t {
        background,
        general,
        foreground,
        effects,
        UI
    };

    /**
     * @brief Retrieves all layers in rendering order.
     * @return the vector of all layers.
     */
    std::vector<Layer>* getAllLayers() {
        return &allLayers;
    }

    /**
     * @brief The number of RenderObjectContainer layers in the Environment.
     */
    static constexpr uint8_t LayerCount = 5;

    //------------------------------------------
    //Constructor

    explicit Environment(Nebulite::Utility::JSON* documentPtr);

    // Suppress copy/move operators
    Environment(Environment&& other) = delete;
    Environment& operator=(Environment&& other) = delete;
    Environment& operator=(Environment const& other) = delete;

    //------------------------------------------
    // Marshalling

    /**
     * @brief Serializes the Environment to a JSON string.
     *
     * The serialized JSON string consists of n many arrays, one for each layer.
     * Each array contains the serialized representation of the RenderObjects in that layer.
     *
     * @return A JSON string representation of the Environment.
     */
    std::string serialize();

    /**
     * @brief Deserializes the Environment from a JSON string.
     *
     * The deserialized JSON string is expected to have the same structure as the serialized format.
     * See `serialize()` for more details.
     *
     * @param serialOrLink The JSON string to deserialize or a link to the JSON file.
     * @param dispResX Display resolution width. Necessary to position the object correctly in its tile-based container.
     * @param dispResY Display resolution height. Necessary to position the object correctly in its tile-based container.
     */
    void deserialize(std::string const& serialOrLink, uint16_t const& dispResX, uint16_t const& dispResY);

    //------------------------------------------
    // Object Management

    /**
     * @brief Appends a RenderObject to the environment.
     *
     * This function adds a new RenderObject to the specified layer of the environment.
     *
     * @param toAppend Pointer to the RenderObject to append.
     * @param dispResX Display resolution width. Necessary to position the object correctly in its tile-based container.
     * @param dispResY Display resolution height. Necessary to position the object correctly in its tile-based container.
     * @param layer Layer index to append the object to (default is 0).
     */
    void append(RenderObject* toAppend, uint16_t const& dispResX, uint16_t const& dispResY, uint8_t const& layer = 0);

    /**
     * @brief Updates the environment's state.
     * @param tilePositionX current camera tile position in the X direction.
     * @param tilePositionY current camera tile position in the Y direction.
     * @param dispResX display resolution width. Necessary for potential RenderObject reinsertions.
     * @param dispResY display resolution height. Necessary for potential RenderObject reinsertions.
     */
    void updateObjects(int16_t const& tilePositionX, int16_t const& tilePositionY, uint16_t const& dispResX, uint16_t const& dispResY);

    /**
     * @brief Rebuilds the Container structure.
     *
     * This function is responsible for reinserting all render objects into their respective containers.
     *
     * @param dispResX Display resolution width. Necessary for positioning the objects correctly in their tile-based containers.
     * @param dispResY Display resolution height. Necessary for positioning the objects correctly in their tile-based containers.
     */
    void reinsertAllObjects(uint16_t const& dispResX, uint16_t const& dispResY);

    /**
     * @brief Retrieves a RenderObject by its ID.
     *
     * @param id The ID of the RenderObject to retrieve.
     * @return A pointer to the RenderObject if found, nullptr otherwise.
     */
    RenderObject* getObjectFromId(uint32_t const& id);

    //------------------------------------------
    // Container Management

    /**
     * @brief Retrieves the RenderObjectContainer at the specified position and layer.
     *
     * @param x The X coordinate of the tile.
     * @param y The Y coordinate of the tile.
     * @param layer The layer index.
     * @return A reference to the RenderObjectContainer at the specified position and layer: A vector of batched RenderObjects.
     */
    std::vector<RenderObjectContainer::Batch>& getContainerAt(int16_t x, int16_t y, Layer layer);

    /**
     * @brief Checks if the specified position and layer are valid, meaning they are within the bounds of the environment.
     *
     * @param x The X coordinate of the tile.
     * @param y The Y coordinate of the tile.
     * @param layer The layer index.
     * @return True if the position and layer are valid, false otherwise.
     */
    bool isValidPosition(int x, int y, Layer layer);

    /**
     * @brief Purges all objects from the environment by placing them in the deletion process.
     */
    void purgeObjects();

    /**
     * @brief Retrieves the total number of render objects in the environment.
     *
     * @return The total number of render objects in the environment.
     */
    [[nodiscard]] size_t getObjectCount() const;

private:
    // All layers in rendering order
    std::vector<Layer> allLayers = {Layer::background, Layer::general, Layer::foreground, Layer::effects, Layer::UI};

    // Inner RenderObject container layers
    std::array<RenderObjectContainer, LayerCount> roc;
};
} // namespace Nebulite::Core

#endif // NEBULITE_CORE_ENVIRONMENT_HPP