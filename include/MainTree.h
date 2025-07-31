#pragma once

// Basic includes
#include "ErrorTypes.h"         // Basic Return Type: enum ERROR_TYPE
#include "FuncTreeWrapper.h"    // All FuncTrees inherit from this


// Include Subcategories of MainTree
#include "MT_General.h"
#include "MT_Renderer.h"
#include "MT_Debug.h"

namespace Nebulite{

// Forward declaration of classes
class Invoke;
class GlobalSpace;

class MainTree : public FuncTreeWrapper<Nebulite::ERROR_TYPE>{
public:
    MainTree(Nebulite::Invoke* invoke, Nebulite::GlobalSpace* globalSpace);

    // Subclasses are friends of MainTree, allowing access to private members
    friend class Nebulite::MainTreeCategory::General;
    friend class Nebulite::MainTreeCategory::Renderer;
    friend class Nebulite::MainTreeCategory::Debug;

private:

    //---------------------------------------
    // Subclasses of Commands, keeps MainTree clean
    Nebulite::MainTreeCategory::General    general;
    Nebulite::MainTreeCategory::Renderer   renderer;
    Nebulite::MainTreeCategory::Debug      debug;

    //----------------------------------
    // Private Variables for linkage
    Nebulite::Invoke* invoke;
    Nebulite::GlobalSpace* global;
};
}


