/*
RenderObjectDraft extends the Global Space Tree to provide an in-memory RenderObject to manipulate and spawn.

It also exposes the Renderobject-Internal functions to the gui via a globally accessible help-function:

./bin/Nebulite help         # As the RenderObjectTree is not a subTree of GlobalSpaceTree, this will NOT show RenderObject specific help.
./bin/Nebulite draft-help   # However, this will!
*/

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"
#include "RenderObject.h"

namespace Nebulite {
class GlobalSpace; // Forward declaration of domain class GlobalSpace 
namespace GlobalSpaceTreeExpansion {
class RenderObjectDraft : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace, RenderObjectDraft> {
public:
    using Wrapper<Nebulite::GlobalSpace, RenderObjectDraft>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Available Functions

    // Prints all Renderobject help information
    Nebulite::ERROR_TYPE mockHelp(int argc, char* argv[]);

    // Parse Renderobject-specific functions on the mock
    Nebulite::ERROR_TYPE onMock(int argc, char* argv[]);

    // Spawn the created mock object
    Nebulite::ERROR_TYPE spawnMock(int argc, char* argv[]);

    // Reset the mock (does not reset any spawned ones!)
    Nebulite::ERROR_TYPE resetMock(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions
    void setupBindings() {
        bindFunction(&RenderObjectDraft::mockHelp,   "draft-help",    "Available functions for the RenderObjectDraft");
        bindFunction(&RenderObjectDraft::onMock,     "on-draft",      "Parse Renderobject-specific functions on the draft");
        bindFunction(&RenderObjectDraft::spawnMock,  "spawn-draft",   "Spawn the created draft object");
        bindFunction(&RenderObjectDraft::resetMock,  "reset-draft",   "Reset the draft object (does not reset any spawned ones!)");
    }

private:
    RenderObject mock;
};
}
}