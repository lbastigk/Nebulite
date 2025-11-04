/**
 * @file RenderObject.hpp
 * @brief Declaration of the RenderObject class.
 * 
 * Defines the Nebulite::Core::RenderObject class.
 */

#ifndef NEBULITE_CORE_RENDEROBJECT_HPP
#define NEBULITE_CORE_RENDEROBJECT_HPP

//------------------------------------------
// Includes

// External
#include <SDL.h>		// SDL Renderer is used for some methods to calculate text
#include <SDL_ttf.h>	// Same for ttf

// Nebulite
#include "Constants/KeyNames.hpp"
#include "Core/Texture.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Utility/JSON.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
	namespace Core{
		class GlobalSpace; // Forward declaration of core class GlobalSpace
	}
	namespace Interaction{
		class Invoke; 		// Forward declaration of interaction class Invoke
		struct Ruleset; 	// Forward declaration of interaction struct Ruleset
	}
}

//------------------------------------------
namespace Nebulite::Core {
/**
 * @class Nebulite::Core::RenderObject
 * @brief Represents a renderable object in the Nebulite engine. 
 * This class encapsulates all data and logic needed to
 * display, update, and interact with a single object on the screen.
 *
 * Key Features:
 * 
 *   - Stores all render-related data in an internal JSON document for flexible property management.
 * 
 *   - Provides methods for serialization and deserialization to and from strings or files.
 * 
 *   - Supports dynamic property access and modification via set/get templates.
 * 
 *   - Manages SDL_Rect structures for sprite and text positioning, and caches these for performance.
 * 
 *   - Handles text rendering using SDL_ttf, including texture creation and caching.
 * 
 *   - Supports invoke command lists for both general (global) and internal (local) interactions.
 * 
 *   - Offers update and reload mechanisms to synchronize state with global invokes and JSON data.
 * 
 *   - Calculates computational cost estimates for update operations.
 * 
 *   - allows for flexible additions through DomainModules for local function calls.
 *
 * Interaction with Invoke:
 * 
 *   - Invokes are loaded in from JSON files and categorized into global and local commands.
 * 
 *   - Global commands are based on a self-other-global interaction model, while local commands are focused on self-global interactions.
 *     They are broadcasted to the Invoke class on defined topics to find matching objects that are listening to those topics.
 * 
 *   - Local commands are defined by an empty topic and are based on a self-global interaction model.
 *
 * Usage:
 *
 *   - Instantiate a RenderObject to represent a sprite, text, or other visual entity. Create a JSON document to hold its properties.
 *
 *   - Append the RenderObject to the Renderer via `./bin/Nebulite spawn myObject.json`
 * 
 * Copy and move operations are disabled to prevent accidental resource duplication.
 */
NEBULITE_DOMAIN(RenderObject){
public:
	//------------------------------------------
	// Special member Functions

	/**
	 * @brief Constructs a new RenderObject.
	 * 
	 * RenderObjects are initialized with reference to the Globalspace.
	 * 
	 * @param globalSpace A pointer to the Globalspace instance.
	 */
	explicit RenderObject(GlobalSpace* globalSpace);

	/**
	 * @brief Destroys the RenderObject.
	 * 
	 * Cleans up any resources used by the RenderObject, including
	 * textures and surfaces.
	 */
	~RenderObject() override;

	// Suppress copy/move operators
	RenderObject(RenderObject const& other) = delete;
	RenderObject(RenderObject&& other) = delete;
	RenderObject& operator=(RenderObject&& other) = delete;

	//------------------------------------------
	// Set/Get

    /**
	 * @brief Sets a value in the Domain's JSON document.
	 * 
	 * @tparam T The type of the value to set.
	 * @param key The key of the value to set.
	 * @param data The value to set.
	 */
    template <typename T> void set(char const* key, T const& data);

    /**
     * @brief Gets a value from the Domain's JSON document.
     * 
     * @tparam T The type of the value to get.
     * @param key The key of the value to get.
     * @param defaultValue The default value to return if the key is not found.
     * @return The value from the JSON document, or the default value if not found.
     */
    template <typename T> T get(char const* key, T const& defaultValue = T());


	//------------------------------------------
	// Serializing/Deserializing

	/**
	 * @brief Serializes the RenderObject to a JSON string.
	 * 
	 * @return A string representation of the RenderObject's JSON document.
	 */
	std::string serialize();

	/**
	 * @brief Deserializes the RenderObject from a JSON string.
	 * 
	 * @param serialOrLink The JSON string to deserialize.
	 */
	void deserialize(std::string const& serialOrLink);

	/**
	 * @brief Gets a pointer to the SDL_Rect describing the destination of the sprite.
	 * 
	 * @return A pointer to the SDL_Rect describing the destination of the sprite.
	 */
	SDL_Rect* getDstRect();

	/**
	 * @brief Gets a pointer to the SDL_Rect describing the source of the sprite.
	 * 
	 * @return A pointer to the SDL_Rect describing the source of the sprite.
	 */
	SDL_Rect* getSrcRect();

	/**
	 * @brief Gets a pointer to the SDL_Rect describing the destination of the text.
	 * 
	 * @return A pointer to the SDL_Rect describing the destination of the text.
	 */
	SDL_Rect* getTextRect();

	/**
	 * @brief Gets the texture of the text.
	 * 
	 * @return A pointer to the SDL_Texture representing the text.
	 */
	SDL_Texture* getTextTexture();
	
	//------------------------------------------
	// Update-Oriented functions

	/**
	 * @brief Updates the RenderObject based on the given global invoke.
	 * 
	 * - updates the internal FuncTrees
	 * 
	 * - reloads invokes if needed
	 * 
	 * - updates local invokes
	 * 
	 * - listens to global invokes
	 * 
	 * - broadcasts its own global invokes
	 * 
	 * - calculates source and destination rects
	 *
	 * @return Constants::Error indicating success or failure.
	 */
	Constants::Error update() override;

	/**
	 * @brief Calculates the text texture for the RenderObject.
	 * 
	 * @param renderer The SDL_Renderer to use for rendering.
	 * @param font The TTF_Font to use for rendering the text.
	 * @param renderPositionX The X position of the renderer used for text offset.
	 * @param renderPositionY The Y position of the renderer used for text offset.
	 */
	void calculateText(SDL_Renderer* renderer,TTF_Font* font, int renderPositionX, int renderPositionY);

	/**
	 * @brief Calculates the destination rectangle for the sprite.
	 */
	void calculateDstRect();

	// Calculate sprite source
	/**
	 * @brief Calculates the source rectangle for the sprite.
	 */
	void calculateSrcRect();

	/**
	 * @brief Estimates the computational cost of updating the RenderObject.
	 * 
	 * Based on the amount of evaluations and variables in the ruleset.
	 * 
	 * @param onlyInternal If true, only considers internal rulesets. Defaults to true.
	 * @return The estimated computational cost.
	 */
	uint64_t estimateComputationalCost(bool onlyInternal = true);

	//------------------------------------------
	// Management Flags for Renderer-Interaction

	/**
	 * @struct flag
	 * @brief Flags for managing RenderObject behavior
	 */
	struct flag{
		bool deleteFromScene = false;	// If true, delete this object from scene on next update
		bool calculateText = false;		// If true, calculate text texture on next update
		bool reloadInvokes = false;		// If true, reload invokes on next update
	} flag;

	//------------------------------------------
	// Texture related

	/**
     * @brief Links an external SDL_Texture to this domain.
     * 
     * @param externalTexture Pointer to the external SDL_Texture.
     */
    void linkExternalTexture(SDL_Texture* externalTexture){
        baseTexture.linkExternalTexture(externalTexture);
    }

    /**
     * @brief Checks if the texture has been modified.
     * 
     * @return true if the texture has been modified, false otherwise.
     */
    [[nodiscard]] bool isTextureStoredLocally() const {
        return baseTexture.isTextureStoredLocally();
    }

    /**
     * @brief Checks if the texture is valid (not null).
     * 
     * @return true if the texture is valid, false otherwise.
     */
    [[nodiscard]] bool isTextureValid() const {
        return baseTexture.isTextureValid();
    }

    /**
     * @brief Gets the current SDL_Texture.
     * 
     * @return Pointer to the current SDL_Texture.
     */
    [[nodiscard]] SDL_Texture* getSDLTexture() const {
        return baseTexture.getSDLTexture();
    }

	Texture* getTexture(){
		return &baseTexture;
	}

private:
	// Main doc holding values
	Utility::JSON json;

	// Size of subscriptions
	size_t subscription_size = 0;

	//------------------------------------------
	// References to JSON

	/**
	 * @struct FrequentRefs
	 * @brief Holds frequently used references for quick access.
	 */
	struct FrequentRefs{
		// Identity
		double* id;

		// Position and Size
		double* posX;
		double* posY;
		double* pixelSizeX;
		double* pixelSizeY;

		// Spritesheet
		double* isSpritesheet;
		double* spritesheetOffsetX;
		double* spritesheetOffsetY;
		double* spritesheetSizeX;
		double* spritesheetSizeY;

		// Text
		double* fontSize;
		double* textDx;
		double* textDy;
		double* textColorR;
		double* textColorG;
		double* textColorB;
		double* textColorA;
	} refs;

	/**
	 * @brief Links frequently used references from the JSON document for quick access.
	 */
	void linkFrequentRefs(){
		// Identity
		refs.id                 = json.getStableDoublePointer(Constants::keyName.renderObject.id);

		// Position and Size
		refs.posX			    = json.getStableDoublePointer(Constants::keyName.renderObject.positionX);
		refs.posY			    = json.getStableDoublePointer(Constants::keyName.renderObject.positionY);
		refs.pixelSizeX         = json.getStableDoublePointer(Constants::keyName.renderObject.pixelSizeX);
		refs.pixelSizeY         = json.getStableDoublePointer(Constants::keyName.renderObject.pixelSizeY);

		// Spritesheet
		refs.isSpritesheet      = json.getStableDoublePointer(Constants::keyName.renderObject.isSpritesheet);
		refs.spritesheetOffsetX = json.getStableDoublePointer(Constants::keyName.renderObject.spritesheetOffsetX);
		refs.spritesheetOffsetY = json.getStableDoublePointer(Constants::keyName.renderObject.spritesheetOffsetY);
		refs.spritesheetSizeX   = json.getStableDoublePointer(Constants::keyName.renderObject.spritesheetSizeX);
		refs.spritesheetSizeY   = json.getStableDoublePointer(Constants::keyName.renderObject.spritesheetSizeY);

		// Text
		refs.fontSize           = json.getStableDoublePointer(Constants::keyName.renderObject.textFontsize);
		refs.textDx				= json.getStableDoublePointer(Constants::keyName.renderObject.textDx);
		refs.textDy				= json.getStableDoublePointer(Constants::keyName.renderObject.textDy);
		refs.textColorR         = json.getStableDoublePointer(Constants::keyName.renderObject.textColorR);
		refs.textColorG         = json.getStableDoublePointer(Constants::keyName.renderObject.textColorG);
		refs.textColorB         = json.getStableDoublePointer(Constants::keyName.renderObject.textColorB);
		refs.textColorA         = json.getStableDoublePointer(Constants::keyName.renderObject.textColorA);
	}

	//------------------------------------------
	// Texture related

	// Base Texture
	Texture baseTexture;
	
	// === TO REWORK ===

	// for caching of SDL Positions
	SDL_Rect dstRect;	// destination of sprite
	SDL_Rect srcRect;	// source of sprite from spritesheet
	SDL_Rect textRect;	// destination of text texture

	// Surface and Texture of Text
	SDL_Surface* textSurface;	// Surface for the text
	SDL_Texture* textTexture;	// Texture for the text

	// ==================


	//------------------------------------------
	// Invoke Commands
	std::vector<std::shared_ptr<Interaction::Ruleset>> entries_global;	// Global commands, intended for self-other-global interaction
	std::vector<std::shared_ptr<Interaction::Ruleset>> entries_local;		// Internal commands, intended for self-global interaction

	//------------------------------------------
	// Linkages

	// Linkage to the entire global space
	GlobalSpace* globalSpace;

	// Each RenderObject with linkage to the global document
	Utility::JSON* global;

	// Linkage to the global invoke for update calls
	Interaction::Invoke* invoke;
};
} // namespace Nebulite::Core


//------------------------------------------
// Templated setter/getter functions

template <typename T> void Nebulite::Core::RenderObject::set(char const* key, T const& data){
	json.set(key,data);
}

template <typename T> T Nebulite::Core::RenderObject::get(char const* key, T const& defaultValue){
	T result = json.get<T>(key,defaultValue);
	return result;
}
#endif // NEBULITE_CORE_RENDEROBJECT_HPP
