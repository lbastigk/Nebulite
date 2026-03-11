/**
 * @file RenderObject.hpp
 * @brief Declaration of the RenderObject class.
 */

#ifndef NEBULITE_CORE_RENDEROBJECT_HPP
#define NEBULITE_CORE_RENDEROBJECT_HPP

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
    explicit RenderObject(Utility::Capture& parentCapture);

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
    [[nodiscard]] std::string serialize() const ;

    /**
     * @brief Deserializes the RenderObject from a JSON string.
     * @param serialOrLink The JSON string to deserialize.
     */
    void deserialize(std::string const& serialOrLink);

    //------------------------------------------
    // Get position

    struct Position {
        int32_t x;
        int32_t y;
    };

    /**
     * @brief Gets the position of the RenderObject.
     * @return The position of the RenderObject as a Position struct.
     */
    [[nodiscard]] Position getPosition() const {
        return {
            static_cast<int32_t>(std::lround(*refs.posX)),
            static_cast<int32_t>(std::lround(*refs.posY))
        };
    }

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
     * @brief Estimates the computational cost of updating the RenderObject.
     *        Based on the amount of evaluations and variables in the ruleset.
     * @param onlyInternal If true, only considers internal rulesets. Defaults to true.
     * @return The estimated computational cost.
     */
    uint64_t estimateComputationalCost(bool const& onlyInternal = true);

    //------------------------------------------
    // Special getters

    [[nodiscard]] uint32_t getId() const {
        // A double can represent all uint32_t values exactly, so this is safe
        return static_cast<uint32_t>(static_cast<int64_t>(*refs.id));
    }

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
    // Draw

    /**
     * @brief Draws the RenderObject at the specified offset.
     * @param offsetX The camera offset in the X direction.
     * @param offsetY The camera offset in the Y direction.
     */
    void draw(float const& offsetX, float const& offsetY) {
        for (auto const& member : drawcallOrder) {
            drawcalls[member]->draw(
                static_cast<float>(*refs.posX) - offsetX,
                static_cast<float>(*refs.posY) - offsetY
            );
        }
    }

private:
    //------------------------------------------
    // Initialization

    /**
     * @brief Helper function to avoid calls to virtual functions in constructor.
     *        In order for this one to make more sense, it initializes the inherited domains and DomainModules as well.
     */
    void init();

    //------------------------------------------
    // Draw calls

    // TODO: expose drawcall init/reinit for a domainmodule to use
    //       This way, we may add new drawcalls at runtime via scripts

    absl::flat_hash_map<std::string, std::shared_ptr<Graphics::Drawcall>> drawcalls;

    // Reference to members in hashmap sorted by their draw order
    std::vector<std::string> drawcallOrder;

    void sortDrawcalls();

    // Re-initialize all drawcalls from document
    void reinitDrawcalls();

    // Initialize only unknown drawcalls from document
    void initDrawcalls();

    // Reinitialize a specific drawcall from document
    void reInitDrawcall(std::string const& drawcallName);

    // Update all drawcalls
    void updateDrawcalls();

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
    } refs = {};

    /**
     * @brief Links frequently used references from the JSON document for quick access.
     */
    void linkFrequentRefs();
};
} // namespace Nebulite::Core
#endif // NEBULITE_CORE_RENDEROBJECT_HPP
