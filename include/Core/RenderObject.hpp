/**
 * @file RenderObject.hpp
 * @brief Declaration of the RenderObject class.
 */

#ifndef NEBULITE_CORE_RENDEROBJECT_HPP
#define NEBULITE_CORE_RENDEROBJECT_HPP

//------------------------------------------
// Includes

// External
#include <SDL.h>	// SDL Renderer is used for some methods to calculate text
#include <SDL_ttf.h>	// Same for ttf

// Nebulite
#include "Constants/KeyNames.hpp"
#include "Core/Texture.hpp"
#include "Data/JSON.hpp"
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Rules {
struct Ruleset;
}

namespace Nebulite::Utility {
class JSON;
}

//------------------------------------------
namespace Nebulite::Core {
/**
 * @class Nebulite::Core::RenderObject
 * @brief Represents a renderable object in the Nebulite engine.
 *        This class encapsulates all data and logic needed to
 *        display, update, and interact with a single object on the screen.
 */
NEBULITE_DOMAIN(RenderObject) {
public:
    //------------------------------------------
    // Special member Functions

    /**
     * @brief Constructs a new RenderObject.
     */
    RenderObject();

    /**
     * @brief Destroys the RenderObject.
     *        Cleans up any resources used by the RenderObject, including
     *        textures and surfaces.
     */
    ~RenderObject() override;

    //------------------------------------------
    // Disable Copying and Moving

    RenderObject(RenderObject const& other) = delete;
    RenderObject(RenderObject&& other) = delete;
    RenderObject& operator=(RenderObject&& other) = delete;

    //------------------------------------------
    // Serializing/Deserializing

    /**
     * @brief Serializes the RenderObject to a JSON string.
     * @return A string representation of the RenderObject's JSON document.
     */
    std::string serialize();

    /**
     * @brief Deserializes the RenderObject from a JSON string.
     * @param serialOrLink The JSON string to deserialize.
     */
    void deserialize(std::string const& serialOrLink);

    /**
     * @brief Gets a pointer to the SDL_Rect describing the destination of the sprite.
     * @return A pointer to the SDL_Rect describing the destination of the sprite.
     */
    SDL_Rect* getDstRect();

    /**
     * @brief Gets a pointer to the SDL_Rect describing the source of the sprite.
     * @return A pointer to the SDL_Rect describing the source of the sprite.
     */
    SDL_Rect* getSrcRect();

    /**
     * @brief Gets a pointer to the SDL_Rect describing the destination of the text.
     * @return A pointer to the SDL_Rect describing the destination of the text.
     */
    SDL_Rect* getTextRect();

    /**
     * @brief Gets the texture of the text.
     * @return A pointer to the SDL_Texture representing the text.
     */
    [[nodiscard]] SDL_Texture* getTextTexture() const;

    //------------------------------------------
    // Update-Oriented functions

    /**
     * @brief Updates the RenderObject.
     *        - updates the domain
     *        - reloads rulesets if needed
     *        - updates local rulesets
     *        - listens to global rulesets
     *        - broadcasts its own global rulesets
     *        - calculates source and destination rects
     * @return Constants::Error indicating success or failure.
     */
    Constants::Error update() override;

    /**
     * @brief Calculates the text texture for the RenderObject.
     * @param renderer The SDL_Renderer to use for rendering.
     * @param font The TTF_Font to use for rendering the text.
     * @param renderPositionX The X position of the renderer used for text offset.
     * @param renderPositionY The Y position of the renderer used for text offset.
     */
    void calculateText(SDL_Renderer* renderer, TTF_Font* font, int const& renderPositionX, int const& renderPositionY);

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
     *        Based on the amount of evaluations and variables in the ruleset.
     * @param onlyInternal If true, only considers internal rulesets. Defaults to true.
     * @return The estimated computational cost.
     */
    uint64_t estimateComputationalCost(bool const& onlyInternal = true);

    //------------------------------------------
    // Management Flags for Renderer-Interaction

    /**
     * @struct flag
     * @brief Flags for managing RenderObject behavior
     */
    struct flag {
        bool deleteFromScene = false; // If true, delete this object from scene on next update
        bool calculateText = false; // If true, calculate text texture on next update
        bool reloadInvokes = false; // If true, reload invokes on next update
    } flag;

    //------------------------------------------
    // Texture related

    /**
     * @brief Links an external SDL_Texture to this domain.
     * @param externalTexture Pointer to the external SDL_Texture.
     */
    void linkExternalTexture(SDL_Texture* externalTexture) {
        baseTexture.linkExternalTexture(externalTexture);
    }

    /**
     * @brief Checks if the texture has been modified.
     * @return true if the texture has been modified, false otherwise.
     */
    [[nodiscard]] bool isTextureStoredLocally() const {
        return baseTexture.isTextureStoredLocally();
    }

    /**
     * @brief Checks if the texture is valid (not null).
     * @return true if the texture is valid, false otherwise.
     */
    [[nodiscard]] bool isTextureValid() const {
        return baseTexture.isTextureValid();
    }

    /**
     * @brief Gets the current SDL_Texture.
     * @return Pointer to the current SDL_Texture.
     */
    [[nodiscard]] SDL_Texture* getSDLTexture() const {
        return baseTexture.getSDLTexture();
    }

    /**
     * @brief Gets the Texture object.
     * @return Pointer to the Texture object.
     */
    Texture* getTexture() {
        return &baseTexture;
    }

private:
    // Size of subscriptions
    size_t subscription_size = 0;

    // Each RenderObject has its own JSON document
    Data::JSON document;

    //------------------------------------------
    // Initialization

    /**
     * @brief Helper function to avoid calls to virtual functions in constructor.
     *        In order for this one to make more sense, it initializes the inherited domains and DomainModules as well.
     */
    void init();

    //------------------------------------------
    // References to JSON

    /**
     * @struct FrequentRefs
     * @brief Holds frequently used references for quick access.
     * @note Another option would be to use static pointers for each method that needs them,
     *       making variables more enclosed to their use case, but that would create duplicate pointers.
     *       So this is a compromise between encapsulation and memory usage.
     *       Later on, we may wish to use static in-function pointers like it's planned for rulesets.
     */
    struct FrequentRefs {
        // Identity
        double* id = nullptr;

        // Position and Size
        double* posX = nullptr;
        double* posY = nullptr;
        double* pixelSizeX = nullptr;
        double* pixelSizeY = nullptr;

        // Spritesheet
        double* isSpritesheet = nullptr;
        double* spritesheetOffsetX = nullptr;
        double* spritesheetOffsetY = nullptr;
        double* spritesheetSizeX = nullptr;
        double* spritesheetSizeY = nullptr;

        // Text
        double* fontSize = nullptr;
        double* textDx = nullptr;
        double* textDy = nullptr;
        double* textColorR = nullptr;
        double* textColorG = nullptr;
        double* textColorB = nullptr;
        double* textColorA = nullptr;
    } refs = {};

    /**
     * @brief Links frequently used references from the JSON document for quick access.
     */
    void linkFrequentRefs();

    //------------------------------------------
    // Texture related

    // Base Texture
    Texture baseTexture;

    // === TO REWORK ===

    // for caching of SDL Positions
    SDL_Rect dstRect = {0, 0, 0, 0}; // destination of sprite
    SDL_Rect srcRect = {0, 0, 0, 0}; // source of sprite from spritesheet
    SDL_Rect textRect = {0, 0, 0, 0}; // destination of text texture

    // Surface and Texture of Text
    SDL_Surface* textSurface; // Surface for the text
    SDL_Texture* textTexture; // Texture for the text

    // ==================

    //------------------------------------------
    // Ruleset Management

    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsGlobal; // Global rulesets, intended for self-other-global interaction
    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsLocal; // Internal rulesets, intended for self-global interaction
};
} // namespace Nebulite::Core
#endif // NEBULITE_CORE_RENDEROBJECT_HPP