#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAIN_MODULE_BASE_HPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAIN_MODULE_BASE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Interaction/Execution/DomainTree.hpp"
#include "Interaction/Execution/FuncTree.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Interaction::Execution {
/**
 * @class Nebulite::Interaction::Execution::DomainModuleBase
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
        std::shared_ptr<DomainTree> funcTreePtr,
        Data::JsonScope& w,
        Data::JsonScope const& s
    );

    virtual ~DomainModuleBase();

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
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    template <typename Obj, typename Func, typename FuncTreeType>
    static void bindFunctionStatic(
        FuncTreeType* tree,
        Obj* objectPtr,
        Func functionPtr,
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    //------------------------------------------
    // Non-static Binding Functions

    template <typename R, typename C, typename... Ps>
    void bindFunction(
        R (C::*functionPtr)(Ps...),
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    template <typename R, typename C, typename... Ps>
    void bindFunction(
        R (C::*functionPtr)(Ps...) const,
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    template <typename Func>
    void bindFunction(
        Func functionPtr,
        std::string_view const& name,
        std::string_view const& helpDescription
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
    void bindCategory(std::string_view const& name, std::string_view const& helpDescription) const {
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
    void bindVariable(bool* variablePtr, std::string_view const& name, std::string_view const& helpDescription) const {
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

    enum class RoutineUpdateMode {
        BEFORE_UPDATE_HOOK, ///< Update routines before calling the update hook
        AFTER_UPDATE_HOOK   ///< Update routines after calling the update hook
    };

protected:
    //------------------------------------------
    // Routine management

    /**
     * @brief Adds a routine to the DomainModule's routine list. Automatically updated on each update-call.
     * @param routine The routine to add
     * @param mode Update routine type, either before or after the update hook.
     */
    void addRoutine(Utility::Coordination::TimedRoutine const& routine, RoutineUpdateMode const& mode) {
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
    std::shared_ptr<DomainTree> funcTree;
};
} // namespace Nebulite::Interaction::Execution
#include "Interaction/Execution/DomainModuleBase.tpp"
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAIN_MODULE_BASE_HPP
