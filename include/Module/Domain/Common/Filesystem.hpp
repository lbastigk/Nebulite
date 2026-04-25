/**
 * @file Filesystem.hpp
 * @brief Implementation of force and clearForce functions for forcing JSON variable values.
 */

#ifndef NEBULITE_DOMAIN_MODULE_COMMON_FILESYSTEM_HPP
#define NEBULITE_DOMAIN_MODULE_COMMON_FILESYSTEM_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
NEBULITE_DOMAINMODULE(Nebulite::Interaction::Execution::Domain, Filesystem) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event cat(std::span<std::string const> const& args) const ;
    static auto constexpr cat_name = "cat";
    static auto constexpr cat_desc = "Opens a provided file and prints its content to the console.\n"
        "Usage: cat <filePath>\n"
        "\n"
        "- <filePath>: The path to the file to be read and printed.\n";

    [[nodiscard]] Constants::Event ls(std::span<std::string const> const& args) const ;
    static auto constexpr ls_name = "ls";
    static auto constexpr ls_desc = "Lists the contents of a provided directory.\n"
        "Usage: ls [directoryPath]\n"
        "\n"
        "- [directoryPath]: The path to the directory to list. If not provided, lists the current directory.\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Interaction::Execution::Domain, Filesystem) {
        // Binding
        bindFunction(&Filesystem::cat, cat_name, cat_desc);
        bindFunction(&Filesystem::ls, ls_name, ls_desc);
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
} // namespace Nebulite::Module::Domain::Common
#endif // NEBULITE_DOMAIN_MODULE_COMMON_FILESYSTEM_HPP
