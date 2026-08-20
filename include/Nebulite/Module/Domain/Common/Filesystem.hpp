#ifndef NEBULITE_MODULE_DOMAIN_COMMON_FILESYSTEM_HPP
#define NEBULITE_MODULE_DOMAIN_COMMON_FILESYSTEM_HPP

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
class Filesystem final : public Base::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event cat(std::span<std::string_view const> args) const ;
    static auto constexpr catName = "cat";
    static auto constexpr catDesc = "Opens a provided file and prints its content to the console.\n"
        "Usage: cat <filePath>\n"
        "\n"
        "- <filePath>: The path to the file to be read and printed.\n";

    [[nodiscard]] Constants::Event ls(std::span<std::string_view const> args) const ;
    static auto constexpr lsName = "ls";
    static auto constexpr lsDesc = "Lists the contents of a provided directory.\n"
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
        bindFunction(&Filesystem::cat, catName, catDesc);
        bindFunction(&Filesystem::ls, lsName, lsDesc);
    }
};
} // namespace Nebulite::Module::Domain::Common
#endif // NEBULITE_MODULE_DOMAIN_COMMON_FILESYSTEM_HPP
