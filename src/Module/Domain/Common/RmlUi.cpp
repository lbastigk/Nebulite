//------------------------------------------
// Includes

// Nebulite
#include "Module/Domain/Common/RmlUi.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

RmlUi::~RmlUi(){
    Graphics::RmlInterface::instance().removeReferencesToId(domain.getId());
}

} // namespace Nebulite::Module::Domain::Common
