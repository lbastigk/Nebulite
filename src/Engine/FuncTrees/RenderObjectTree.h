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

namespace Nebulite {
  class RenderObject;  // Forward declaration
}

namespace Nebulite{

class RenderObjectTree : public FuncTreeWrapper<Nebulite::ERROR_TYPE>{
public:
    RenderObjectTree(RenderObject* self);   // Created inside each renderobject, with linkage to the object
private:
    // Pointer to the owning RenderObject
    RenderObject* self;

    //===== Layout & Geometry =====//

    // Aliign text to object dimensions
    Nebulite::ERROR_TYPE align_text(int argc, char* argv[]);  

    // Create text box
    Nebulite::ERROR_TYPE make_box(int argc, char* argv[]);    

    //===== Computation & Internal Updates =====//

    // TODO: DeleteObject doesnt work on Pong test!
    Nebulite::ERROR_TYPE deleteObject(int argc, char* argv[]);

    Nebulite::ERROR_TYPE calculate_text(int argc, char* argv[]);
    Nebulite::ERROR_TYPE recalculate_all(int argc, char* argv[]);
    Nebulite::ERROR_TYPE reload_invokes(int argc, char* argv[]);

    //===== Data management =====//

    // e.g. call function and store result: store tmp.assertResult assert_nonzero keyToCheck
    Nebulite::ERROR_TYPE store(int argc, char* argv[]); 
    
    // move part of json doc from a to b
    Nebulite::ERROR_TYPE move(int argc, char* argv[]);  
    
    // copy part of json doc from a to b
    Nebulite::ERROR_TYPE copy(int argc, char* argv[]);   

    // delete a key from json doc
    Nebulite::ERROR_TYPE keydelete(int argc, char* argv[]);     

    //===== Debugging / Logging =====//

    // Echoing to cout locally. Not recommended for production due to threaded nature of the update loop
    // Only useful for debugging RenderObject logic
    Nebulite::ERROR_TYPE echo(int argc, char* argv[]);
    Nebulite::ERROR_TYPE log(int argc, char* argv[]);
    Nebulite::ERROR_TYPE log_value(int argc, char* argv[]);
    Nebulite::ERROR_TYPE assert_nonzero(int argc, char* argv[]);
    Nebulite::ERROR_TYPE assert_not_in_doc(int argc, char* argv[]);

    // Print all local invokes to stdout for debugging
    Nebulite::ERROR_TYPE print_local_invokes(int argc, char* argv[]);

    //===== Children & Invokes =====//
    Nebulite::ERROR_TYPE addChildren(int argc, char* argv[]);
    Nebulite::ERROR_TYPE removeChildren(int argc, char* argv[]);
    Nebulite::ERROR_TYPE removeAllChildren(int argc, char* argv[]);
    Nebulite::ERROR_TYPE addInvoke(int argc, char* argv[]);
    Nebulite::ERROR_TYPE removeInvoke(int argc, char* argv[]);
    Nebulite::ERROR_TYPE removeAllInvokes(int argc, char* argv[]);
    
    //===== Complex ideas =====//
    Nebulite::ERROR_TYPE sql_call(int argc, char* argv[]);      // idea is to get data from a read-only sql database managed by FileManagement/Invoke
    Nebulite::ERROR_TYPE json_call(int argc, char* argv[]);     // idea is to get data from a read-only json database managed by FileManagement/Invoke
    
};
}