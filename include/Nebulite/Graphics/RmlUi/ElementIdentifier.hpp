#ifndef NEBULITE_GRAPHICS_RMLUI_ELEMENTIDENTIFIER_HPP
#define NEBULITE_GRAPHICS_RMLUI_ELEMENTIDENTIFIER_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <string>
#include <utility>

// External
#include <RmlUi/Core/Element.h>

//------------------------------------------
namespace Nebulite::Graphics::RmlUi {

/**
 * @brief Handles unique element identification for context management.
 * @details Uses the Rml Attribute functionality to set and retrieve unique identifications
 */
class ElementIdentifier {
    static auto constexpr identifierAttribute = "element-identifier";

    std::size_t id; // This elements id

    static std::size_t& count(); // Get the current id count as reference

    static std::size_t idRoll();

    /**
     * @brief Construct an identifier with a known id.
     * @details This should only be used if you are sure the id is not already assigned to another element,
     *          e.g. for the reflection module to assign a list of owned, pre-allocated, identifiers to newly generated elements
     * @param knownId The id to use
     */
    explicit ElementIdentifier(std::size_t const knownId) : id(knownId) {}

public:
    /**
     * @brief Get the current count of assigned identifiers.
     * @return The count of assigned identifiers.
     */
    static std::size_t getCount();

    /**
     * @brief Forces an Element to have a certain identifier id
     * @details This should be used with caution and only used with ids that are known to not be registered elsewhere
     * @param element The element to manipulate
     * @param identifier The identifier to set
     */
    static void forceElementIdentifier(Rml::Element* element, ElementIdentifier const& identifier);

    /**
     * @brief Remove the identifier attribute from an element, effectively unregistering it.
     *        The freed id cannot be reused.
     * @param element The element to manipulate
     */
    static void removeElementIdentifier(Rml::Element* element);

    /**
     * @brief Checks if an element has an identifier.
     * @param element The element to check
     * @return True if it has an identifier, false otherwise.
     */
    static bool hasElementIdentifier(Rml::Element const* element);

    /**
     * @brief Construct an identifier for/from a given Rml::Element.
     * @details If the element already has an identifier, it will be used.
     *          Otherwise, a new identifier will be generated and assigned to the element.
     * @param e The element to construct the identifier for
     */
    explicit ElementIdentifier(Rml::Element* e);

    /**
     * @brief Generates a new ElementIdentifier with a unique id.
     * @return The new ElementIdentifier
     */
    static ElementIdentifier newIdentifier();

    /**
     * @brief Turns the ElementIdentifier into a string representation of its id.
     * @return The string representation of the id
     */
    [[nodiscard]] explicit operator std::string() const {
        return std::to_string(id);
    }

    bool operator==(ElementIdentifier const& other) const {
        return id == other.id;
    }

    template <typename H>
    friend H AbslHashValue(H h, ElementIdentifier const& toHash) { // NOLINT
        return H::combine(std::move(h), toHash.id);
    }
};

} // namespace Nebulite::Graphics::RmlUi
#endif // NEBULITE_GRAPHICS_RMLUI_ELEMENTIDENTIFIER_HPP
