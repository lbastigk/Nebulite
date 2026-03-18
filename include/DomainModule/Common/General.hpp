/**
 * @file Debug.hpp
 * @brief Implementation of force and clearForce functions for forcing JSON variable values.
 */

#ifndef NEBULITE_DOMAIN_MODULE_COMMON_GENERAL_HPP
#define NEBULITE_DOMAIN_MODULE_COMMON_GENERAL_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations

//------------------------------------------
namespace Nebulite::DomainModule::Common {
NEBULITE_DOMAINMODULE(Nebulite::Interaction::Execution::Domain, General) {
public:
    [[nodiscard]] Constants::Event update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event imguiView(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr imguiView_name = "imgui-view";
    static auto constexpr imguiView_desc = "Creates an ImGui view of the domain.\n"
       "\n"
       "Usage: imgui-view <on/off>\n";
    static auto constexpr imguiView_Enable = "imgui-view on";
    static auto constexpr imguiView_Disable = "imgui-view off";

    [[nodiscard]] static Constants::Event eval(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr eval_name = "eval";
    static auto constexpr eval_desc = "Evaluates an expression string and executes it.\n"
        "Every argument after eval is concatenated with a whitespace to form the expression to be evaluated and then reparsed.\n"
        "\n"
        "Usage: eval <expression>\n"
        "\n"
        "Examples:\n"
        "\n"
        "eval echo $(1+1)\n"
        "outputs: 2.000000\n"
        "First, eval evaluates every argument, then concatenates them with a whitespace,\n"
        "and finally executes the resulting string as a command.\n"
        "The string 'echo $(1+1)' is evaluated to \"echo 2.000000\", which is then executed.\n"
        "\n"
        "eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json\n"
        "This evaluates to 'spawn ./Resources/RenderObjects/NAME.json',\n"
        "where NAME is the current value of the global variable ToSpawn\n";

    [[nodiscard]] static Constants::Event nop(std::span<std::string const> const& args);
    static auto constexpr nop_name = "nop";
    static auto constexpr nop_desc = "No operation. Does nothing.\n"
        "\n"
        "Usage: nop <blind arguments>\n"
        "\n"
        "Useful for testing or as a placeholder in scripts where no action is required,\n"
        "but a command is syntactically necessary.\n";

    [[nodiscard]] static Constants::Event func_assert(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr assert_name = "assert";
    static auto constexpr assert_desc = "Asserts a condition and throws a custom error if false.\n"
        "\n"
        "Usage: assert <condition>\n"
        "\n"
        "It is recommended to wrap the condition in quotes to prevent parsing issues.\n"
        "\n"
        "Example:\n"
        "assert '$(eq(1+1,2))'    // No error\n"
        "assert '$(eq(1+1,3))'    // Critical Error: A custom assertion failed.\n"
        "Assertion failed: $(eq(1+1,3)) is not true.\n";

    [[nodiscard]] static Constants::Event func_for(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr func_for_name = "for";
    static auto constexpr func_for_desc = "Executes a for-loop with a function call.\n"
        "\n"
        "Usage: for <var> <start> <end> <functioncall>\n"
        "\n"
        "Example:\n"
        "for i 1 5 echo Iteration {i}\n"
        "This will output:\n"
        "    Iteration 1\n"
        "    Iteration 2\n"
        "    Iteration 3\n"
        "    Iteration 4\n"
        "    Iteration 5\n"
        "\n"
        "This is useful for:\n"
        "- Repeating actions a specific number of times.\n"
        "- Iterating over a range of values.\n"
        "- Creating complex control flows in scripts.\n";

    [[nodiscard]] static Constants::Event func_if(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr func_if_name = "if";
    static auto constexpr func_if_desc = "Executes a block of code if a condition is true.\n"
        "\n"
        "Usage: if <condition> <functioncall>\n"
        "\n"
        "It is recommended to wrap the condition in quotes to prevent parsing issues.\n"
        "\n"
        "Example:\n"
        "if '$(eq(1+1,2))' echo Condition is true!\n";

    [[nodiscard]] Constants::Event echo(std::span<std::string const> const& args) const ;
    static auto constexpr echo_name = "echo";
    static auto constexpr echo_desc = "Echoes all arguments as string to the standard output.\n"
        "\n"
        "Usage: echo <string>\n"
        "\n"
        "This command concatenates all arguments with a whitespace and outputs them to the standard output (cout).\n"
        "Example:\n"
        "./bin/Nebulite echo Hello World!\n"
        "Outputs:\n"
        "Hello World!\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Interaction::Execution::Domain, General) {
        // Binding
        BIND_FUNCTION(&General::imguiView, imguiView_name, imguiView_desc);
        BIND_FUNCTION(&General::eval, eval_name, eval_desc);

        BIND_FUNCTION(&General::func_for, func_for_name, func_for_desc);
        BIND_FUNCTION(&General::func_if, func_if_name, func_if_desc);
        BIND_FUNCTION(&General::echo, echo_name, echo_desc);
        BIND_FUNCTION(&General::func_assert, assert_name, assert_desc);
        BIND_FUNCTION(&General::nop, nop_name, nop_desc);
    }

private:
    bool imguiViewEnabled = false;

    // Store addresses of who called the last imgui-view command, so we can pass them to the imgui view later on
    // Due to design constraints, a Common DomainModule cannot have a scope...
    Data::JsonScope* lastImguiCallerScope = nullptr;
};
} // namespace Nebulite::DomainModule::Common
#endif // NEBULITE_DOMAIN_MODULE_COMMON_GENERAL_HPP
