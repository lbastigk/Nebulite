#ifndef NEBULITE_UTILITY_ARGS_SHAPECLASSIFIER_HPP
#define NEBULITE_UTILITY_ARGS_SHAPECLASSIFIER_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <tuple>
#include <type_traits>

// Nebulite
#include "Nebulite/Utility/Args/CmdArgs.hpp"
#include "Nebulite/Utility/CompileTimeEvaluate.hpp"

//------------------------------------------
namespace Nebulite::Utility::Args {
/**
 * @class ShapeClassifier
 * @brief Helper class for FuncTree to classify function pointer shapes at compile time.
 */
class ShapeClassifier {
public:
    enum class FunctionShape : std::uint8_t {
        unknown,

        // Member shapes
        memberLegacyIntConstChar,
        memberModernNoAddArgs,
        memberModernNoAddArgsConstRef,
        memberModernFull,
        memberModernFullConstRef,
        memberNoArgs,
        memberNoCmdArgs,

        // Free / static shapes
        freeLegacyIntChar,
        freeLegacyIntConstChar,
        freeModernNoAddArgs,
        freeModernNoAddArgsConstRef,
        freeModernFull,
        freeModernFullConstRef,
        freeNoArgs,
        freeNoCmdArgs,
    };

    // Extract return, class and parameter list from member-function pointer types
    template <typename T> struct MemberFunctionPointerTraits; // primary

    template <typename R, typename C, typename... Ps>
    struct MemberFunctionPointerTraits<R(C::*)(Ps...)> {
        using ReturnType = R;
        using ClassType = C;
        using Params  = std::tuple<Ps...>;
        static constexpr bool isConst = false;
    };

    template <typename R, typename C, typename... Ps>
    struct MemberFunctionPointerTraits<R(C::*)(Ps...) const> {
        using ReturnType = R;
        using ClassType = C;
        using Params  = std::tuple<Ps...>;
        static constexpr bool isConst = true;
    };

    // Helper to shorten the if-constexpr chain if both C& and C const& are checked
    template<typename ReturnValue, typename M, typename C, typename ... Args>
    static bool constexpr isInvocableWithArgs = std::is_invocable_r_v<ReturnValue, M, C&, Args...> || std::is_invocable_r_v<ReturnValue, M, C const&, Args...>;

    // Helper to shorten the if-constexpr chain if both C& and C const& are checked
    template<typename ReturnValue, typename M, typename C>
    static bool constexpr isInvocableWithoutArgs = std::is_invocable_r_v<ReturnValue, M, C&> || std::is_invocable_r_v<ReturnValue, M, C const&>;

    // Classify function pointers
    template <typename FunctionPointer, typename ReturnValue, typename... AdditionalArgs>
    static constexpr FunctionShape classifyFunctionPtr() {
        using M = std::decay_t<FunctionPointer>;
        using Traits = MemberFunctionPointerTraits<M>;
        using C = Traits::ClassType;
        using Span = CmdArgs::Span;
        using SpanConstRef = CmdArgs::SpanConstRef;

        // Determine type
        if constexpr (isInvocableWithArgs<ReturnValue, M, C, int, char const**>) {
            return FunctionShape::memberLegacyIntConstChar;
        }
        else if constexpr (isInvocableWithArgs<ReturnValue, M, C, SpanConstRef, AdditionalArgs...>) {
            return FunctionShape::memberModernFullConstRef;
        }
        else if constexpr (isInvocableWithArgs<ReturnValue, M, C, Span, AdditionalArgs...>) {
            return FunctionShape::memberModernFull;
        }
        else if constexpr (isInvocableWithArgs<ReturnValue, M, C, SpanConstRef>) {
            return FunctionShape::memberModernNoAddArgsConstRef;
        }
        else if constexpr (isInvocableWithArgs<ReturnValue, M, C, Span>) {
            return FunctionShape::memberModernNoAddArgs;
        }
        else if constexpr (isInvocableWithArgs<ReturnValue, M, C, AdditionalArgs...>) {
            return FunctionShape::memberNoCmdArgs;
        }
        else if constexpr (isInvocableWithoutArgs<ReturnValue, M, C>) {
            return FunctionShape::memberNoArgs;
        }
        else {
            return FunctionShape::unknown;
        }
    }

    // Classify free/static function pointers
    template <typename FunctionPointer, typename ReturnValue, typename... AdditionalArgs>
    static constexpr FunctionShape classifyFreeFunction() {
        using F = std::decay_t<FunctionPointer>;
        using Span = CmdArgs::Span;
        using SpanConstRef = CmdArgs::SpanConstRef;

        // Determine type
        if constexpr (std::is_invocable_r_v<ReturnValue, F, int, char**>) {
            return FunctionShape::freeLegacyIntChar;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, int, char const**>) {
            return FunctionShape::freeLegacyIntConstChar;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, SpanConstRef, AdditionalArgs...>) {
            return FunctionShape::freeModernFullConstRef;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, Span, AdditionalArgs...>) {
            return FunctionShape::freeModernFull;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, SpanConstRef>) {
            return FunctionShape::freeModernNoAddArgsConstRef;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, Span>) {
            return FunctionShape::freeModernNoAddArgs;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, AdditionalArgs...>) {
            return FunctionShape::freeNoCmdArgs;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F>) {
            return FunctionShape::freeNoArgs;
        }
        else {
            return FunctionShape::unknown;
        }
    }

    // Unified classifier that dispatches based on pointer category
    template <typename FunctionPointer, typename ReturnValue, typename... AdditionalArgs>
    static constexpr FunctionShape classifyFunction() {
        if constexpr (std::is_member_function_pointer_v<FunctionPointer>) {
            return classifyFunctionPtr<FunctionPointer, ReturnValue, AdditionalArgs...>();
        } else if constexpr (std::is_pointer_v<FunctionPointer> &&
                             std::is_function_v<std::remove_pointer_t<FunctionPointer>>) {
            return classifyFreeFunction<FunctionPointer, ReturnValue, AdditionalArgs...>();
        } else {
            static_assert(CompileTimeEvaluate::alwaysFalse(), "classifyFunction received an unsupported function pointer type.");
            return FunctionShape::unknown;
        }
    }
};
} // namespace Nebulite::Utility::Args
#endif // NEBULITE_UTILITY_ARGS_SHAPECLASSIFIER_HPP
