//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <string_view>
#include <type_traits>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Logic/LinkedNumericValue.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {

LinkedNumericValue::LinkedNumericValue(std::string_view const k) : key(ContextDeriver::stripContext(k)), scopedKey(key) {
    static_assert(std::is_standard_layout_v<LinkedNumericValue>, "LinkedNumericValue must remain standard-layout because member order is checked with offsetof");
    static_assert(offsetof(LinkedNumericValue, key) < offsetof(LinkedNumericValue, scopedKey), "key must precede scopedKey");
}

void LinkedNumericValue::copyFromJson(Data::JsonScope const& json) {
    copiedValue = json.get<double>(scopedKey).value_or(0.0);
}

void LinkedNumericValue::setDirect(double const val) noexcept {
    copiedValue = val;
}

} // namespace Nebulite::Interaction::Logic
