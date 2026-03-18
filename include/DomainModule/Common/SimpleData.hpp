/**
 * @file SimpleData.hpp
 * @brief DomainModule for simple data operations on domain class Nebulite::Data::JSON
 */

#ifndef NEBULITE_DOMAINMODULE_COMMON_SIMPLEDATA_HPP
#define NEBULITE_DOMAINMODULE_COMMON_SIMPLEDATA_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Common {
/**
 * @class Nebulite::DomainModule::Common::SimpleData
 * @brief DomainModule for simple data operations on domain class Nebulite::Data::JSON
 */
NEBULITE_DOMAINMODULE(Interaction::Execution::Domain, SimpleData) {
public:
    [[nodiscard]] Constants::Event update() override ;
    void reinit() override {} // No reinitialization needed, SimpleData is stateless

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event set(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr set_name = "set";
    static auto constexpr set_desc = "Set a key to a value in the JSON document.\n"
        "\n"
        "Usage: set <key> <value>\n"
        "\n"
        "Note: All values are stored as strings.\n";

    [[nodiscard]] static Constants::Event move(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr move_name = "move";
    static auto constexpr move_desc = "Move data from one key to another.\n"
        "\n"
        "Usage: move <source_key> <destination_key>\n";

    [[nodiscard]] static Constants::Event copy(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr copy_name = "copy";
    static auto constexpr copy_desc = "Copy data from one key to another.\n"
        "\n"
        "Usage: copy <source_key> <destination_key>\n";

    [[nodiscard]] static Constants::Event keyDelete(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr keyDelete_name = "keyDelete";
    static auto constexpr keyDelete_desc = "Delete a key from the JSON document.\n"
        "\n"
        "Usage: keyDelete <key>\n";

    [[nodiscard]] static Constants::Event ensureArray(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr ensureArray_name = "ensure-array";
    static auto constexpr ensureArray_desc = "Ensure that a key is an array, converting a value to an array if necessary.\n"
        "\n"
        "Usage: ensure-array <key>\n";

    [[nodiscard]] static Constants::Event push_back(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr push_back_name = "push-back";
    static auto constexpr push_back_desc = "Push a value to the back of an array.\n"
        "\n"
        "Usage: push-back <key> <value>\n";

    [[nodiscard]] static Constants::Event pop_back(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr pop_back_name = "pop-back";
    static auto constexpr pop_back_desc = "Pop a value from the back of an array.\n"
        "\n"
        "Usage: pop-back <key>\n";

    [[nodiscard]] static Constants::Event push_front(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr push_front_name = "push-front";
    static auto constexpr push_front_desc = "Push a value to the front of an array.\n"
        "\n"
        "Usage: push-front <key> <value>\n";

    [[nodiscard]] static Constants::Event pop_front(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr pop_front_name = "pop-front";
    static auto constexpr pop_front_desc = "Pop a value from the front of an array.\n"
        "\n"
        "Usage: pop-front <key>\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Interaction::Execution::Domain, SimpleData) {
        // Bind functions specific to complex data handling
        BIND_FUNCTION(&SimpleData::set, set_name, set_desc);

        // Internal move/copy
        BIND_FUNCTION(&SimpleData::move, move_name, move_desc);
        BIND_FUNCTION(&SimpleData::copy, copy_name, copy_desc);

        // Internal key deletion
        BIND_FUNCTION(&SimpleData::keyDelete, keyDelete_name, keyDelete_desc);

        // Array manipulation
        BIND_FUNCTION(&SimpleData::push_back, push_back_name, push_back_desc);
        BIND_FUNCTION(&SimpleData::pop_back, pop_back_name, pop_back_desc);
        BIND_FUNCTION(&SimpleData::push_front, push_front_name, push_front_desc);
        BIND_FUNCTION(&SimpleData::pop_front, pop_front_name, pop_front_desc);
        BIND_FUNCTION(&SimpleData::ensureArray, ensureArray_name, ensureArray_desc);
    }
};
} // namespace Nebulite::DomainModule::Common
#endif // NEBULITE_DOMAINMODULE_COMMON_SIMPLEDATA_HPP
