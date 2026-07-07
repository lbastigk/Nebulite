//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Graphics/RmlInterface.hpp"
#include "Nebulite/Module/Domain/Common/RmlUi.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

RmlUi::~RmlUi(){
    Graphics::RmlInterface::instance().removeReferencesToId(domain.getId());
}

} // namespace Nebulite::Module::Domain::Common
