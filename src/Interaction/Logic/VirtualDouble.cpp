#include "Interaction/Logic/VirtualDouble.hpp"

#include "Nebulite.hpp"

namespace Nebulite::Interaction::Logic {

VirtualDouble::VirtualDouble(std::string k) noexcept : key(std::move(k)) {
    if (key.starts_with(ContextPrefix::self)) {
        key.erase(0, ContextPrefix::self.size());
    }
    else if (key.starts_with(ContextPrefix::other)) {
        key.erase(0, ContextPrefix::other.size());
    }
    else if (key.starts_with(ContextPrefix::global)) {
        key.erase(0, ContextPrefix::global.size());
    }
    scopedKey = Data::ScopedKey(key);
}

void VirtualDouble::linkExternalCache(Data::JsonScopeBase const& json){
    if (externalReference != nullptr) {
        // This function should only be called once, throw error
        throw std::logic_error("External cache is already linked for key: " + key);
    }
    externalReference = json.getStableDoublePointer(scopedKey);
}

void VirtualDouble::copyExternalCache() {
    if (externalReference == nullptr) {
        // This function should only be called after linking, throw error
        throw std::logic_error("External cache is not linked for key: " + key);
    }
    copiedValue = *externalReference;
}

void VirtualDouble::copyFromJson(Data::JsonScopeBase const& json) {
    copiedValue = json.get<double>(scopedKey, 0.0);
}

void VirtualDouble::setDirect(double const& val) noexcept {
    copiedValue = val;
}

} // namespace Nebulite::Interaction::Logic
