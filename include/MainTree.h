#pragma once

// Basic includes
#include "ErrorTypes.h"         // Basic Return Type: enum ERROR_TYPE
#include "FuncTreeWrapper.h"    // All FuncTrees inherit from this for ease of use


// Include Expansions of MainTree
#include "MTE_General.h"    // General functions like eval, exit, wait, etc.
#include "MTE_Renderer.h"   // Renderer functions for graphics and display
#include "MTE_Debug.h"      // Debugging and logging functions

namespace Nebulite{

// Forward declaration of classes
class Invoke;
class GlobalSpace;
class MainTree : public FuncTreeWrapper<Nebulite::ERROR_TYPE>{
public:
    MainTree(Nebulite::Invoke* invoke, Nebulite::GlobalSpace* globalSpace);

private:

    //---------------------------------------
    // Commands to the MainTree are added via Expansion files to keep the MainTree clean 
    // and allow for easy implementation and removal of collaborative features.
    // Maintainers can separately implement their own features and merge them into the MainTree. 
    //
    // 1.) Create a new category by inheriting from Nebulite::MainTreeExpansion::Wrapper ( .h file in ./include and .cpp file in ./src)
    // 2.) Ensure the category is a friend of Nebulite::GlobalSpace (see GlobalSpace.h)
    // 3.) Implement the setupBindings() method to bind functions
    // 4.) Add the new category object here
    // 5.) Initialize via make_unique in the MainTree constructor
    //---------------------------------------
    std::unique_ptr<Nebulite::MainTreeExpansion::Debug> debug;
    std::unique_ptr<Nebulite::MainTreeExpansion::General> general;
    std::unique_ptr<Nebulite::MainTreeExpansion::Renderer> renderer;
};
}


