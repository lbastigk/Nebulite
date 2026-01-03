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
        Nebulite::global().getDoc().setSubDoc(mirrorKey.c_str(), getDoc());

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
    Nebulite::global().getDoc().removeKey(mirrorKey.c_str());
    return Constants::ErrorTable::NONE();
}

Constants::Error Mirror::mirror_fetch() {
    if (Nebulite::global().getDoc().memberType(mirrorKey) != Data::KeyType::object) {
        return Constants::ErrorTable::addError("Mirror fetch failed: Key '" + mirrorKey + "' not of type document", Constants::Error::NON_CRITICAL);
    }
    domain.deserialize(Nebulite::global().getDoc().serialize(mirrorKey));
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Helper

Constants::Error Mirror::setupMirrorKey() {
    // Only fetch key once we turn on mirroring
    int const id = getDoc().get<int>(Constants::KeyNames::RenderObject::id, 0);
    if (id < 1) {
        return Constants::ErrorTable::addError("Mirror key setup failed: RenderObject has invalid id", Constants::Error::NON_CRITICAL);
    }

    mirrorKey = "mirror.renderObject.id" + std::to_string(id);
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::RenderObject
