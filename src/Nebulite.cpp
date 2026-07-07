//------------------------------------------
// Includes

// Standard library
#include <string>

// Nebulite
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/ScopeAccessor.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite {

Core::GlobalSpace& Global::instance(){
    static Core::GlobalSpace instance{"Nebulite"};
    return instance;
}

Data::JsonScope const& Global::settings() {
    static auto const& settingsScopeConst = globalDoc().shareManagedScope("settings");
    return settingsScopeConst;
}

Data::JsonScope& Global::shareScope(ScopeAccessor::BaseAccessToken const& at, std::string const& prefix) {
    return globalDoc().shareManagedScope(at.getPrefix() + prefix);
}

Utility::IO::Capture& Global::capture() {
    return instance().capture;
}

//------------------------------------------
// Private methods

Data::JSON& Global::globalDoc() {
    static Data::JSON instance;
    return instance;
}

} // namespace Nebulite
