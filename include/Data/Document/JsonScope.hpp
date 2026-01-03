/**
 * @file JsonScope.hpp
 * @brief This file contains the definition of the JsonScope and JsonScobeBase class, which provides a scoped interface
 *        for accessing and modifying JSON documents within the Nebulite engine.
 */

#ifndef NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP
#define NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP

//------------------------------------------
// Includes

// Standard library

// Nebulite
#include "Data/Document/JsonScopeBase.hpp"
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
namespace Nebulite::Data {

/**
 * @brief Domain-ized JsonScope class
 * @details Inherits from Interaction::Execution::Domain to integrate with the Nebulite interaction system.
 *          Also inherits from JsonScopeBase to provide scoped JSON document access.
 */
class JsonScope final : public Interaction::Execution::Domain<JsonScope>, public JsonScopeBase {
public:
    //------------------------------------------
    // Constructors

    // Constructing a JsonScopeBase from a JSON document and a prefix
    JsonScope(JSON& doc, std::string const& prefix, std::string const& name = "Unnamed JsonScope");

    // Constructing a JsonScopeBase from another JsonScopeBase and a sub-prefix
    JsonScope(JsonScope const& other, std::string const& prefix, std::string const& name = "Unnamed JsonScope");

    // Default constructor, we create a self-owned empty JSON document
    explicit JsonScope(std::string const& name = "Unnamed JsonScope");

    //------------------------------------------
    // Special member functions

    // TODO: No copy or move? Current design causes infinite recursion in some cases!!!
    JsonScope(JsonScope const& other) = delete;
    JsonScope(JsonScope&& other) noexcept = delete;
    JsonScope& operator=(JsonScope const& other) = delete;
    JsonScope& operator=(JsonScope&& other) noexcept = delete;

    ~JsonScope() override = default;

    //------------------------------------------
    // Proper scope sharing

    // Proper scope sharing with nested unscoped key generation
    [[nodiscard]] JsonScope& shareScope(ScopedKey const& key) const ;

    //------------------------------------------
    // Domain related stuff

    Constants::Error update() override {
        updateModules();
        return Constants::ErrorTable::NONE();
    }

    //------------------------------------------
    // Overwrite deserialization to add token parsing

    void deserialize(std::string const& serialOrLink) override ;
};

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP
