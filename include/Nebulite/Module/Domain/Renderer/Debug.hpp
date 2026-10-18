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

    static auto constexpr scaleName = "imguidb scale";
    static auto constexpr scaleDesc = "Scales all Dear ImGui style sizes.\n"
        "\n"
        "Usage: imguidb scale <scale>\n";

    [[nodiscard]] Constants::Event framebufferScale(Utility::Args::SegmentedStringView const& args) const;
    static auto constexpr framebufferScaleName = "imguidb framebuffer-scale";
    static auto constexpr framebufferScaleDesc = "Overrides Dear ImGui's framebuffer scale for the current frame.\n"
        "\n"
        "Usage: imguidb framebuffer-scale <scale>\n";

    [[nodiscard]] Constants::Event mouseScale(Utility::Args::SegmentedStringView const& args) const;
    static auto constexpr mouseScaleName = "imguidb mouse-scale";
    static auto constexpr mouseScaleDesc = "Scales mouse coordinates forwarded to Dear ImGui.\n"
        "\n"
        "Usage: imguidb mouse-scale <scale>\n";

    [[nodiscard]] Constants::Event renderScale(Utility::Args::SegmentedStringView const& args) const;
    static auto constexpr renderScaleName = "imguidb render-scale";
    static auto constexpr renderScaleDesc = "Scales Dear ImGui vertices during its SDL render pass.\n"
        "\n"
        "Usage: imguidb render-scale <scale>\n";

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
