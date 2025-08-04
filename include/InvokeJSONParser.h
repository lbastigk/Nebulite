#include "JSON.h"
#include "RenderObject.h"

namespace Nebulite{

class Invoke;

// ==== INVOKE PARSING HELPER FUNCTIONS ====  
class InvokeJSONParser{
public:
    static void parse(
        Nebulite::JSON& doc, 
        std::vector<std::shared_ptr<Nebulite::Invoke::Entry>>& entries_global, 
        std::vector<std::shared_ptr<Nebulite::Invoke::Entry>>& entries_local, 
        Nebulite::RenderObject* self
    );
private:
    static void getFunctionCalls(Nebulite::JSON& entryDoc,Nebulite::Invoke::Entry& invokeEntry);
    static bool getExpression(Nebulite::Invoke::AssignmentExpression& assignmentExpr, Nebulite::JSON& entry, int index);
    static std::string getLogicalArg(Nebulite::JSON& entry);
    static bool getInvokeEntry(Nebulite::JSON& doc, Nebulite::JSON& entry, int index);
};
}