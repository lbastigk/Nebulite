#ifndef UTILITY_FUNCTIONIDENTITY_HPP
#define UTILITY_FUNCTIONIDENTITY_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cstddef>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <type_traits>

//------------------------------------------
namespace Nebulite::Utility {

/**
 * @brief A unique identity to any FunctionPtr
 * @details Allowing for comparison of function pointers of free and member functions.
 */
struct FunctionIdentity {
    static auto constexpr functionPtrSize = sizeof(void*)*2;

    void const* object = nullptr; // nullptr for free/static functions
    std::array<std::byte, functionPtrSize> function{}; // stores function pointer bits

    // Equality
    bool operator==(FunctionIdentity const& other) const {
        return object == other.object &&
               std::memcmp(function.data(), other.function.data(), function.size()) == 0;
    }

    // Check if Fn is of the correct type

    // --- Constructors ---

    // Free/static function
    template<typename Fn>
    explicit FunctionIdentity(Fn fn) {
        // Lambdas / closures are class types
        static_assert(!std::is_class_v<Fn>,
            "Lambdas and functors are not allowed. Extracting a pointer from them is not portable and may lead to collisions. Use a raw function pointer instead."
        );

        // std::function is not allowed
        static_assert(!std::is_same_v<std::remove_cv_t<std::remove_reference_t<Fn>>, std::function<void()>>,
            "std::function objects are not allowed. Use a raw function pointer instead."
        );

        // Must be a raw function pointer
        static_assert(std::is_pointer_v<Fn> && std::is_function_v<std::remove_pointer_t<Fn>>,
            "Only raw function pointers are allowed."
        );

        // Ensure the function fits in storage
        static_assert(sizeof(function) >= sizeof(fn),
            "Function too large to store in FunctionIdentity buffer. Please increase buffer size in FunctionIdentity::function."
        );
        std::memcpy(function.data(), &fn, sizeof(fn));
    }

    // Member function + object
    template<typename Obj, typename MemFn>
    explicit FunctionIdentity(Obj* obj, MemFn memFn) : object(obj) {
        // Ensure the passed obj is not a nullptr
        // As we use "this" inside the constructor of FuncTree to bind functions like "help" and "__complete__", this somehow fails...
        if (obj == nullptr) {
            throw std::invalid_argument(
                "Object pointer cannot be nullptr for member function pointers. "
                "Please use the free/static function constructor for improved safety."
            );
        }

        // Ensure the function fits in storage
        static_assert(sizeof(function) >= sizeof(memFn),
            "Function too large to store in FunctionIdentity buffer. Please increase buffer size in FunctionIdentity::function."
        );
        std::memcpy(function.data(), &memFn, sizeof(memFn));
    }
};

} // namespace Nebulite::Utility
#endif
