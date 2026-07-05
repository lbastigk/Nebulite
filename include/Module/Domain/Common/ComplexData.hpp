#ifndef MODULE_DOMAIN_COMMON_COMPLEXDATA_HPP
#define MODULE_DOMAIN_COMMON_COMPLEXDATA_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

// Nebulite
#include "Constants/Event.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
/**
 * @class Nebulite::Module::Domain::Common::ComplexData
 * @brief DomainModule for complex data operations on domain class Nebulite::Data::JSON
 */
class ComplexData final : public Base::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    /**
     * @todo Not implemented yet
     */
    [[nodiscard]] Constants::Event querySet();
    static auto constexpr querySet_name = "query set";
    static auto constexpr querySet_desc = "Sets a key from a SQL query result.\n"
        "Not implemented yet.\n";

    [[nodiscard]] static Constants::Event jsonSet(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr jsonSet_name = "json set";
    static auto constexpr jsonSet_desc = "Sets a key from a given expression evaluated as JSON, allowing for complex objects to be set.\n"
        "Usage: json set <key> <expression>\n"
        "\n"
        "Examples:\n"
        "json set namesStartingWithF {global:names|filterGlob F*}\n"
        "json set userInfo {global:users|filterRegex {!^user[0-9]+$}}\n"
        "json set readOnlyDoc {./Resources/sample.json:key1.key2}\n"
        "json set sizeCopy {self:size}\n"
        "Same as the function 'assign', but does not allow assigning values in the Global context, and is only for set ('=') operations.\n";

    [[nodiscard]] static Constants::Event evaluateMember(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr evaluateMember_name = "json evaluate member";
    static auto constexpr evaluateMember_desc = "If the member is a string or number, treats it as an expression and evaluates it as JSON, setting the member to the result.\n"
        "If the member is an array or object, it will do nothing.\n"
        "Usage: json evaluate member <key>\n"
        "\n"
        "Examples:\n"
        "evaluate member myExpression\n"
        "If the member myExpression is a string, for example \"{global:names|filterGlob F*}\", myExpression will be set to an array of names starting with F from the global scope.\n";

    [[nodiscard]] static Constants::Event evaluateRecursive(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr evaluateRecursive_name = "json evaluate recursive";
    static auto constexpr evaluateRecursive_desc = "Recursively evaluates all string members in the JSON object as expressions, allowing for complex nested structures to be evaluated and set in one command.\n"
        "Usage: json evaluate recursive <key>\n"
        "\n"
        "Examples:\n"
        "evaluate recursive myObject\n"
        "MyObject could be an array of expressions, that are all evaluated and replaced with their results,\n"
        "or an object with nested objects and arrays containing expressions, all of which are evaluated and replaced with their results.\n";

    //------------------------------------------
    // Categories

    static auto constexpr query_name = "query";
    static auto constexpr query_desc = "Functions to manipulate JSON data via SQL query results";

    static auto constexpr json_name = "json";
    static auto constexpr json_desc = "Functions to manipulate JSON data via read-only JSON documents";

    static auto constexpr jsonEvaluate_name = "json evaluate";
    static auto constexpr jsonEvaluate_desc = "Functions to evaluate and set JSON data as expressions";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit ComplexData(ConstructorParams const& params) : DomainModule(params) {
        // Bind functions specific to complex data handling

        // SQL Queries
        bindCategory(query_name, query_desc);
        bindFunction(&ComplexData::querySet, querySet_name, querySet_desc);

        // Set from read only JSON documents
        bindCategory(json_name, json_desc);
        bindFunction(&ComplexData::jsonSet, jsonSet_name, jsonSet_desc);

        // Evaluate members as expressions
        bindCategory(jsonEvaluate_name, jsonEvaluate_desc);
        bindFunction(&ComplexData::evaluateMember, evaluateMember_name, evaluateMember_desc);
        bindFunction(&ComplexData::evaluateRecursive, evaluateRecursive_name, evaluateRecursive_desc);
    }
};
} // namespace Nebulite::Module::Domain::Common
#endif // MODULE_DOMAIN_COMMON_COMPLEXDATA_HPP
