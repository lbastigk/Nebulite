//------------------------------------------
// Includes

// Standard library
#include <string>
#include <type_traits>

// Nebulite
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/Json.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/ScopeAccessor.hpp"

//------------------------------------------
namespace {
Nebulite::Data::Json& globalDoc() noexcept(std::is_nothrow_constructible_v<Nebulite::Data::Json>) {
    static Nebulite::Data::Json instance;
    return instance;
}
} // namespace

//------------------------------------------
namespace Nebulite::Global {

Core::GlobalSpace& instance(){
    static Core::GlobalSpace instance{"Nebulite"};
    return instance;
}

Data::JsonScope const& settings() {
    static auto const& settingsScopeConst = globalDoc().shareManagedScope("settings");
    return settingsScopeConst;
}

Data::JsonScope& shareScope(ScopeAccessor::BaseAccessToken const& at, std::string const& prefix) {
    return globalDoc().shareManagedScope(at.getPrefix() + prefix);
}

Utility::Io::Capture& capture() {
    return instance().capture;
}

} // namespace Nebulite::Global
