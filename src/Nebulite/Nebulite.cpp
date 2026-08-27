//------------------------------------------
// Includes

// Standard library
#include <stdexcept>
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

Data::JsonScope& shareScope(ScopeAccessor::BaseAccessToken const& at, std::string_view const prefix) {
    // Build full prefix
    if (!at.getPrefix().has_value()) {
        throw std::runtime_error("Nebulite::Global::shareScope: The provided access token does not have a valid prefix.");
    }
    std::string const fullKey = [&] {
        if (at.getPrefix()->empty()) {
            return std::string(prefix);
        }
        if (prefix.starts_with(Data::Json::SpecialCharacter::arrayOpen)) {
            return *at.getPrefix() + std::string(prefix);
        }
        return *at.getPrefix() + "." + std::string(prefix);
    }();
    return globalDoc().shareManagedScope(fullKey);
}

Utility::Io::Capture& capture() {
    return instance().capture;
}

} // namespace Nebulite::Global
