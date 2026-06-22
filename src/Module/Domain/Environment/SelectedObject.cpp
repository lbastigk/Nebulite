//------------------------------------------
// Includes

// Standard library
#include <filesystem>
#include <span>
#include <string>
#include <vector>

// Nebulite
#include "Constants/Event.hpp"
#include "Constants/StandardCapture.hpp"
#include "Core/RenderObject.hpp"
#include "Core/Renderer.hpp"
#include "Interaction/Context.hpp"
#include "Module/Domain/Environment/SelectedObject.hpp"
#include "Utility/StringHandler.hpp"

namespace Nebulite::Module::Domain::Environment {

//------------------------------------------
// Update

Constants::Event SelectedObject::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event SelectedObject::selectedObjectUpdate() const {
    if (selectedRenderObject) {
        return selectedRenderObject->update();
    }
    domain.capture.warning.println("No RenderObject selected! Use selectedObjectGet <id> to select a valid object.");
    return Constants::Event::Warning;
}

Constants::Event SelectedObject::selectedObjectGet(int const argc, char const** argv){
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (argc > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    // Supports only uint32_t ids
    size_t const idx = std::stoul(argv[1]);
    if (auto objAndScope = domain.getObjectFromIndex(idx); objAndScope.has_value()) {
        auto [obj, scope] = objAndScope.value();
        selectedRenderObject = obj;
        selectedRenderObjectData = scope;
        return Constants::Event::Success;
    }
    selectedRenderObject = nullptr;
    domain.capture.warning.println("No RenderObject with ID ", idx, " found. Selection cleared.");
    return Constants::Event::Warning;
}

Constants::Event SelectedObject::selectedObjectParse(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const command = Utility::StringHandler::recombineArgs(args.subspan(1));
    if (selectedRenderObject == nullptr || selectedRenderObjectData == nullptr) {
        domain.capture.warning.println("No RenderObject selected! Use selectedObjectGet <id> to select a valid object.");
        return Constants::Event::Warning;
    }

    Interaction::Context objectCtx = {
        {
            .self = *selectedRenderObject,
            .other = *selectedRenderObject,
            .global = ctx.global
        }
    };
    Interaction::ContextScope objectCtxScope = {
        {
            .self = *selectedRenderObjectData,
            .other = *selectedRenderObjectData,
            .global = ctxScope.global
        }
    };
    return selectedRenderObject->parseStr(std::string(__FUNCTION__) + " " + command, objectCtx, objectCtxScope);
}

} // namespace Nebulite::Module::Domain::Environment
