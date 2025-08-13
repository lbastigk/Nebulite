#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class RenderObject; // Forward declaration of Domain class RenderObject

namespace RenderObjectTreeExpansion {
class StateUpdate : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::RenderObject, StateUpdate> {
public:
    using Wrapper<Nebulite::RenderObject, StateUpdate>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

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

    void setupBindings() {
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