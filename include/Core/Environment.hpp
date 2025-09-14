/**
 * @file Environment.hpp
 * @brief Contains the definition of the Nebulite::Core::Environment class.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// External
#include "absl/container/flat_hash_map.h"

// Nebulite
#include "Nebulite.hpp"
#include "Core/RenderObjectContainer.hpp"
#include "Interaction/Invoke.hpp"
#include "Utility/JSON.hpp"

//------------------------------------------
namespace Nebulite{
namespace Core{
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
class Environment {
public:
	/**
	 * @enum Nebulite::Core::Environment::Layer
	 * @brief Enum representing the different rendering layers.
	 * 
	 * Each layer is technically responsible for a specific type of rendering.
	 * However, there is no real distinction in how the layers are processed.
	 * The only difference is the order in which they are rendered.
	 * Each layer can be thought of as a separate "pass" over the render objects.
	 * Starting with the lowest layer (background) and ending with the highest layer (menue).
	 * 
	 * @todo Once GDM_GUI and renderer texture queuing is properly implemented, 
	 * the layer size may be reduced and layer names reworked.
	 */
	enum Layer {
		background,
		general,
		foreground,
		effects,
		menue
	};

	/**
	 * @brief The number of RenderObjectContainer layers in the Environment.
	 */
	const static unsigned int LayerCount = 5;

	//------------------------------------------
	//Constructor

	/**
	 * @brief Constructs the Environment with a global Invoke instance.
	 *
	 * Creates an environment with its subcontainers for proper layer-based rendering.
	 *
	 * @param globalInvoke Pointer to the global Invoke instance.
	 * The Global Invoke instance is necessary for the Environment and its Container Layers to communicate with the global space.
	 * This is necessary for:
	 * 
	 * - RenderObject updates
	 * 
	 * - RenderObject cost estimation
	 * 
	 * - RenderObject creation
	 */
	Environment(Nebulite::Interaction::Invoke* globalInvoke);

	// Suppress copy/move operators
	Environment(Environment&& other) = delete;
	Environment& operator=(Environment&& other) = delete;
	Environment& operator=(const Environment& other) = delete;

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
	void deserialize(std::string serialOrLink, int dispResX,int dispResY);
	
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
	void append(Nebulite::Core::RenderObject* toAppend,int dispResX, int dispResY, int layer = 0);

	/**
	 * @brief Updates the environment's state.
	 * 
	 * This function is responsible for updating the state of all render objects in the environment.
	 * 
	 * @param tileXpos current camera tile position in the X direction.
	 * @param tileYpos current camera tile position in the Y direction.
	 * @param dispResX display resolution width. Necessary for potential RenderObject reinsertions).
	 * @param dispResY display resolution height. Necessary for potential RenderObject reinsertions).
	 * @param globalInvoke pointer to the global Invoke instance.
	 */
	void update(int16_t tileXpos, int16_t tileYpos, int dispResX, int dispResY, Nebulite::Interaction::Invoke* globalInvoke);

	/**
	 * @brief Rebuilds the Container structure.
	 * 
	 * This function is responsible for reinserting all render objects into their respective containers.
	 * 
	 * @param dispResX Display resolution width. Necessary for positioning the objects correctly in their tile-based containers.
	 * @param dispResY Display resolution height. Necessary for positioning the objects correctly in their tile-based containers.
	 */
	void reinsertAllObjects(int dispResX,int dispResY);

	/**
	 * @brief Retrieves a RenderObject by its ID.
	 * 
	 * @param id The ID of the RenderObject to retrieve.
	 * @return A pointer to the RenderObject if found, nullptr otherwise.
	 */
	Nebulite::Core::RenderObject* getObjectFromId(uint32_t id);

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
	std::vector<Nebulite::Core::RenderObjectContainer::batch>& getContainerAt(int16_t x, int16_t y, Environment::Layer layer);

	/**
	 * @brief Checks if the specified position and layer are valid, meaning they are within the bounds of the environment.
	 * 
	 * @param x The X coordinate of the tile.
	 * @param y The Y coordinate of the tile.
	 * @param layer The layer index.
	 * @return True if the position and layer are valid, false otherwise.
	 */
	bool isValidPosition(int x, int y, Environment::Layer layer);

	/**
	 * @brief Purges all objects from the environment by placing them in the deletion process.
	 */
	void purgeObjects();

	/**
	 * @brief Retrieves the total number of render objects in the environment.
	 * 
	 * @return The total number of render objects in the environment.
	 */
	uint32_t getObjectCount();

private:
	// Link to Global Values
    Nebulite::Utility::JSON* global;

	// Inner RenderObject container layers
	std::array<Nebulite::Core::RenderObjectContainer, Nebulite::Core::Environment::LayerCount> roc;
};
} // namespace Core
} // namespace Nebulite