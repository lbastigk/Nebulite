#ifndef NEBULITE_UI_PLUGIN_BASE_HPP
#define NEBULITE_UI_PLUGIN_BASE_HPP

#include <RmlUi/Core.h>

#include "Utility/Capture.hpp"


namespace Nebulite::UI {
// TODO: Rename to Nebulite::UI::RmlModule? But placing the Modules inside Nebulite::UI::RmlModule::MyModule is annoying; having the same name for namespace and class...
class PluginBase : public Rml::Plugin {
public:
    explicit PluginBase(Utility::Capture& c);

    virtual void update();

protected:
    Utility::Capture& capture;
};
} // Namespace
#endif // NEBULITE_UI_PLUGIN_BASE_HPP
