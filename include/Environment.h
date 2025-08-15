#pragma once

#include <iostream>
#include <string>
#include "absl/container/flat_hash_map.h"
#include <utility>
#include <vector>


#include "RenderObjectContainer.h"
#include "Invoke.h"
#include "JSON.h"



#define RENDEROBJECTCONTAINER_COUNT 5

namespace Nebulite{

/**
 * @brief Represents the container of all render objects loaded in memory.
 *
 * This class is responsible for containing all RenderObject instances.
 * Lifecycle management is handled inside the RenderObjectContainer.
 * The environment is split into multiple layers, each containing a grid of render objects.
 * The grid size depends on the display resolution: <display_resolution_x> x <display_resolution_y>
 */
class Environment {
public:
	//-----------------------------------------------------------
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
	Environment(Nebulite::Invoke* globalInvoke);

	// Suppress copy/move operators
	Environment(Environment&& other) = delete;
	Environment& operator=(Environment&& other) = delete;
	Environment& operator=(const Environment& other) = delete;

	//-----------------------------------------------------------
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
	 */
	void deserialize(std::string serialOrLink, int dispResX,int dispResY);
	
	//-----------------------------------------------------------
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
	void append(Nebulite::RenderObject* toAppend,int dispResX, int dispResY, int layer = 0);

	/**
	 * @brief Updates the environment's state.
	 * 
	 * This function is responsible for updating the state of all render objects in the environment.
	 * 
	 * @param tileXpos current camera tile position in the X direction.
	 * @param tileYpos current camera tile position in the Y direction.
	 * @param dispResX display resolution width (necessary for potential RenderObject reinsertions).
	 * @param dispResY display resolution height (necessary for potential RenderObject reinsertions).
	 * @param globalInvoke pointer to the global Invoke instance.
	 */
	void update(int16_t tileXpos, int16_t tileYpos,int dispResX,int dispResY,Nebulite::Invoke* globalInvoke);

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
	Nebulite::RenderObject* getObjectFromId(uint32_t id);

	//-----------------------------------------------------------
	// Container Management

	/**
	 * @brief Retrieves the RenderObjectContainer at the specified position and layer.
	 * 
	 * @param x The X coordinate of the tile.
	 * @param y The Y coordinate of the tile.
	 * @param layer The layer index.
	 * @return A reference to the RenderObjectContainer at the specified position and layer.
	 */
	std::vector<Nebulite::RenderObjectContainer::batch>& getContainerAt(int16_t x, int16_t y, int layer);

	/**
	 * @brief Checks if the specified position and layer are valid, meaning they are within the bounds of the environment.
	 * 
	 * @param x The X coordinate of the tile.
	 * @param y The Y coordinate of the tile.
	 * @param layer The layer index.
	 * @return True if the position and layer are valid, false otherwise.
	 */
	bool isValidPosition(int x, int y, int layer);

	/**
	 * @brief Purges all objects from the environment.
	 * 
	 * This function removes all render objects from all layers.
	 * 
	 * TODO: The approach of direct deletion is not compatible with the current architecture.
	 * Move all objects into trash before deletion.
	 */
	void purgeObjects();

	/**
	 * @brief Retrieves the total number of render objects in the environment.
	 * 
	 * @return The total number of render objects in the environment.
	 */
	size_t getObjectCount();

private:
	// Link to Global Values
    Nebulite::JSON* global;

	// Inner containers
	std::array<Nebulite::RenderObjectContainer, RENDEROBJECTCONTAINER_COUNT> roc;
};
}