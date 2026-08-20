#ifndef NEBULITE_MODULE_DOMAIN_COMMON_SIMPLEDATA_HPP
#define NEBULITE_MODULE_DOMAIN_COMMON_SIMPLEDATA_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
/**
 * @class Nebulite::Module::Domain::Common::SimpleData
 * @brief DomainModule for simple data operations on domain class Nebulite::Data::Json
 */
class SimpleData final : public Base::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override ;
    void reinit() override {} // No reinitialization needed, SimpleData is stateless

    //------------------------------------------
    // Available Functions

    [[nodiscard]] static Constants::Event set(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr setName = "set";
    static auto constexpr setDesc = "Set a key to a string value in the JSON document.\n"
        "\n"
        "Usage: set <key> [value]\n"
        "If no value is provided, the value will be set to an empty string.\n";

    [[nodiscard]] static Constants::Event assign(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr assignName = "assign";
    static auto constexpr assignDesc = "Assign a key to a value in the JSON document (self) or the global context (global)\n"
        "\n"
        "Usage: assign <context>.<key> <assignment-operator> <expression>\n"
        "\n"
        "Example: 'assign global:rngCurrentValuesCopy = {global:random}"
        "Supports complex types like arrays or objects.\n"
        "The assignment has full access to the entire global scope here, so be cautious when using this function to overwrite global values.\n"
        "Use json set instead, if you only wish to modify values in the context self with no special operators.\n";

    [[nodiscard]] static Constants::Event move(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr moveName = "move";
    static auto constexpr moveDesc = "Move data from one key to another.\n"
        "\n"
        "Usage: move <source_key> <destination_key>\n";

    [[nodiscard]] static Constants::Event copy(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr copyName = "copy";
    static auto constexpr copyDesc = "Copy data from one key to another.\n"
        "\n"
        "Usage: copy <source_key> <destination_key>\n";

    [[nodiscard]] static Constants::Event keyDelete(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr keyDeleteName = "keyDelete";
    static auto constexpr keyDeleteDesc = "Delete a key from the JSON document.\n"
        "\n"
        "Usage: keyDelete <key>\n";

    [[nodiscard]] static Constants::Event ensureArray(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr ensureArrayName = "ensure-array";
    static auto constexpr ensureArrayDesc = "Ensure that a key is an array, converting a value to an array if necessary.\n"
        "\n"
        "Usage: ensure-array <key>\n";

    [[nodiscard]] static Constants::Event pushBack(std::span<std::string_view const> args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr pushBackName = "push-back";
    static auto constexpr pushBackDesc = "Push a value to the back of an array.\n"
        "\n"
        "Usage: push-back <key> <value>\n";

    [[nodiscard]] static Constants::Event popBack(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr popBackName = "pop-back";
    static auto constexpr popBackDesc = "Pop a value from the back of an array.\n"
        "\n"
        "Usage: pop-back <key>\n";

    [[nodiscard]] static Constants::Event pushFront(std::span<std::string_view const> args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr pushFrontName = "push-front";
    static auto constexpr pushFrontDesc = "Push a value to the front of an array.\n"
        "\n"
        "Usage: push-front <key> <value>\n";

    [[nodiscard]] static Constants::Event popFront(std::span<std::string_view const> args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr popFrontName = "pop-front";
    static auto constexpr popFrontDesc = "Pop a value from the front of an array.\n"
        "\n"
        "Usage: pop-front <key>\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit SimpleData(ConstructorParams const& params) : DomainModule(params) {
        // Bind functions specific to complex data handling
        bindFunction(&SimpleData::set, setName, setDesc);
        bindFunction(&SimpleData::assign, assignName, assignDesc);

        // Internal move/copy
        bindFunction(&SimpleData::move, moveName, moveDesc);
        bindFunction(&SimpleData::copy, copyName, copyDesc);

        // Internal key deletion
        bindFunction(&SimpleData::keyDelete, keyDeleteName, keyDeleteDesc);

        // Array manipulation
        bindFunction(&SimpleData::pushBack, pushBackName, pushBackDesc);
        bindFunction(&SimpleData::popBack, popBackName, popBackDesc);
        bindFunction(&SimpleData::pushFront, pushFrontName, pushFrontDesc);
        bindFunction(&SimpleData::popFront, popFrontName, popFrontDesc);
        bindFunction(&SimpleData::ensureArray, ensureArrayName, ensureArrayDesc);
    }
};
} // namespace Nebulite::Module::Domain::Common
#endif // NEBULITE_MODULE_DOMAIN_COMMON_SIMPLEDATA_HPP
