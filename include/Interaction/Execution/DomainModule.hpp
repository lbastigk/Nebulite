/**
 * @file DomainModule.hpp
 * @brief This file defines the DomainModule class, which extends the functionality of the FuncTree
 *        class to support category-based function bindings.
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "ScopeAccessor.hpp"
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/FuncTree.hpp"
#include "Data/Document/JsonScopeBase.hpp"

//------------------------------------------
// Macro for DomainModule definition

// NOLINTNEXTLINE
#define NEBULITE_DOMAINMODULE(DomainName,DomainModuleName) \
    class DomainModuleName final : public Nebulite::Interaction::Execution::DomainModule<DomainName>

// NOLINTNEXTLINE
#define NEBULITE_DOMAINMODULE_CONSTRUCTOR(DomainName,DomainModuleName) \
    explicit DomainModuleName( \
        std::string const& name, DomainName& domainReference, \
        std::shared_ptr<Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error, Nebulite::Interaction::Execution::DomainBase&, Nebulite::Data::JsonScopeBase&>> funcTreePtr, \
        Data::JsonScopeBase& w, \
        Data::JsonScopeBase const& s \
    ) \
    : DomainModule(name, domainReference, std::move(funcTreePtr), w, s)

// Common macro to create a floating DomainModule with proper linkage
// Floating DomainModules are handled separately from regular DomainModules
// They are not updated automatically via the domains updateModules function.
// However, they offer the same "separation of concerns" as regular DomainModules
// without the additional overhead if we were to turn them into full Domains.
// Useful for small "runners" with neatly separated functionality, that need the ability to be called
// separately.
// NOLINTNEXTLINE
#define NEBULITE_FLOATING_DOMAINMODULE(DomainModule, DomainModuleName, Document, Workspace, Settings) \
    std::make_unique<DomainModule>( \
        DomainModuleName, \
        *this, \
        getFuncTree(), \
        Document.domainScope.shareScopeBase(Workspace), \
        Settings \
    )

#define BIND_FUNCTION(func, name,desc) \
static_assert(::Nebulite::Constants::Assert::endsWithNewline(desc), "Function description must end with a newline character."); \
    bindFunction(func, name, desc)

//------------------------------------------
namespace Nebulite::Interaction::Execution {
/**
 * @class Nebulite::Interaction::Execution::DomainModule
 * @brief Wrapper class for binding functions to a specific category in the FuncTree and adding separate update routines.
 * @details Allows for cleaner separation of object files for different categories
 *          and reduces boilerplate code when attaching functions to the FuncTree.
 */
template <typename DomainType>
class DomainModule : public DomainModuleBase {
public:
    /**
     * @brief Constructor for the DomainModule base class.
     * @details The constructor initializes the DomainModule with a reference to the domain and
     *          the FuncTree.
     * @param name Name of the DomainModule, useful for debugging and logging.
     * @param domainReference Reference to the Domain instance this module is associated with.
     * @param funcTreePtr Shared pointer to the FuncTree for binding functions and variables.
     * @param scope JsonScopeBase reference for this module to use as workspace.
     * @param settings Const JsonScopeBase reference for settings.
     */
    DomainModule(
        std::string name,
        DomainType& domainReference,
        std::shared_ptr<FuncTree<Constants::Error, DomainBase&, Data::JsonScopeBase&>> funcTreePtr,
        Data::JsonScopeBase& scope,
        Data::JsonScopeBase const& settings
    );

    /**
     * @brief Virtual destructor for DomainModule.
     */
    virtual ~DomainModule();

    // Prevent copying
    DomainModule(DomainModule const&) = delete;

    // Prevent assignment
    DomainModule& operator=(DomainModule const&) = delete;

protected:
    //------------------------------------------
    // Name of Module

    /**
     * @brief Name of the DomainModule, useful for debugging and logging.
     */
    std::string moduleName;

    //------------------------------------------
    // Linkages

    /**
     * @brief Workspace of the DomainModule
     */
    DomainType& domain;

    //------------------------------------------
    // Access token for Global Space access

    static ScopeAccessor::BaseAccessToken getDomainModuleAccessToken(DomainModule const& derivedModule) {
        return ScopeAccessor::DomainModuleToken(derivedModule);
    }
};
} // namespace Nebulite::Interaction::Execution
#include "Interaction/Execution/DomainModule.tpp"
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP
