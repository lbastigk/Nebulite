#pragma once

#include "ErrorTypes.h"
#include "RT_ExpansionWrapper.h"

namespace Nebulite {

// Forward declaration of classes
class Invoke;
class GlobalSpace;

namespace RenderObjectTreeExpansion {
class StateUpdate : public Wrapper<StateUpdate> {
public:
    using Wrapper<StateUpdate>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE deleteObject(int argc, char* argv[]);
    Nebulite::ERROR_TYPE updateText(int argc, char* argv[]);
    Nebulite::ERROR_TYPE reloadInvokes(int argc, char* argv[]);
    Nebulite::ERROR_TYPE addInvoke(int argc, char* argv[]);
    Nebulite::ERROR_TYPE removeInvoke(int argc, char* argv[]);
    Nebulite::ERROR_TYPE removeAllInvokes(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions

    void setupBindings(){
        bindFunction(&StateUpdate::deleteObject, "delete", "Marks object for deletion");
        bindFunction(&StateUpdate::updateText, "update-text", "Calculate text texture");
        bindFunction(&StateUpdate::reloadInvokes, "reload-invokes", "Reload all invokes");
        bindFunction(&StateUpdate::addInvoke, "add-invoke", "Add an invoke from a file");
        bindFunction(&StateUpdate::removeInvoke, "remove-invoke", "Remove an invoke by name");
        bindFunction(&StateUpdate::removeAllInvokes, "remove-all-invokes", "Remove all invokes");
    }
};
}
}