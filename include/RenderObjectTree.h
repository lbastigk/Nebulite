/*
===========================================================
RenderObjectTree – Function Tree for Local RenderObject Logic
===========================================================

This class extends FuncTreeWrapper<ERROR_TYPE> to provide
a focused, self-contained parsing interface (functioncalls)
for `RenderObject`.

Unlike global invoke entries (which allow inter-object logic),
functioncalls in RenderObjectTree operate *exclusively* on
the RenderObject they are attached to (the "self" object).

-----------------------------------------------------------
Why is this layer needed?

Invokes are general-purpose and powerful, but they are best
used for dataflow and inter-object logic. However, some tasks:

  - require more complex conditional logic,
  - are hard to express as value changes alone,
  - need tight control over internal render object state.

RenderObjectTree enables these operations cleanly via keywords
bound to C++ functions, keeping the parsing logic in a separate,
well-scoped layer.

This system allows you to:
  ✓ Simplify invoke rules
  ✓ Remove clutter like `flag_delete` toggles
  ✓ Add utility logic for layout and state changes
  ✓ Log/debug self object behavior locally

-----------------------------------------------------------
Design Constraints:

- All functioncalls operate on `self` (the attached RenderObject)
- No global access (delegated to the threaded Invoke system)
- Values are accessed/updated via `valueGet()` / `valueSet()`
- Logic is meant to be simple, traceable, and local
*/


#pragma once

#include "ErrorTypes.h"
#include "FuncTreeWrapper.h"
#include "FileManagement.h"

// Expansion includes
#include "RTE_Data.h"
#include "RTE_Layout.h"
#include "RTE_Logging.h"
#include "RTE_Parenting.h"
#include "RTE_StateUpdate.h"

namespace Nebulite {
  class RenderObject;  // Forward declaration
}

namespace Nebulite{

class RenderObjectTree : public FuncTreeWrapper<Nebulite::ERROR_TYPE>{
public:
    RenderObjectTree(RenderObject* self);   // Created inside each renderobject, with linkage to the object
private:

    //---------------------------------------
    // Commands to the RenderObjectTree are added via Expansion files to keep the RenderObjectTree clean
    // and allow for easy implementation and removal of collaborative features.
    // Maintainers can separately implement their own features and merge them into the RenderObjectTree.
    //
    // 1.) Create a new Class by inheriting from Nebulite::RenderObjectTreeExpansion::Wrapper ( .h file in ./include and .cpp file in ./src)
    // 2.) Ensure the Class is a friend of Nebulite::GlobalSpace (see GlobalSpace.h)
    // 3.) Implement the setupBindings() method to bind functions
    // 4.) Insert the new object here as a unique pointer
    // 5.) Initialize via make_unique in the MainTree constructor
    //---------------------------------------
    std::unique_ptr<Nebulite::RenderObjectTreeExpansion::Data> data;
    std::unique_ptr<Nebulite::RenderObjectTreeExpansion::Layout> layout;
    std::unique_ptr<Nebulite::RenderObjectTreeExpansion::Logging> logging;
    std::unique_ptr<Nebulite::RenderObjectTreeExpansion::Parenting> parenting;
    std::unique_ptr<Nebulite::RenderObjectTreeExpansion::StateUpdate> stateUpdate;

};
}