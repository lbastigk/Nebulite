/**
 * @file SimpleData.hpp
 * @brief DomainModule for simple data operations on domain class Nebulite::Data::JSON
 */

#ifndef NEBULITE_JSDM_SIMPLEDATA_HPP
#define NEBULITE_JSDM_SIMPLEDATA_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data


//------------------------------------------
namespace Nebulite::DomainModule::JsonScope {
/**
 * @class Nebulite::DomainModule::JsonScope::SimpleData
 * @brief DomainModule for simple data operations on domain class Nebulite::Data::JSON
 */
NEBULITE_DOMAINMODULE(Nebulite::Data::JsonScope, SimpleData) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error set(int argc, char** argv);
    static std::string_view constexpr set_name = "set";
    static std::string_view constexpr set_desc = "Set a key to a value in the JSON document.\n"
        "\n"
        "Usage: set <key> <value>\n"
        "\n"
        "Note: All values are stored as strings.\n";

    Constants::Error move(int argc, char** argv);
    static std::string_view constexpr move_name = "move";
    static std::string_view constexpr move_desc = "Move data from one key to another.\n"
        "\n"
        "Usage: move <source_key> <destination_key>\n";

    Constants::Error copy(int argc, char** argv);
    static std::string_view constexpr copy_name = "copy";
    static std::string_view constexpr copy_desc = "Copy data from one key to another.\n"
        "\n"
        "Usage: copy <source_key> <destination_key>\n";

    Constants::Error keyDelete(int argc, char** argv);
    static std::string_view constexpr keyDelete_name = "keyDelete";
    static std::string_view constexpr keyDelete_desc = "Delete a key from the JSON document.\n"
        "\n"
        "Usage: keyDelete <key>\n";

    Constants::Error ensureArray(int argc, char** argv);
    static std::string_view constexpr ensureArray_name = "ensure-array";
    static std::string_view constexpr ensureArray_desc = "Ensure that a key is an array, converting a value to an array if necessary.\n"
        "\n"
        "Usage: ensure-array <key>\n";

    Constants::Error push_back(int argc, char** argv);
    static std::string_view constexpr push_back_name = "push-back";
    static std::string_view constexpr push_back_desc = "Push a value to the back of an array.\n"
        "\n"
        "Usage: push-back <key> <value>\n";

    Constants::Error pop_back(int argc, char** argv);
    static std::string_view constexpr pop_back_name = "pop-back";
    static std::string_view constexpr pop_back_desc = "Pop a value from the back of an array.\n"
        "\n"
        "Usage: pop-back <key>\n";

    Constants::Error push_front(int argc, char** argv);
    static std::string_view constexpr push_front_name = "push-front";
    static std::string_view constexpr push_front_desc = "Push a value to the front of an array.\n"
        "\n"
        "Usage: push-front <key> <value>\n";

    Constants::Error pop_front(int argc, char** argv);
    static std::string_view constexpr pop_front_name = "pop-front";
    static std::string_view constexpr pop_front_desc = "Pop a value from the front of an array.\n"
        "\n"
        "Usage: pop-front <key>\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Data::JsonScope, SimpleData) {
        // Bind functions specific to complex data handling
        BINDFUNCTION(&SimpleData::set, set_name, set_desc);

        // Internal move/copy
        BINDFUNCTION(&SimpleData::move, move_name, move_desc);
        BINDFUNCTION(&SimpleData::copy, copy_name, copy_desc);

        // Internal key deletion
        BINDFUNCTION(&SimpleData::keyDelete, keyDelete_name, keyDelete_desc);

        // Array manipulation
        BINDFUNCTION(&SimpleData::push_back, push_back_name, push_back_desc);
        BINDFUNCTION(&SimpleData::pop_back, pop_back_name, pop_back_desc);
        BINDFUNCTION(&SimpleData::push_front, push_front_name, push_front_desc);
        BINDFUNCTION(&SimpleData::pop_front, pop_front_name, pop_front_desc);
        BINDFUNCTION(&SimpleData::ensureArray, ensureArray_name, ensureArray_desc);
    }
};
} // namespace Nebulite::DomainModule::JsonScope
#endif // NEBULITE_JSDM_SIMPLEDATA_HPP
