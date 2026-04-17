#include "Core/RenderObject.hpp"
#include "Module/Domain/RenderObject/Mirror.hpp"
#include "Nebulite.hpp"

namespace Nebulite::DomainModule::RenderObject {

//------------------------------------------
// Update
Constants::Event Mirror::updateHook() {
    if (mirrorEnabled || mirrorOnceEnabled) {
        // Mirror to GlobalSpace
        auto const token = getDomainModuleAccessToken(*this);
        auto const baseKey = Global::shareScope(token).getRootScope();
        Global::shareScope(token).setSubDoc(baseKey + mirrorKey, moduleScope);

        // Reset once-flag
        mirrorOnceEnabled = false;
    }
    return Constants::Event::Success;
}

//------------------------------------------
// Available Functions

Constants::Event Mirror::mirror_once() {
    if (auto const event = setupMirrorKey(); event != Constants::Event::Success)
        return event;
    mirrorOnceEnabled = true;
    return Constants::Event::Success;
}

Constants::Event Mirror::mirror_on() {
    if (auto const event = setupMirrorKey(); event != Constants::Event::Success)
        return event;
    mirrorEnabled = true;
    return Constants::Event::Success;
}

Constants::Event Mirror::mirror_off() {
    mirrorEnabled = false;
    return Constants::Event::Success;
}

Constants::Event Mirror::mirror_delete() const {
    auto const token = getDomainModuleAccessToken(*this);
    auto const baseKey = Global::shareScope(token).getRootScope();
    Global::shareScope(token).removeMember(baseKey + mirrorKey);
    return Constants::Event::Success;
}

Constants::Event Mirror::mirror_fetch() const {
    auto const token = getDomainModuleAccessToken(*this);
    auto const baseKey = Global::shareScope(token).getRootScope();
    if (Global::shareScope(token).memberType(baseKey + mirrorKey) != Data::KeyType::object) {
        domain.capture.warning.println("Mirror fetch failed: Key '" + mirrorKey + "' not of type document");
        return Constants::Event::Warning;
    }
    domain.deserialize(Global::shareScope(token).serialize(baseKey + mirrorKey));
    return Constants::Event::Success;
}

//------------------------------------------
// Helper

Constants::Event Mirror::setupMirrorKey() {
    // Only fetch key once we turn on mirroring
    auto const id = domain.getId();
    if (id < 1) {
        domain.capture.warning.println("Mirror key setup failed: RenderObject has invalid id");
        return Constants::Event::Warning;
    }

    auto const idx = Global::instance().getIndexFromId(id);
    if (!idx.has_value()) {
        mirrorKey = "";
        domain.capture.warning.println("Mirror key setup failed: RenderObject id not found in Renderer");
        return Constants::Event::Warning;
    }
    mirrorKey = "mirror.renderObject.idx" + std::to_string(idx.value());
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::RenderObject
