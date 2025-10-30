#include "DomainModule/RenderObject/RODM_Mirror.hpp"

#include "Constants/KeyNames.hpp"
#include "Core/RenderObject.hpp"
#include "Core/GlobalSpace.hpp"

namespace Nebulite::DomainModule::RenderObject {

std::string const Mirror::mirror_name = "mirror";
std::string const Mirror::mirror_desc = R"(Mirror utilities for RenderObject to GlobalSpace synchronization)";

//------------------------------------------
// Update
Nebulite::Constants::Error Mirror::update(){
    if (mirrorEnabled || mirrorOnceEnabled){
        // Values
        auto globalDoc = domain->getGlobalSpace()->getDoc();
        auto objectDoc = domain->getDoc();

        // Mirror to GlobalSpace
        globalDoc->set_subdoc(mirrorKey.c_str(), objectDoc);

        // Reset once-flag
        mirrorOnceEnabled = false;
    }
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

Nebulite::Constants::Error Mirror::mirror_once(int argc,  char** argv){
    auto err = setupMirrorKey();
    if(err.isError()) return err;
    mirrorOnceEnabled = true;
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Mirror::mirror_once_name = "mirror once";
std::string const Mirror::mirror_once_desc = R"(Mirrors the object to the GlobalSpace document once on next update

Usage: mirror once

Mirroring is only done for one frame.
Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>
)";

Nebulite::Constants::Error Mirror::mirror_on(int argc,  char** argv){
    auto err = setupMirrorKey();
    if(err.isError()) return err;
    mirrorEnabled = true;
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Mirror::mirror_on_name = "mirror on";
std::string const Mirror::mirror_on_desc = R"(Enables mirroring to the GlobalSpace document

Usage: mirror on

Constant mirroring is active until turned off with 'mirror off'

Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>
)";

Nebulite::Constants::Error Mirror::mirror_off(int argc,  char** argv){
    mirrorEnabled = false;
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Mirror::mirror_off_name = "mirror off";
std::string const Mirror::mirror_off_desc = R"(Disables mirroring to the GlobalSpace document

Usage: mirror off

Constant mirroring is inactive until turned on again with 'mirror on'

Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>
)";

Nebulite::Constants::Error Mirror::mirror_delete(int argc,  char** argv){
    domain->getGlobalSpace()->getDoc()->remove_key(mirrorKey.c_str());
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Mirror::mirror_delete_name = "mirror delete";
std::string const Mirror::mirror_delete_desc = R"(Deletes the GlobalSpace document entry for this RenderObject

Usage: mirror delete

Mirrors are removed from the GlobalSpace document under key "mirror.renderObject.id<id>
)";

Nebulite::Constants::Error Mirror::mirror_fetch(int argc,  char** argv){
    Nebulite::Utility::JSON::KeyType keyStatus = domain->getGlobalSpace()->getDoc()->memberCheck(mirrorKey.c_str());
    if (keyStatus != Nebulite::Utility::JSON::KeyType::document){
        return Nebulite::Constants::ErrorTable::addError("Mirror fetch failed: Key '" + mirrorKey + "' not of type document", Nebulite::Constants::Error::NON_CRITICAL);
    }
    domain->deserialize(domain->getGlobalSpace()->getDoc()->serialize(mirrorKey.c_str()));
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Mirror::mirror_fetch_name = "mirror fetch";
std::string const Mirror::mirror_fetch_desc = R"(Deserializes the RenderObject from the GlobalSpace document entry

Usage: mirror fetch

Mirrors are fetched from the GlobalSpace document under key "mirror.renderObject.id<id>
)";

//------------------------------------------
// Helper

Nebulite::Constants::Error Mirror::setupMirrorKey(){
    // Only fetch key once we turn on mirroring
    int id = domain->get<int>(Nebulite::Constants::keyName.renderObject.id.c_str(), 0);
    if(id < 1){
        return Nebulite::Constants::ErrorTable::addError("Mirror key setup failed: RenderObject has invalid id", Nebulite::Constants::Error::NON_CRITICAL);
    }

    mirrorKey = "mirror.renderObject.id" + std::to_string(id);
    return Nebulite::Constants::ErrorTable::NONE();
}

}   // namespace Nebulite::DomainModule::RenderObject