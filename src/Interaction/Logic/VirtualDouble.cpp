//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <type_traits>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Logic/VirtualDouble.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {

VirtualDouble::VirtualDouble(std::string_view const k) : key(ContextDeriver::stripContext(k)), scopedKey(key) {
    static_assert(std::is_standard_layout_v<VirtualDouble>, "VirtualDouble must remain standard-layout because member order is checked with offsetof");
    static_assert(offsetof(VirtualDouble, key) < offsetof(VirtualDouble, scopedKey), "key must precede scopedKey");
}

void VirtualDouble::linkExternalCache(Data::JsonScope const& json) {
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

void VirtualDouble::copyFromJson(Data::JsonScope const& json) {
    copiedValue = json.get<double>(scopedKey).value_or(0.0);
}

void VirtualDouble::setDirect(double const val) noexcept {
    copiedValue = val;
}

} // namespace Nebulite::Interaction::Logic
