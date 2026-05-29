/**
 * @file Ruleset.hpp
 * @brief Contains the declaration of the Ruleset DomainModule for the GlobalSpace domain.
 */

#ifndef MODULE_DOMAIN_COMMON_RULESET_HPP
#define MODULE_DOMAIN_COMMON_RULESET_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/KeyGroup.hpp"
#include "Interaction/Rules/Listener.hpp"
#include "Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
/**
 * @class Nebulite::Module::Domain::Common::Ruleset
 * @brief DomainModule for managing rulesets.
 */
class Ruleset final : public Base::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override ;
    void reinit() override ;

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event invokeOnce(std::span<std::string_view const> const& args) const ;
    static auto constexpr invokeOnceName = "ruleset invoke";
    static auto constexpr invokeOnceDesc = "Applies the given ruleset on the next update\n"
        "\n"
        "Usage: invoke <ruleset>\n"
        "\n"
        "Use invoke ::help to list all available static rulesets.";

    [[nodiscard]] Constants::Event broadcast(std::span<std::string_view const> const& args) const ;
    static auto constexpr broadcast_name = "ruleset broadcast";
    static auto constexpr broadcast_desc = "Broadcasts a ruleset to its specified topic.\n"
        "Usage: broadcast <ruleset>\n"
        "\n"
        "- ruleset: The ruleset content to be broadcasted.";

    [[nodiscard]] Constants::Event listen(std::span<std::string_view const> const& args) const ;
    static auto constexpr listen_name = "ruleset listen";
    static auto constexpr listen_desc = "Listens for rulesets on a specified topic.\n"
        "Usage: listen <topic>\n"
        "\n"
        "- topic: The topic to listen for incoming rulesets.";

    [[nodiscard]] Constants::Event reload();
    static auto constexpr reload_name = "ruleset reload";
    static auto constexpr reload_desc = "Reloads all rulesets for this domain on the next update.\n"
        "\n"
        "Usage: ruleset reload\n"
        "\n"
        "All rulesets are re-evaluated and reloaded on the next update cycle.\n";

    //------------------------------------------
    // Categories

    static auto constexpr ruleset_name = "ruleset";
    static auto constexpr ruleset_desc = "Functions for managing rulesets.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit Ruleset(ConstructorParams const& params) : DomainModule(params) {
        //------------------------------------------
        // Binding functions to the FuncTree
        bindCategory(ruleset_name, ruleset_desc);
        bindFunction(&Ruleset::invokeOnce, invokeOnceName, invokeOnceDesc);
        bindFunction(&Ruleset::broadcast, broadcast_name, broadcast_desc);
        bindFunction(&Ruleset::listen, listen_name, listen_desc);
        bindFunction(&Ruleset::reload, reload_name, reload_desc);

        //------------------------------------------
        // Initialize empty arrays for broadcast/listen, if keys don't exist
        Key const scopedKey(moduleScope);
        if (params.scope.memberType(scopedKey.broadcast) == Data::KeyType::null) {
            params.scope.setEmptyArray(scopedKey.broadcast);
        }
        if (params.scope.memberType(scopedKey.listen) == Data::KeyType::null) {
            params.scope.setEmptyArray(scopedKey.listen);
        }
    }

    struct Key : Data::KeyGroup<Data::ScopedKey::domainRootScope> {
        explicit Key(Data::JsonScope const& scope) {
            broadcast = scope.getRootScope().addMember("ruleset").addMember("list");
            listen = scope.getRootScope().addMember("ruleset").addMember("listen");
            costLocal = scope.getRootScope().addMember("ruleset").addMember("cost").addMember("local");
            costGlobal = scope.getRootScope().addMember("ruleset").addMember("cost").addMember("global");
            costTotal = scope.getRootScope().addMember("ruleset").addMember("cost").addMember("total");
        }
        Data::ScopedKey broadcast;
        Data::ScopedKey listen;
        Data::ScopedKey costLocal;
        Data::ScopedKey costGlobal;
        Data::ScopedKey costTotal;
    };

private:
    // Check if the module is initialized and ready to use
    bool initialized = false;

    // Size of subscriptions
    size_t subscription_size = 0;

    // Check if rulesets need to be reloaded
    bool reloadRulesets = true;

    // Check if no rulesets are present
    bool noRulesets = false;

    // Global rulesets, intended for self-other-global interaction
    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsGlobal;

    // Internal rulesets, intended for self-global interaction
    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsLocal;

    // Topic subscriptions
    std::vector<std::shared_ptr<Interaction::Rules::Listener>> listeners;
};
} // namespace Nebulite::Module::Domain::Common
#endif // MODULE_DOMAIN_COMMON_RULESET_HPP
