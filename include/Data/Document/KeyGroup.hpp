#ifndef NEBULITE_DATA_DOCUMENT_KEY_GROUP_HPP
#define NEBULITE_DATA_DOCUMENT_KEY_GROUP_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/ScopedKey.hpp"

namespace Nebulite::Data {

/**
 * @class KeyGroup
 * @brief A template class to group related keys under a common scope.
 * @details This class allows for defining a group of related keys that share a common scope.
 *          The scope is defined as an optional template parameter.
 *          If the parameter is not provided, the scope is arbitrary and the keys will be non-scoped.
 *          This is useful for organizing keys related to specific domains or modules within the Nebulite framework.
 * @tparam Prefix A string that defines the scope prefix for the keys in this group.
 *                Pass Data::ScopedKey::noScope to indicate that the keys in this group are not scoped and can be used in any scope.
 *                Note that there is a difference between a root scope (Prefix = "") and no scope (Prefix = Data::ScopedKey::noScope):
 *                - a root scope means the jsonScope we access with the key must also be at the root, otherwise access will fail
 *                - no scope means the jsonScope may be at any scope, and the key will be accessed at the root of that scope,
 *                  meaning it can be used in any scope without failing. Of course, if the jsonScope is not at the expected scope, the retrieval may return an error.
 */
template <OptionalFixedString Prefix>
class KeyGroup {
public:
    static consteval auto makeScoped(const char* keyStr) {
        return ScopedKeyView::createFromOptionalFixedString<Prefix>(keyStr);
    }

    static constexpr auto getScope() {
        if constexpr (Prefix.has_scope) {
            return std::string_view(Prefix.value, [] (const char* p) constexpr {
                std::size_t i = 0;
                while (p[i] != '\0') ++i;
                return i;
            }(Prefix.value));
        } else {
            return std::nullopt;
        }
    }

    static auto constexpr hasScope() {
        return Prefix.has_scope;
    }
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_KEY_GROUP_HPP
