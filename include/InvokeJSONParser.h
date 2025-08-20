#pragma once
#include "JSON.h"
#include "RenderObject.h"

namespace Nebulite{

class Invoke;

// ==== INVOKE PARSING HELPER FUNCTIONS ====  
class InvokeJSONParser{
public:

    /**
     * @brief Parses a JSON encoded set of Invoke Entries inside a RenderObject into InvokeEntry objects.
     */
    static void parse(
        std::vector<std::shared_ptr<Nebulite::InvokeEntry>>& entries_global, 
        std::vector<std::shared_ptr<Nebulite::InvokeEntry>>& entries_local, 
        Nebulite::RenderObject* self,
        Nebulite::DocumentCache* docCache,
        Nebulite::JSON* global
    );
private:
    static void getFunctionCalls(
        Nebulite::JSON& entryDoc,
        Nebulite::InvokeEntry& invokeEntry, 
        Nebulite::RenderObject* self,
        Nebulite::DocumentCache* docCache,
        Nebulite::JSON* global
    );
    static bool getExpression(
        Nebulite::Assignment& assignmentExpr, 
        Nebulite::JSON& entry, 
        int index
    );
    static std::string getLogicalArg(Nebulite::JSON& entry);
    static bool getInvokeEntry(
        Nebulite::JSON& doc, 
        Nebulite::JSON& entry, 
        int index
    );
};
}

// TODO: Idea for Invoke ruleset overwrites:
// In addition, add the field "overwrites" to the JSON doc:
// "overwrites": {
//     "key1": "value1",
//     "key2": "value2"
// }
// Then, on parsing, the overwrites are applied:
// $(overwrites.key1) would be replaced by "value1"
// If, however, an overwrite is not found:
// $(overwrites.key3) would be replaced by $(global.key3)
// This allows us to flexibly overwrite values in the invoke without changing the original JSON file.
// Also, the behavior is well-defined, as it defaults to the global value if no overwrite is defined.
// Note: retrieval of overwrites in a type object might be difficult. Instead, perhaps:
// myInvoke.jsonc|push-back overwrites 'key1 -> value1'
// Example JSON:
/*
"overwrites": {
    "key1 -> value1",
    "key2 -> value2"
}
*/
// This makes subkey-overwrites easier to parse, e.g.: "overwrites" [ "physics.G -> 9.81" ] 
// turns an "$(overwrites.physics.G)" into "9.81" and 
// defaults to "{global.physics.G}" if not overwritten.