#include "DomainModule/Renderer/RenderObjectDraft.hpp"
#include "Core/Renderer.hpp"

namespace Nebulite::DomainModule::Renderer {

Constants::Error RenderObjectDraft::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error RenderObjectDraft::draft_parse(int argc, char** argv) {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string const command = Utility::StringHandler::recombineArgs(argc - 1, argv + 1);
    return draft.get()->parseStr(__FUNCTION__ + std::string(" ") + command);
}

// NOLINTNEXTLINE
Constants::Error RenderObjectDraft::draft_spawn(int argc, char** argv) {
    if (argc != 1) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }
    // Make a copy of the draft's serialized data
    // Create a new RenderObject on the heap and append it to the renderer
    std::string const serial = draft.get()->serialize();
    auto* newObj = new Core::RenderObject();
    newObj->deserialize(serial);
    domain->append(newObj);
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error RenderObjectDraft::draft_reset(int argc, char** argv) {
    if (argc != 1) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }
    Core::RenderObject newDraft;
    draft.get()->deserialize(newDraft.serialize());
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::GlobalSpace
