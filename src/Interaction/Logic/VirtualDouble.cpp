#include "Interaction/Logic/VirtualDouble.hpp"

#include "Nebulite.hpp"

namespace Nebulite::Interaction::Logic {

VirtualDouble::VirtualDouble(std::string k) noexcept : key(std::move(k)) {
    if (key.starts_with(ContextPrefix::self)) { key.erase(0, ContextPrefix::self.size()); } else if (key.starts_with(ContextPrefix::other)) { key.erase(0, ContextPrefix::other.size()); } else
        if (key.starts_with(ContextPrefix::global)) { key.erase(0, ContextPrefix::global.size()); }
}

void VirtualDouble::setUpInternalCache(Core::JsonScope& json) {
    copied_value = *json.getStableDoublePointer(key);
    reference = &copied_value;
}

void VirtualDouble::setUpInternalCache() {
    copied_value = *Nebulite::global().getDocCache().getStableDoublePointer(key);
    reference = &copied_value;
}

void VirtualDouble::setUpExternalCache(Core::JsonScope& json) {
    reference = json.getStableDoublePointer(key);
}

} // namespace Nebulite::Interaction::Logic
