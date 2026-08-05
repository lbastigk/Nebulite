//------------------------------------------
// Includes

// Standard library
#include <string>

// Nebulite
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/Json.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/ScopeAccessor.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Utility::Io {
class Capture;
} // namespace Nebulite::Utility::Io

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

Utility::Io::Capture& Global::capture() {
    return instance().capture;
}

//------------------------------------------
// Private methods

Data::Json& Global::globalDoc() {
    static Data::Json instance;
    return instance;
}

} // namespace Nebulite
