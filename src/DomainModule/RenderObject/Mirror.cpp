#include "DomainModule/RenderObject/Mirror.hpp"

#include "Constants/KeyNames.hpp"
#include "Core/RenderObject.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::RenderObject {

std::string const Mirror::mirror_name = "mirror";
std::string const Mirror::mirror_desc = R"(Mirror utilities for RenderObject to GlobalSpace synchronization)";

//------------------------------------------
// Update
Constants::Error Mirror::update() {
    if (mirrorEnabled || mirrorOnceEnabled) {
        // Values
        auto const globalDoc = Nebulite::global().getDoc();
        auto const objectDoc = domain->getDoc();

        // Mirror to GlobalSpace
        globalDoc->setSubDoc(mirrorKey.c_str(), *objectDoc);

        // Reset once-flag
        mirrorOnceEnabled = false;
    }
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

// NOLINTNEXTLINE
Constants::Error Mirror::mirror_once(int argc, char** argv) {
    if (auto const err = setupMirrorKey(); err.isError())
        return err;
    mirrorOnceEnabled = true;
    return Constants::ErrorTable::NONE();
}

std::string const Mirror::mirror_once_name = "mirror once";
std::string const Mirror::mirror_once_desc = R"(Mirrors the object to the GlobalSpace document once on next update

Usage: mirror once

Mirroring is only done for one frame.
Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>
)";

// NOLINTNEXTLINE
Constants::Error Mirror::mirror_on(int argc, char** argv) {
    if (auto const err = setupMirrorKey(); err.isError())
        return err;
    mirrorEnabled = true;
    return Constants::ErrorTable::NONE();
}

std::string const Mirror::mirror_on_name = "mirror on";
std::string const Mirror::mirror_on_desc = R"(Enables mirroring to the GlobalSpace document

Usage: mirror on

Constant mirroring is active until turned off with 'mirror off'

Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>
)";

// NOLINTNEXTLINE
Constants::Error Mirror::mirror_off(int argc, char** argv) {
    mirrorEnabled = false;
    return Constants::ErrorTable::NONE();
}

std::string const Mirror::mirror_off_name = "mirror off";
std::string const Mirror::mirror_off_desc = R"(Disables mirroring to the GlobalSpace document

Usage: mirror off

Constant mirroring is inactive until turned on again with 'mirror on'

Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>
)";

// NOLINTNEXTLINE
Constants::Error Mirror::mirror_delete(int argc, char** argv) {
    Nebulite::global().getDoc()->removeKey(mirrorKey.c_str());
    return Constants::ErrorTable::NONE();
}

std::string const Mirror::mirror_delete_name = "mirror delete";
std::string const Mirror::mirror_delete_desc = R"(Deletes the GlobalSpace document entry for this RenderObject

Usage: mirror delete

Mirrors are removed from the GlobalSpace document under key "mirror.renderObject.id<id>
)";

// NOLINTNEXTLINE
Constants::Error Mirror::mirror_fetch(int argc, char** argv) {
    if (Nebulite::global().getDoc()->memberType(mirrorKey) != Data::JSON::KeyType::object) {
        return Constants::ErrorTable::addError("Mirror fetch failed: Key '" + mirrorKey + "' not of type document", Constants::Error::NON_CRITICAL);
    }
    domain->deserialize(Nebulite::global().getDoc()->serialize(mirrorKey));
    return Constants::ErrorTable::NONE();
}

std::string const Mirror::mirror_fetch_name = "mirror fetch";
std::string const Mirror::mirror_fetch_desc = R"(Deserializes the RenderObject from the GlobalSpace document entry

Usage: mirror fetch

Mirrors are fetched from the GlobalSpace document under key "mirror.renderObject.id<id>
)";

//------------------------------------------
// Helper

Constants::Error Mirror::setupMirrorKey() {
    // Only fetch key once we turn on mirroring
    int const id = domain->getDoc()->get<int>(Constants::KeyNames::RenderObject::id, 0);
    if (id < 1) {
        return Constants::ErrorTable::addError("Mirror key setup failed: RenderObject has invalid id", Constants::Error::NON_CRITICAL);
    }

    mirrorKey = "mirror.renderObject.id" + std::to_string(id);
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::RenderObject