#ifndef NEBULITE_UTILITY_PROMISE_HPP
#define NEBULITE_UTILITY_PROMISE_HPP

//------------------------------------------
// Includes

// Standard library
#include <type_traits>

//------------------------------------------
namespace Nebulite::Utility {
template <auto Check>
struct Promise {
    // The Check template parameter must either be of type bool or a callable that returns a bool
    static_assert(
        std::is_member_function_pointer_v<decltype(Check)> ||
        std::is_member_object_pointer_v<decltype(Check)>,
        "Check must be a pointer to a member function or a member object."
    );
};

struct Any {};
struct All {};

template <typename Policy, auto... Checks>
struct PromiseList {
    static_assert(
        std::is_same_v<Policy, Any> || std::is_same_v<Policy, All>,
        "Policy must be either Any or All."
    );
    static_assert(
        (... && (
            std::is_member_function_pointer_v<decltype(Checks)> ||
            std::is_member_object_pointer_v<decltype(Checks)>
        )),
        "Each check must be a pointer to a member function or member object."
    );
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_PROMISE_HPP
