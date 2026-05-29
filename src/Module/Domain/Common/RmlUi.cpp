//------------------------------------------
// Includes

// Nebulite
#include "Graphics/RmlInterface.hpp"
#include "Module/Domain/Common/RmlUi.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

RmlUi::~RmlUi(){
    Graphics::RmlInterface::instance().removeReferencesToId(domain.getId());
}

} // namespace Nebulite::Module::Domain::Common
