#pragma once
#include "JSON.h"
#include "RenderObject.h"

namespace Nebulite{

class Invoke;

// ==== INVOKE PARSING HELPER FUNCTIONS ====  
class InvokeJSONParser{
public:
    static void parse(
        Nebulite::JSON& doc, 
        std::vector<std::shared_ptr<Nebulite::InvokeEntry>>& entries_global, 
        std::vector<std::shared_ptr<Nebulite::InvokeEntry>>& entries_local, 
        Nebulite::RenderObject* self,
        Nebulite::DocumentCache* docCache
    );
private:
    static void getFunctionCalls(Nebulite::JSON& entryDoc,Nebulite::InvokeEntry& invokeEntry, Nebulite::DocumentCache* docCache);
    static bool getExpression(Nebulite::InvokeAssignmentExpression& assignmentExpr, Nebulite::JSON& entry, int index);
    static std::string getLogicalArg(Nebulite::JSON& entry);
    static bool getInvokeEntry(Nebulite::JSON& doc, Nebulite::JSON& entry, int index);
};
}