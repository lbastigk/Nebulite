#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/KeyGroup.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {

class Grid final : public Interaction::Execution::DomainModule<Core::Renderer> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Event gridToggle(std::span<std::string const> const& args);
    static auto constexpr gridToggleName = "grid";
    static auto constexpr gridToggleDesc = "Toggle grid overlay on or off.\n"
        "Usage: grid [on/off]\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit Grid(ConstructorParams const& params) : DomainModule(params) {
        bindFunction(&Grid::gridToggle, gridToggleName, gridToggleDesc);
    }

    struct Key : Data::KeyGroup<"renderer.">{};

private:
    bool gridOn = false;
};

} // namespace Nebulite::Module::Domain::Renderer
