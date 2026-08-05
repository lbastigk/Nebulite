#ifndef NEBULITE_MODULE_RMLUI_CONTEXTMANAGER_HPP
#define NEBULITE_MODULE_RMLUI_CONTEXTMANAGER_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Module/Base/RmlUiModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Graphics {
class RmlInterface;
} // namespace Nebulite::Graphics

namespace Nebulite::Utility::Io {
class Capture;
} // namespace Nebulite::Utility::Io

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class ContextManager final : public Base::RmlUiModule {
public:
    explicit ContextManager(Utility::Io::Capture& c, Graphics::RmlInterface& i);

    void update() override ;
};
} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_CONTEXTMANAGER_HPP
