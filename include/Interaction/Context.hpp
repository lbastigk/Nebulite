/**
 * @file Context.hpp
 * @brief Defines context structures for interaction of rulesets and functions
 *        within the Nebulite engine.
 */
#ifndef NEBULITE_INTERACTION_CONTEXT_HPP
#define NEBULITE_INTERACTION_CONTEXT_HPP

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScopeBase;
} // namespace Nebulite::Data

namespace Nebulite::Interaction::Execution {
class DomainBase;
template <typename DomainType> class Domain;
} // namespace Nebulite::Interaction::Execution

namespace Nebulite::Interaction {

// ContextScopeBase -> JSON scope access with JsonScopeBase references
//                     Access to the scoped data only
// ContextBase      -> Base domain access with DomainBase references
//                     Additional Access to parsing, functree inheriting, update routines.
// ContextFull      -> Full domain access with specific domain types
//                     Additional Access to class of the domain,
//                     module initialization and updating.

/**
 * @struct ContextScopeBase
 * @brief Reduced context structure using JsonScopeBase references.
 *        Useful for functions that only need access to the JSON scopes of the domains.
 * @details Third layer of abstraction, only JSON scope access available.
 */
class ContextScopeBase {
public:
    Data::JsonScopeBase& self;
    Data::JsonScopeBase& other;
    Data::JsonScopeBase& global;
    // TODO: Parent context?
};

/**
 * @struct ContextBase
 * @brief Context structure passed to static ruleset functions and other interaction functions.
 *        Contains references to the 'self', 'other', and 'global' domains, with base domain types.
 * @details Second layer of abstraction, only base domain functionality available.
 */
class ContextBase {
public:
    Execution::DomainBase& self;
    Execution::DomainBase& other;
    Execution::DomainBase& global;
    // TODO: Parent context?
private:
    [[nodiscard]] ContextScopeBase demote() const ;
};

/**
 * @class ContextFull
 * @tparam SelfDomainType The domain type for context 'self'.
 * @tparam OtherDomainType The domain type for context 'other'.
 * @tparam GlobalDomainType The domain type for context 'global'.
 * @brief Full context structure containing references to the 'self', 'other', and 'global' domains,
 *        with their specific domain types.
 * @details First layer of abstraction, full domain with all functionality available.
 */
template <typename SelfDomainType, typename OtherDomainType, typename GlobalDomainType>
class ContextFull {
public:
    Execution::Domain<SelfDomainType>& self;
    Execution::Domain<OtherDomainType>& other;
    Execution::Domain<GlobalDomainType>& global;
    // TODO: Parent context?
private:
    [[nodiscard]] ContextBase demote() const {
        return ContextBase{
            static_cast<Execution::DomainBase&>(self),
            static_cast<Execution::DomainBase&>(other),
            static_cast<Execution::DomainBase&>(global)
        };
    }
};

} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_CONTEXT_HPP
