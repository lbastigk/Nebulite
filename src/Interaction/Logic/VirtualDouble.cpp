#include "Interaction/Logic/VirtualDouble.hpp"

#include "Nebulite.hpp"

namespace Nebulite::Interaction::Logic {

VirtualDouble::VirtualDouble(std::string k) noexcept : key(std::move(k)) {
    if (key.starts_with(ContextPrefix::self)) { key.erase(0, ContextPrefix::self.size()); } else if (key.starts_with(ContextPrefix::other)) { key.erase(0, ContextPrefix::other.size()); } else
        if (key.starts_with(ContextPrefix::global)) { key.erase(0, ContextPrefix::global.size()); }
}

void VirtualDouble::setUpInternalCache(Data::JSON* json) {
    if (json != nullptr) {
        copied_value = *json->getStableDoublePointer(key);
        reference = &copied_value;
    } else {
        copied_value = *Nebulite::global().getDocCache()->getStableDoublePointer(key);
        reference = &copied_value;
    }
}

void VirtualDouble::setUpExternalCache(Data::JSON* json) {
    if (json != nullptr) {
        reference = json->getStableDoublePointer(key);
    } else {
        reference = Nebulite::global().getDocCache()->getStableDoublePointer(key);
    }
}

} // namespace Nebulite::Interaction::Logic
