#ifndef NEBULITE_MODULE_BASE_DOMAINMODULEBASE_HPP
#define NEBULITE_MODULE_BASE_DOMAINMODULEBASE_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Interaction/Execution/DomainTree.hpp"
#include "Nebulite/Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Base {
/**
 * @class Nebulite::Module::Base::DomainModuleBase
 * @brief Base class for DomainModule to handle common functionality.
 * @details This class provides the foundational functionality for binding functions
 *          and variables to the FuncTree, allowing derived classes to focus on
 *          domain-specific implementations.
 */
class DomainModuleBase {
public:
    /**
     * @brief Constructor for the DomainModule base class.
     * @param funcTreePtr Shared pointer to the FuncTree for binding functions and variables.
     * @param w Reference to a JsonScope document for this module to use as workspace.
     * @param s Reference to a JsonScope document for settings.
     * @details The constructor initializes the DomainModuleBase with
     *          the FuncTree pointer for binding functions and variables.
     */
    explicit DomainModuleBase(
        std::shared_ptr<Interaction::Execution::DomainTree> funcTreePtr,
        Data::JsonScope& w,
        Data::JsonScope const& s
    );

    virtual ~DomainModuleBase();

    DomainModuleBase(DomainModuleBase const&) = delete;
    DomainModuleBase& operator=(DomainModuleBase const&) = delete;
    DomainModuleBase(DomainModuleBase&&) = delete;
    DomainModuleBase& operator=(DomainModuleBase&&) = delete;

    //------------------------------------------
    // Virtual functions for derived classes to implement

    /**
     * @brief Updates all routines and calls the update hook
     */
    [[nodiscard]] Constants::Event update() {
        for (auto& routine : routinesBeforeHook) {
            routine.update();
        }
        auto const event = updateHook();
        for (auto& routine : routinesAfterHook) {
            routine.update();
        }
        return event;
    }

    /**
     * @brief Virtual update function to be Overwritten by derived classes.
     */
    [[nodiscard]] virtual Constants::Event updateHook();

    /**
     * @brief Virtual re-initialization function to be Overwritten by derived classes.
     */
    virtual void reinit();

    //------------------------------------------
    // Static Binding Functions

    template <typename Func,typename FuncTreeType>
    static void bindFunctionStatic(
        FuncTreeType* tree,
        Func functionPtr,
        std::string_view name,
        std::string_view helpDescription
    );

    template <typename Obj, typename Func, typename FuncTreeType>
    static void bindFunctionStatic(
        FuncTreeType* tree,
        Obj* objectPtr,
        Func functionPtr,
        std::string_view name,
        std::string_view helpDescription
    );

    //------------------------------------------
    // Non-static Binding Functions

    template <typename R, typename C, typename... Ps>
    void bindFunction(
        R (C::*functionPtr)(Ps...),
        std::string_view name,
        std::string_view helpDescription
    );

    template <typename R, typename C, typename... Ps>
    void bindFunction(
        R (C::*functionPtr)(Ps...) const,
        std::string_view name,
        std::string_view helpDescription
    );

    template <typename Func>
    void bindFunction(
        Func functionPtr,
        std::string_view name,
        std::string_view helpDescription
    );

    //------------------------------------------
    // Category and Variable Binding

    /**
     * @brief Binds a category to the FuncTree.
     * @details A category acts as a "function bundler" to the main tree.
     * @param name Name of the category
     * @param helpDescription Description of the category, shown in the help command.
     *                        First line is shown in the general help, full description in detailed help
     * @throws std::runtime_error if the category already exists.
     */
    void bindCategory(std::string_view const name, std::string_view const helpDescription) const {
        funcTree->bindCategory(name, helpDescription);
    }

    /**
     * @brief Binds a variable to the command tree.
     * @details Once bound, it can be set via command line arguments: --varName=value (Must be before the function name!)
     *          A simple argument of '--varName' will set the value to "true"
     * @param variablePtr Pointer to the variable to bind.
     * @param name Name of the variable in the command tree.
     * @param helpDescription Description of the variable, shown in the help command.
     */
    void bindVariable(bool* variablePtr, std::string_view const name, std::string_view const helpDescription) const {
        // Bind a variable to the FuncTree
        funcTree->bindVariable(variablePtr, name, helpDescription);
    }

    /**
     * @brief Reference to the JsonScope document.
     * @details This allows derived DomainModules to access and manipulate
     *          the JSON document as needed.
     */
    Data::JsonScope& moduleScope;

    /**
     * @brief Reference to the JsonScope document for settings.
     * @details This allows derived DomainModules to access settings for initial configuration.
     */
    Data::JsonScope const& settingsScope;

    enum class RoutineUpdateMode : std::uint8_t {
        BEFORE_UPDATE_HOOK, // Update routines before calling the update hook
        AFTER_UPDATE_HOOK   // Update routines after calling the update hook
    };

protected:
    //------------------------------------------
    // Routine management

    /**
     * @brief Adds a routine to the DomainModule's routine list. Automatically updated on each update-call.
     * @param routine The routine to add
     * @param mode Update routine type, either before or after the update hook.
     */
    void addRoutine(Utility::Coordination::TimedRoutine const& routine, RoutineUpdateMode const mode) {
        if (mode == RoutineUpdateMode::BEFORE_UPDATE_HOOK) {
            routinesBeforeHook.push_back(routine);
        }
        else if (mode == RoutineUpdateMode::AFTER_UPDATE_HOOK) {
            routinesAfterHook.push_back(routine);
        }
        else {
            std::unreachable();
        }
    }

private:
    /**
     * @brief Vector of update routines for this DomainModule, which will be called every update cycle by the Domain.
     */
    std::vector<Utility::Coordination::TimedRoutine> routinesBeforeHook;
    std::vector<Utility::Coordination::TimedRoutine> routinesAfterHook;

    /**
     * @brief Pointer to the internal FuncTree for binding functions and variables.
     */
    std::shared_ptr<Interaction::Execution::DomainTree> funcTree;
};
} // namespace Nebulite::Module::Base
#include "DomainModuleBase.tpp" // NOLINT
#endif // NEBULITE_MODULE_BASE_DOMAINMODULEBASE_HPP
