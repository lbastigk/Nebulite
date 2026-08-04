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
    template <typename T> struct mfp_traits; // primary

    template <typename R, typename C, typename... Ps>
    struct mfp_traits<R(C::*)(Ps...)> {
        using return_t = R;
        using class_t = C;
        using params  = std::tuple<Ps...>;
        static constexpr bool is_const = false;
    };

    template <typename R, typename C, typename... Ps>
    struct mfp_traits<R(C::*)(Ps...) const> {
        using return_t = R;
        using class_t = C;
        using params  = std::tuple<Ps...>;
        static constexpr bool is_const = true;
    };

    // Classify function pointers
    template <typename FunctionPointer, typename ReturnValue, typename... AdditionalArgs>
    static constexpr FunctionShape classifyFunctionPtr() {
        using M = std::decay_t<FunctionPointer>;
        using Traits = mfp_traits<M>;
        using C = Traits::class_t;

        using Span = CmdArgs::Span;
        using SpanConstRef = CmdArgs::SpanConstRef;

        // We test with both const and non-const object to support both member types
        using Obj = C&;
        using ConstObj = C const&;

        //------------------------------------------

        if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, int, char const**> ||
                      std::is_invocable_r_v<ReturnValue, M, ConstObj, int, char const**>) {
            return FunctionShape::memberLegacyIntConstChar;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, SpanConstRef, AdditionalArgs...> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj, SpanConstRef, AdditionalArgs...>) {
            return FunctionShape::memberModernFullConstRef;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, Span, AdditionalArgs...> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj, Span, AdditionalArgs...>) {
            return FunctionShape::memberModernFull;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, SpanConstRef> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj, SpanConstRef>) {
            return FunctionShape::memberModernNoAddArgsConstRef;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, Span> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj, Span>) {
            return FunctionShape::memberModernNoAddArgs;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, AdditionalArgs...> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj, AdditionalArgs...>) {
            return FunctionShape::memberNoCmdArgs;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj>) {
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
