//------------------------------------------
// Includes

// Nebulite
#include "Data/TaskQueue.hpp"
#include "Module/Transformation/Domain.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Domain::bindTransformations(){
    bindTransformation(&Domain::injectScript, injectScriptName, injectScriptDesc);
}

bool Domain::injectScript(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc){
    if (args.size() < 2) return false;
    auto const link = Utility::StringHandler::recombineArgs(args.subspan(1));
    Interaction::Execution::Domain tempDomain("injectScriptTempDomain", *jsonDoc, Global::capture());
    auto ctx = Interaction::Context{tempDomain,tempDomain,tempDomain};
    auto ctxScope = Interaction::ContextScope{*jsonDoc,*jsonDoc,*jsonDoc};
    Data::TaskQueue taskQueue("injectScript");
    taskQueue.addScript(link, Global::capture());
    do {
        taskQueue.resolve(ctx, ctxScope, false);
        taskQueue.decrementWaitCounter();
    } while (taskQueue.isWaiting());
    return true;
}

} // namespace Nebulite::Module::Transformation
