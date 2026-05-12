#ifndef MODULE_RMLUI_CONTEXT_MANAGER_HPP
#define MODULE_RMLUI_CONTEXT_MANAGER_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class ContextManager final : public Base::RmlUiModule {
public:
    explicit ContextManager(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;
};
} // namespace Nebulite::Module::RmlUi
#endif // MODULE_RMLUI_CONTEXT_MANAGER_HPP
