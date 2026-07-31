//------------------------------------------
// Includes

// Standard library
#include <string>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Core/RenderObject.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Module/Domain/RenderObject/Mirror.hpp"
#include "Nebulite/Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::RenderObject {

//------------------------------------------
// Update
Constants::Event Mirror::updateHook() {
    if (mirrorEnabled || mirrorOnceEnabled) {
        // TODO: store the callers Global context instead of using the access token!
        // Mirror to GlobalSpace
        auto const token = getDomainModuleAccessToken(*this);
        auto const baseKey = Global::shareScope(token).getRootScope();
        Global::shareScope(token).setSubDoc(baseKey.addMember(mirrorKey), moduleScope);

        // Reset once-flag
        mirrorOnceEnabled = false;
    }
    return Constants::Event::success;
}

//------------------------------------------
// Available Functions

Constants::Event Mirror::mirrorOnce() {
    if (auto const event = setupMirrorKey(); event != Constants::Event::success)
        return event;
    mirrorOnceEnabled = true;
    return Constants::Event::success;
}

Constants::Event Mirror::mirrorOn() {
    if (auto const event = setupMirrorKey(); event != Constants::Event::success)
        return event;
    mirrorEnabled = true;
    return Constants::Event::success;
}

Constants::Event Mirror::mirrorOff() {
    mirrorEnabled = false;
    return Constants::Event::success;
}

Constants::Event Mirror::mirrorDelete() const {
    auto const token = getDomainModuleAccessToken(*this);
    auto const baseKey = Global::shareScope(token).getRootScope();
    Global::shareScope(token).removeMember(baseKey.addMember(mirrorKey));
    return Constants::Event::success;
}

Constants::Event Mirror::mirrorFetch() const {
    auto const token = getDomainModuleAccessToken(*this);
    auto const baseKey = Global::shareScope(token).getRootScope();
    if (Global::shareScope(token).memberType(baseKey.addMember(mirrorKey)) != Data::KeyType::object) {
        domain.capture.warning.println("Mirror fetch failed: Key '" + mirrorKey + "' not of type document");
        return Constants::Event::warning;
    }
    domain.deserialize(Global::shareScope(token).serialize(baseKey.addMember(mirrorKey)));
    return Constants::Event::success;
}

//------------------------------------------
// Helper

Constants::Event Mirror::setupMirrorKey() {
    // Only fetch key once we turn on mirroring
    auto const id = domain.getId();
    if (id < 1) {
        domain.capture.warning.println("Mirror key setup failed: RenderObject has invalid id");
        return Constants::Event::warning;
    }

    auto const idx = Global::instance().getIndexFromId(id);
    if (!idx.has_value()) {
        mirrorKey = "";
        domain.capture.warning.println("Mirror key setup failed: RenderObject id not found in Renderer");
        return Constants::Event::warning;
    }
    mirrorKey = "mirror.renderObject.idx" + std::to_string(idx.value());
    return Constants::Event::success;
}

} // namespace Nebulite::Module::Domain::RenderObject
