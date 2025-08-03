/*
===========================================================
MainTree - Function Tree for JSON Modification
===========================================================

This class extends FuncTreeWrapper<ERROR_TYPE> to provide
a focused, self-contained parsing interface (functioncalls)
for Nebulite's JSON data container.
*/

#pragma once

//----------------------------------------------------------
// Basic includes
#include "ErrorTypes.h"         // Basic Return Type: enum ERROR_TYPE
#include "FuncTreeWrapper.h"    // All FuncTrees inherit from this for ease of use

namespace Nebulite{
class JSONTree : public FuncTreeWrapper<Nebulite::ERROR_TYPE> {
public:
    JSONTree();
private:

};
}