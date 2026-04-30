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
namespace Nebulite::Module::Domain::Common {
NEBULITE_DOMAINMODULE(Nebulite::Interaction::Execution::Domain, General) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] static Constants::Event eval(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope);
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
        "eval spawn ./Resources/RenderObjects/{global:ToSpawn}.json\n"
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

    // TODO: Use recombineArgs to use the full line of code instead of args[1]
    [[nodiscard]] static Constants::Event func_assert(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
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

    [[nodiscard]] static Constants::Event func_for(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope);
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

    [[nodiscard]] static Constants::Event func_if(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr func_if_name = "if";
    static auto constexpr func_if_desc = "Executes a block of code if a condition is true.\n"
        "\n"
        "Usage: if <condition> <functioncall>\n"
        "\n"
        "It is recommended to use the if-then syntax to avoid whitespace issues:\n"
        "\n"
        "Usage: if <condition> then <functioncall>\n"
        "\n"
        "Example:\n"
        "if $({global:settings.someFile|strCompare equals ./Resources/myFile.txt}) then echo Condition is true!\n";

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

    // [FORWARD/REPARSE]

    [[nodiscard]] static Constants::Event forwardToOther(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr forwardToOther_name = "forward other";
    static auto constexpr forwardToOther_desc = "Forwards the arguments to the other context without modifying context.\n"
        "Same as a json ruleset functioncall in the other context.\n"
        "Usage: forward other <functioncall>\n"
        "\n"
        "This command takes the arguments after 'forward other' and executes them as a command in the other context.\n"
        "This is useful for executing commands that are only available in the other context.\n";

    [[nodiscard]] static Constants::Event forwardToGlobal(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr forwardToGlobal_name = "forward global";
    static auto constexpr forwardToGlobal_desc = "Forwards the arguments to the global context without modifying context.\n"
        "Same as a json ruleset functioncall in the global context.\n"
        "Usage: forward global <functioncall>\n"
        "\n"
        "This command takes the arguments after 'forward global' and executes them as a command in the global context.\n"
        "This is useful for executing commands that are only available in the global context or for modifying global variables.\n";

    [[nodiscard]] static Constants::Event reparseInOther(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr reparseInOther_name = "reparse other";
    static auto constexpr reparseInOther_desc = "Forwards the arguments to the other context, switching the contexts self and other.\n"
        "Usage: forward other <functioncall>\n"
        "\n"
        "This command takes the arguments after 'forward other' and executes them as a command in the other context.\n"
        "This is useful for modifying variables in the other context.\n";

    [[nodiscard]] static Constants::Event reparseInGlobal(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr reparseInGlobal_name = "reparse global";
    static auto constexpr reparseInGlobal_desc = "Forwards the arguments to the global context, replacing all context with global.\n"
        "Usage: forward global <functioncall>\n"
        "\n"
        "This command takes the arguments after 'forward global' and executes them as a command in the global context.\n"
        "This is useful for modifying global variables.\n";

    // [IMGUI]

    [[nodiscard]] Constants::Event imguiView(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr imguiView_name = "imgui-view";
    static auto constexpr imguiView_desc = "Creates an ImGui view of the domain.\n"
       "\n"
       "Usage: imgui-view <on/off>\n";
    static auto constexpr imguiView_Enable = "imgui-view on";
    static auto constexpr imguiView_Disable = "imgui-view off";

    //------------------------------------------
    // Categories

    static auto constexpr forwardName = "forward";
    static auto constexpr forwardDesc = "Commands for forwarding function calls to other contexts (other or global).";

    static auto constexpr reparseName = "reparse";
    static auto constexpr reparseDesc = "Commands for forwarding function calls to other contexts (other or global) while switching context.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Interaction::Execution::Domain, General) {
        // Binding

        // Base functions
        bindFunction(&General::eval, eval_name, eval_desc);
        bindFunction(&General::func_for, func_for_name, func_for_desc);
        bindFunction(&General::func_if, func_if_name, func_if_desc);
        bindFunction(&General::echo, echo_name, echo_desc);
        bindFunction(&General::func_assert, assert_name, assert_desc);
        bindFunction(&General::nop, nop_name, nop_desc);

        // Forwarding
        bindCategory(forwardName, forwardDesc);
        bindFunction(&General::forwardToOther, forwardToOther_name, forwardToOther_desc);
        bindFunction(&General::forwardToGlobal, forwardToGlobal_name, forwardToGlobal_desc);
        bindCategory(reparseName, reparseDesc);
        bindFunction(&General::reparseInOther, reparseInOther_name, reparseInOther_desc);
        bindFunction(&General::reparseInGlobal, reparseInGlobal_name, reparseInGlobal_desc);

        // Imgui view
        bindFunction(&General::imguiView, imguiView_name, imguiView_desc);
    }

private:
    bool imguiViewEnabled = false;

    // Store addresses of who called the last imgui-view command, so we can pass them to the imgui view later on
    // We assume the lifetime of both self and global exceeds the usage here
    struct LastContext {
        Interaction::Execution::Domain* self = nullptr;
        Interaction::Execution::Domain* global = nullptr;
        Data::JsonScope* selfScope = nullptr;
        Data::JsonScope* globalScope = nullptr;

        [[nodiscard]] bool valid() const {
            return self && global && selfScope && globalScope;
        }
    } lastContext;
};
} // namespace Nebulite::Module::Domain::Common
#endif // NEBULITE_DOMAIN_MODULE_COMMON_GENERAL_HPP
