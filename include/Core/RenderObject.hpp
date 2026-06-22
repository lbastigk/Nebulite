/**
 * @file RenderObject.hpp
 * @brief Declaration of the RenderObject class.
 */

#ifndef CORE_RENDEROBJECT_HPP
#define CORE_RENDEROBJECT_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Graphics/Drawcall.hpp"
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Rules {
class Ruleset;
} // namespace Nebulite::Interaction::Rules

//------------------------------------------
namespace Nebulite::Core {

//------------------------------------------
// Allow renderer to access document

class RenderObjectDocumentAccessor {
    Data::JsonScope& domainScopeForRenderer;

    friend class Environment; // Required for selecting a RenderObject

public:
    explicit RenderObjectDocumentAccessor(Data::JsonScope& d) : domainScopeForRenderer(d) {}
};

/**
 * @class Nebulite::Core::RenderObject
 * @brief Represents a renderable object in the Nebulite engine.
 *        This class encapsulates all data and logic needed to
 *        display, update, and interact with a single object on the screen.
 */
class RenderObject final : public Interaction::Execution::Domain, public RenderObjectDocumentAccessor {
public:
    //------------------------------------------
    // Special member Functions

    /**
     * @brief Constructs a new RenderObject.
     */
    explicit RenderObject(Utility::IO::Capture& parentCapture);

    /**
     * @brief Destroys the RenderObject.
     *        Cleans up any resources used by the RenderObject, including
     *        textures and surfaces.
     */
    ~RenderObject() override;

    //------------------------------------------
    // Disable Copying and Moving

    RenderObject(RenderObject const&) = delete;
    RenderObject& operator=(RenderObject const&) = delete;
    RenderObject(RenderObject&&) = delete;
    RenderObject& operator=(RenderObject&&) = delete;

    //------------------------------------------
    // Serializing/Deserializing

    /**
     * @brief Serializes the RenderObject to a JSON string.
     * @return A string representation of the RenderObject's JSON document.
     */
    [[nodiscard]] std::string serialize() const ;

    /**
     * @brief Deserializes the RenderObject from a JSON string.
     * @param serialOrLink The JSON string to deserialize.
     */
    void deserialize(std::string const& serialOrLink);

    //------------------------------------------
    // Get position/layer

    struct Position {
        int32_t x;
        int32_t y;
    };

    /**
     * @brief Gets the position of the RenderObject.
     * @return The position of the RenderObject as a Position struct.
     */
    [[nodiscard]] Position getPosition() const ;

    /**
     * @brief Gets the layer of the RenderObject.
     * @return The layer of the RenderObject, or 0 if not set.
     */
    [[nodiscard]] uint8_t getLayer() const ;

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
     * @return Constants::Event indicating success or failure.
     */
    [[nodiscard]] Constants::Event update() override;

    //------------------------------------------
    // Management Flags for Renderer-Interaction

    /**
     * @struct flag
     * @brief Flags for managing RenderObject behavior
     */
    struct flag {
        bool deleteFromScene = false; // If true, delete this object from scene on next update
    } flag;

    //------------------------------------------
    // Drawcalls

    /**
     * @brief Draws the RenderObject at the specified offset.
     * @param renderer The renderer to use
     * @param offsetX The camera offset in the X direction.
     * @param offsetY The camera offset in the Y direction.
     */
    void draw(Renderer const& renderer, float const& offsetX, float const& offsetY);

    /**
     * @brief Re-initialize all drawcalls from document
     */
    void reinitDrawcalls();

    /**
     * @brief Initialize only unknown drawcalls from document
     */
    void initDrawcalls();

    /**
     * @brief Reinitialize a specific drawcall from document
     */
    void reInitDrawcall(std::string const& drawcallName);

private:
    //------------------------------------------
    // Initialization

    /**
     * @brief Helper function to avoid calls to virtual functions in constructor.
     *        In order for this one to make more sense, it initializes the inherited domains and DomainModules as well.
     */
    void init();

    //------------------------------------------
    // Private draw call management

    absl::flat_hash_map<std::string, std::shared_ptr<Graphics::Drawcall>> drawcalls;

    // Reference to members in hashmap sorted by their draw order
    std::vector<std::string> drawcallOrder;

    /**
     * @brief Sorts all drawcalls alphabetically
     */
    void sortDrawcalls();

    /**
     * @brief Updates all drawcalls and their associated texture
     */
    void updateDrawcalls();

    //------------------------------------------
    // References to JSON

    /**
     * @struct FrequentRefs
     * @brief Holds frequently used references for quick access.
     */
    struct FrequentRefs {
        // Position
        double* posX = nullptr;
        double* posY = nullptr;
    } refs = {};

    /**
     * @brief Links frequently used references from the JSON document for quick access.
     */
    void linkFrequentRefs();
};
} // namespace Nebulite::Core
#endif // CORE_RENDEROBJECT_HPP
