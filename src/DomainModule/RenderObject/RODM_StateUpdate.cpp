#include "DomainModule/RenderObject/RODM_StateUpdate.hpp"
#include "Core/RenderObject.hpp"

namespace Nebulite::DomainModule::RenderObject{

//------------------------------------------
// Update
Nebulite::Constants::Error StateUpdate::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error StateUpdate::func_delete(int argc,  char* argv[]){
    domain->flag.deleteFromScene = true;
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const StateUpdate::func_delete_name = "delete";
std::string const StateUpdate::func_delete_desc = R"(Marks object for deletion

Usage: delete

Marks the object for deletion on the next update cycle.
)";

Nebulite::Constants::Error StateUpdate::updateText(int argc,  char* argv[]){
    domain->flag.calculateText = true;
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const StateUpdate::updateText_name = "update-text";
std::string const StateUpdate::updateText_desc = R"(Calculate text texture

Usage: update-text

Triggers a recalculation of the text texture.
)";

Nebulite::Constants::Error StateUpdate::invoke_reload(int argc,  char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
std::string const StateUpdate::invoke_reload_name = "reload-invokes";
std::string const StateUpdate::invoke_reload_desc = R"(TODO: Not implemented yet)";

Nebulite::Constants::Error StateUpdate::invoke_add(int argc,  char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
std::string const StateUpdate::invoke_add_name = "add-invoke";
std::string const StateUpdate::invoke_add_desc = R"(TODO: Not implemented yet)";

Nebulite::Constants::Error StateUpdate::invoke_remove(int argc,  char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
std::string const StateUpdate::invoke_remove_name = "remove-invoke";
std::string const StateUpdate::invoke_remove_desc = R"(TODO: Not implemented yet)";

Nebulite::Constants::Error StateUpdate::invoke_purge(int argc,  char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
std::string const StateUpdate::invoke_purge_name = "remove-all-invokes";
std::string const StateUpdate::invoke_purge_desc = R"(TODO: Not implemented yet)";

} // namespace Nebulite::DomainModule::RenderObject