#ifndef NEBULITE_UTILITY_PROMISE_HPP
#define NEBULITE_UTILITY_PROMISE_HPP

//------------------------------------------
// Includes

// Standard library
#include <type_traits>

//------------------------------------------
namespace Nebulite::Utility {

namespace PromiseType {
struct FunctionVerified{};
struct FunctionNotInUse{};
} // namespace PromiseType

template <typename VerificationType, auto Check>
struct Promise {
    static_assert(
        std::is_same_v<VerificationType, PromiseType::FunctionVerified> || std::is_same_v<VerificationType, PromiseType::FunctionNotInUse>,
        "VerificationType must be either PromiseType::FunctionVerified or PromiseType::FunctionNotInUse."
    );

    // The Check template parameter must either be of type bool or a callable that returns a bool
    static_assert(
        std::is_member_function_pointer_v<decltype(Check)> ||
        std::is_member_object_pointer_v<decltype(Check)>,
        "Check must be a pointer to a member function or a member object."
    );
};

namespace PromiseListPolicy {
struct Any {};
struct All {};
} // namespace PromiseListPolicy

template <typename VerificationType, typename Policy, auto... Checks>
struct PromiseList {
    static_assert(
        std::is_same_v<VerificationType, PromiseType::FunctionVerified> || std::is_same_v<VerificationType, PromiseType::FunctionNotInUse>,
        "VerificationType must be either PromiseType::FunctionVerified or PromiseType::FunctionNotInUse."
    );
    static_assert(
        std::is_same_v<Policy, PromiseListPolicy::Any> || std::is_same_v<Policy, PromiseListPolicy::All>,
        "Policy must be either PromiseListPolicy::Any or PromiseListPolicy::All."
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
