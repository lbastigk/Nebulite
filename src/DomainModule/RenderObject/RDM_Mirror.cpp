#include "DomainModule/RenderObject/RDM_Mirror.hpp"

#include "Core/RenderObject.hpp"
#include "Core/GlobalSpace.hpp"

namespace Nebulite{
namespace DomainModule{
namespace RenderObject{

void Mirror::update() {
    if (mirrorEnabled || mirrorOnceEnabled) {
        // Values
        auto globalDoc = domain->getGlobalSpace()->getDoc();
        auto objectDoc = domain->getDoc();

        // Mirror to GlobalSpace
        globalDoc->set_subdoc(mirrorKey.c_str(), objectDoc);

        // Reset once-flag
        mirrorOnceEnabled = false;
    }
}

Nebulite::Constants::Error Mirror::mirrorOn(int argc, char* argv[]) {
    auto err = setupMirrorKey();
    if(err.isError()) return err;
    mirrorEnabled = true;
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Mirror::mirrorOff(int argc, char* argv[]) {
    mirrorEnabled = false;
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Mirror::mirrorDelete(int argc, char* argv[]) {
    domain->getGlobalSpace()->getDoc()->remove_key(mirrorKey.c_str());
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Mirror::mirrorFetch(int argc, char* argv[]) {
    Nebulite::Utility::JSON::KeyType keyStatus = domain->getGlobalSpace()->getDoc()->memberCheck(mirrorKey.c_str());
    if (keyStatus != Nebulite::Utility::JSON::KeyType::document) {
        return Nebulite::Constants::ErrorTable::addError("Mirror fetch failed: Key '" + mirrorKey + "' not of type document", Nebulite::Constants::Error::NON_CRITICAL);
    }
    domain->deserialize(domain->getGlobalSpace()->getDoc()->serialize(mirrorKey.c_str()));
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Mirror::mirrorOnce(int argc, char* argv[]) {
    auto err = setupMirrorKey();
    if(err.isError()) return err;
    mirrorOnceEnabled = true;
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Mirror::setupMirrorKey() {
    // Only fetch key once we turn on mirroring
    int id = domain->get<int>(Nebulite::Constants::keyName.renderObject.id.c_str(), 0);
    if(id < 1){
        return Nebulite::Constants::ErrorTable::addError("Mirror key setup failed: RenderObject has invalid id", Nebulite::Constants::Error::NON_CRITICAL);
    }

    mirrorKey = "mirror.renderObject.id" + std::to_string(id);
    return Nebulite::Constants::ErrorTable::NONE();
}

}   // namespace RenderObject
}   // namespace DomainModule
}   // namespace Nebulite