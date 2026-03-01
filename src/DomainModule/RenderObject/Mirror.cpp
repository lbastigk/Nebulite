#include "DomainModule/RenderObject/Mirror.hpp"

#include "Constants/KeyNames.hpp"
#include "Core/RenderObject.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::RenderObject {

//------------------------------------------
// Update
Constants::Error Mirror::update() {
    if (mirrorEnabled || mirrorOnceEnabled) {
        // Mirror to GlobalSpace
        auto const token = getDomainModuleAccessToken(*this);
        auto const baseKey = Global::shareScopeBase(token).getRootScope();
        Global::shareScopeBase(token).setSubDoc(baseKey + mirrorKey, moduleScope);

        // Reset once-flag
        mirrorOnceEnabled = false;
    }
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

Constants::Error Mirror::mirror_once() {
    if (auto const err = setupMirrorKey(); err.isError())
        return err;
    mirrorOnceEnabled = true;
    return Constants::ErrorTable::NONE();
}

Constants::Error Mirror::mirror_on() {
    if (auto const err = setupMirrorKey(); err.isError())
        return err;
    mirrorEnabled = true;
    return Constants::ErrorTable::NONE();
}

Constants::Error Mirror::mirror_off() {
    mirrorEnabled = false;
    return Constants::ErrorTable::NONE();
}

Constants::Error Mirror::mirror_delete() const {
    auto const token = getDomainModuleAccessToken(*this);
    auto const baseKey = Global::shareScopeBase(token).getRootScope();
    Global::shareScopeBase(token).removeMember(baseKey + mirrorKey);
    return Constants::ErrorTable::NONE();
}

Constants::Error Mirror::mirror_fetch() const {
    auto const token = getDomainModuleAccessToken(*this);
    auto const baseKey = Global::shareScopeBase(token).getRootScope();
    if (Global::shareScopeBase(token).memberType(baseKey + mirrorKey) != Data::KeyType::object) {
        return Constants::ErrorTable::addError("Mirror fetch failed: Key '" + mirrorKey + "' not of type document", Constants::Error::NON_CRITICAL);
    }
    domain.deserialize(Global::shareScopeBase(token).serialize(baseKey + mirrorKey));
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Helper

Constants::Error Mirror::setupMirrorKey() {
    // Only fetch key once we turn on mirroring
    int const id = moduleScope.get<int>(Constants::KeyNames::RenderObject::id).value_or(0);
    if (id < 1) {
        return Constants::ErrorTable::addError("Mirror key setup failed: RenderObject has invalid id", Constants::Error::NON_CRITICAL);
    }

    mirrorKey = "mirror.renderObject.id" + std::to_string(id);
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::RenderObject
