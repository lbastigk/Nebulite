#ifndef NEBULITE_MODULE_DOMAIN_RENDERER_DEBUG_HPP
#define NEBULITE_MODULE_DOMAIN_RENDERER_DEBUG_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"

//------------------------------------------
namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

namespace Nebulite::Module::Domain::Renderer {
/**
 * @class Nebulite::Module::Domain::Renderer::Debug
 * @brief DomainModule for renderer debugging utilities.
 */
class Debug final : public Base::DomainModule<Core::Renderer> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event scale(Utility::Args::SegmentedStringView const& args) const;

    static auto constexpr scaleName = "scale";
    static auto constexpr scaleDesc = "Scales all Dear ImGui style sizes.\n"
        "\n"
        "Usage: imguidb scale <scale>\n";

    //------------------------------------------
    // Categories

    static auto constexpr imguiDebugName = "imguidb";
    static auto constexpr imguiDebugDesc = "Dear ImGui debugging utilities.";

    //------------------------------------------
    // Setup

    explicit Debug(ConstructorParams const& params);
};
} // namespace Nebulite::Module::Domain::Renderer
#endif // NEBULITE_MODULE_DOMAIN_RENDERER_DEBUG_HPP
