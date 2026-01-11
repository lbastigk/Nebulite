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
        auto const globalScope = Nebulite::globalDoc().shareScope(*this).getRootScope();
        Nebulite::globalDoc().shareScope(*this).setSubDoc(globalScope + mirrorKey, moduleScope);

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

Constants::Error Mirror::mirror_delete() {
    auto const globalScope = Nebulite::globalDoc().shareScope(*this).getRootScope();
    Nebulite::globalDoc().shareScope(*this).removeKey(globalScope + mirrorKey);
    return Constants::ErrorTable::NONE();
}

Constants::Error Mirror::mirror_fetch() {
    auto const globalScope = Nebulite::globalDoc().shareScope(*this).getRootScope();
    if (Nebulite::globalDoc().shareScope(*this).memberType(globalScope + mirrorKey) != Data::KeyType::object) {
        return Constants::ErrorTable::addError("Mirror fetch failed: Key '" + mirrorKey + "' not of type document", Constants::Error::NON_CRITICAL);
    }
    domain.deserialize(Nebulite::globalDoc().shareScope(*this).serialize(globalScope + mirrorKey));
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Helper

Constants::Error Mirror::setupMirrorKey() {
    // Only fetch key once we turn on mirroring
    int const id = moduleScope.get<int>(Constants::KeyNames::RenderObject::id, 0);
    if (id < 1) {
        return Constants::ErrorTable::addError("Mirror key setup failed: RenderObject has invalid id", Constants::Error::NON_CRITICAL);
    }

    mirrorKey = "mirror.renderObject.id" + std::to_string(id);
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::RenderObject
