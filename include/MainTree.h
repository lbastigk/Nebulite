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

// TODO: Template class for MainTreeCategory, with function linkage on construction
// e.g.:
/*
namespace Nebulite::MainTreeCategory{
class Addon : public CategoryWrapper<Nebulite::ERROR_TYPE>{
public:
    Addon(Invoke* invoke, Nebulite::GlobalSpace* globalSpace, FuncTree<ERROR_TYPE>* funcTreePtr) 
        : CategoryWrapper<Nebulite::ERROR_TYPE>("Addon", invoke, globalSpace, funcTreePtr) {
        // Bind functions here
        bindFunction(*funcTree, this, &Addon::addonFunction, "addon_function", "This is an example function in the Addon category");
        //...
    }
}
*/
// This keeps the MainTree clean and allows for easy addition of new categories with minimal modification to MainTree
class MainTree : public FuncTreeWrapper<Nebulite::ERROR_TYPE>{
public:
    MainTree(Nebulite::Invoke* invoke, Nebulite::GlobalSpace* globalSpace);

private:

    //---------------------------------------
    // Subclasses of Commands, keeps MainTree clean
    // 1.) Create a new category by inheriting from Nebulite::MainTreeCategory::Wrapper ( .h and .cpp file)
    // 2.) Ensure the category is a friend of Nebulite::GlobalSpace (see GlobalSpace.h)
    // 3.) Implement the setupBindings() method to bind functions
    // 4.) Add the new category object here
    // 5.) Pass the invoke and globalSpace pointers to the category constructor
    // 6.) Call initialize() inside the MainTree Constructor to bind its functions
    // 7.) Add the cpp file to the CMakeLists.txt
    //---------------------------------------
    Nebulite::MainTreeCategory::Debug      debug;
    Nebulite::MainTreeCategory::General    general;
    Nebulite::MainTreeCategory::Renderer   renderer;
};
}


