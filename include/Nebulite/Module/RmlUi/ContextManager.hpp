#ifndef NEBULITE_MODULE_RMLUI_CONTEXTMANAGER_HPP
#define NEBULITE_MODULE_RMLUI_CONTEXTMANAGER_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Module/Base/RmlUiModule.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Graphics {
class RmlInterface;
} // namespace Nebulite::Graphics

namespace Nebulite::Utility::IO {
class Capture;
} // namespace Nebulite::Utility::IO

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class ContextManager final : public Base::RmlUiModule {
public:
    explicit ContextManager(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;
};
} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_CONTEXTMANAGER_HPP
