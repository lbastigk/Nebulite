//------------------------------------------
// Includes

// External
#include <imgui.h>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"
#include "Nebulite/Module/Domain/Renderer/Debug.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {

Constants::Event Debug::updateHook() {
    return Constants::Event::success;
}

Constants::Event Debug::scale(Utility::Args::SegmentedStringView const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    auto const scaleFactor = args.tryGetAs<float>(1);
    if (!scaleFactor.has_value()) {
        return Constants::StandardCapture::Warning::Functional::invalidArgument(domain.capture);
    }

    ImGui::GetStyle().ScaleAllSizes(scaleFactor.value());
    return Constants::Event::success;
}

Constants::Event Debug::framebufferScale(Utility::Args::SegmentedStringView const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    auto const scaleFactor = args.tryGetAs<float>(1);
    if (!scaleFactor.has_value()) {
        return Constants::StandardCapture::Warning::Functional::invalidArgument(domain.capture);
    }

    ImGui::GetIO().DisplayFramebufferScale = ImVec2(scaleFactor.value(), scaleFactor.value());
    return Constants::Event::success;
}

Constants::Event Debug::mouseScale(Utility::Args::SegmentedStringView const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    auto const scaleFactor = args.tryGetAs<float>(1);
    if (!scaleFactor.has_value() || scaleFactor.value() <= 0.0f) {
        return Constants::StandardCapture::Warning::Functional::invalidArgument(domain.capture);
    }

    domain.setImguiMouseScale(scaleFactor.value());
    return Constants::Event::success;
}

Constants::Event Debug::renderScale(Utility::Args::SegmentedStringView const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    auto const scaleFactor = args.tryGetAs<float>(1);
    if (!scaleFactor.has_value() || scaleFactor.value() <= 0.0f) {
        return Constants::StandardCapture::Warning::Functional::invalidArgument(domain.capture);
    }

    domain.setImguiRenderScale(scaleFactor.value());
    return Constants::Event::success;
}

Debug::Debug(ConstructorParams const& params) : DomainModule(params) {
    bindCategory(imguiDebugName, imguiDebugDesc);
    bindFunction(&Debug::scale, scaleName, scaleDesc);
    bindFunction(&Debug::framebufferScale, framebufferScaleName, framebufferScaleDesc);
    bindFunction(&Debug::mouseScale, mouseScaleName, mouseScaleDesc);
    bindFunction(&Debug::renderScale, renderScaleName, renderScaleDesc);
}

} // namespace Nebulite::Module::Domain::Renderer
