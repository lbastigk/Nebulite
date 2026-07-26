#ifndef NEBULITE_MODULE_BASE_RULESETMODULECONCEPT_HPP
#define NEBULITE_MODULE_BASE_RULESETMODULECONCEPT_HPP

//------------------------------------------
// Includes

// Standard library
#include <concepts>
#include <cstdint>
#include <type_traits>

// Nebulite
#include "Nebulite/Data/Document/ScopedKeyView.hpp"

//------------------------------------------

template<typename T>
concept ScopedKeyViewArray = requires {
    typename T::value_type;
    requires std::same_as<typename T::value_type, Nebulite::Data::ScopedKeyView>;
};

/**
 * @brief Requirements for a class to be a valid derived RulesetModule
 * @todo Once C++26 is used, add: baseKeys and Key enum must have the same member count
 */
template<typename T>
concept DerivedFromRulesetModule = requires {
    // Has member baseKeys: array of Nebulite::Data::ScopedKeyView
    { T::baseKeys };
    requires ScopedKeyViewArray<decltype(T::baseKeys)>;

    // Has member Key enum with uint8_t underlying type
    typename T::Key;
    requires std::same_as<
        std::underlying_type_t<typename T::Key>,
        std::uint8_t
    >;
};

#endif // NEBULITE_MODULE_BASE_RULESETMODULECONCEPT_HPP
