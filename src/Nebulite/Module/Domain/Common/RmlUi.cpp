//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Graphics/RmlUi/Interface.hpp"
#include "Nebulite/Module/Domain/Common/RmlUi.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

RmlUi::~RmlUi(){
    Graphics::RmlUi::Interface::instance().removeReferencesToId(domain.getId());
}

} // namespace Nebulite::Module::Domain::Common
