#ifndef NEBULITE_MODULE_DOMAIN_COMMON_FILESYSTEM_HPP
#define NEBULITE_MODULE_DOMAIN_COMMON_FILESYSTEM_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string>
#include <string_view>

// External
#include <absl/container/flat_hash_map.h>

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
class Filesystem final : public Base::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event cat(std::span<std::string_view const> const& args) const ;
    static auto constexpr cat_name = "cat";
    static auto constexpr cat_desc = "Opens a provided file and prints its content to the console.\n"
        "Usage: cat <filePath>\n"
        "\n"
        "- <filePath>: The path to the file to be read and printed.\n";

    [[nodiscard]] Constants::Event ls(std::span<std::string_view const> const& args) const ;
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
    explicit Filesystem(ConstructorParams const& params) : DomainModule(params) {
        // Binding
        bindFunction(&Filesystem::cat, cat_name, cat_desc);
        bindFunction(&Filesystem::ls, ls_name, ls_desc);
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
} // namespace Nebulite::Module::Domain::Common
#endif // NEBULITE_MODULE_DOMAIN_COMMON_FILESYSTEM_HPP
