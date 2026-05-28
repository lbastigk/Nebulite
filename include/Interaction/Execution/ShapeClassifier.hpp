#ifndef INTERACTION_EXECUTION_SHAPECLASSIFIER_HPP
#define INTERACTION_EXECUTION_SHAPECLASSIFIER_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint>

// Nebulite
#include "Utility/CompileTimeEvaluate.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Execution {
/**
 * @class ShapeClassifier
 * @brief Helper class for FuncTree to classify function pointer shapes at compile time.
 */
class ShapeClassifier {
public:
    enum class FunctionShape : uint8_t {
        Unknown,

        // Member shapes
        Member_Legacy_IntConstChar,

        Member_Modern_NoAddArgs,
        Member_Modern_NoAddArgsConstRef,

        Member_Modern_Full,
        Member_Modern_FullConstRef,

        Member_NoArgs,
        Member_NoCmdArgs,

        // Free / static shapes
        Free_Legacy_IntChar,
        Free_Legacy_IntConstChar,

        Free_Modern_NoAddArgs,
        Free_Modern_NoAddArgsConstRef,

        Free_Modern_Full,
        Free_Modern_FullConstRef,

        Free_NoArgs,
        Free_NoCmdArgs
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

        using Span = FuncTree<ReturnValue, AdditionalArgs...>::CmdArgs::Span;
        using SpanConstRef = FuncTree<ReturnValue, AdditionalArgs...>::CmdArgs::SpanConstRef;

        // We test with both const and non-const object to support both member types
        using Obj = C&;
        using ConstObj = const C&;

        //------------------------------------------

        if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, int, char const**> ||
                      std::is_invocable_r_v<ReturnValue, M, ConstObj, int, char const**>) {
            return FunctionShape::Member_Legacy_IntConstChar;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, SpanConstRef, AdditionalArgs...> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj, SpanConstRef, AdditionalArgs...>) {
            return FunctionShape::Member_Modern_FullConstRef;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, Span, AdditionalArgs...> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj, Span, AdditionalArgs...>) {
            return FunctionShape::Member_Modern_Full;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, SpanConstRef> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj, SpanConstRef>) {
            return FunctionShape::Member_Modern_NoAddArgsConstRef;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, Span> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj, Span>) {
            return FunctionShape::Member_Modern_NoAddArgs;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj, AdditionalArgs...> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj, AdditionalArgs...>) {
            return FunctionShape::Member_NoCmdArgs;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, M, Obj> ||
                           std::is_invocable_r_v<ReturnValue, M, ConstObj>) {
            return FunctionShape::Member_NoArgs;
        }
        else {
            return FunctionShape::Unknown;
        }
    }

    // Classify free/static function pointers
    template <typename FunctionPointer, typename ReturnValue, typename... AdditionalArgs>
    static constexpr FunctionShape classifyFreeFunction() {
        using F = std::decay_t<FunctionPointer>;
        using Span = FuncTree<ReturnValue, AdditionalArgs...>::CmdArgs::Span;
        using SpanConstRef = FuncTree<ReturnValue, AdditionalArgs...>::CmdArgs::SpanConstRef;

        if constexpr (std::is_invocable_r_v<ReturnValue, F, int, char**>) {
            return FunctionShape::Free_Legacy_IntChar;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, int, char const**>) {
            return FunctionShape::Free_Legacy_IntConstChar;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, SpanConstRef, AdditionalArgs...>) {
            return FunctionShape::Free_Modern_FullConstRef;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, Span, AdditionalArgs...>) {
            return FunctionShape::Free_Modern_Full;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, SpanConstRef>) {
            return FunctionShape::Free_Modern_NoAddArgsConstRef;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, Span>) {
            return FunctionShape::Free_Modern_NoAddArgs;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F, AdditionalArgs...>) {
            return FunctionShape::Free_NoCmdArgs;
        }
        else if constexpr (std::is_invocable_r_v<ReturnValue, F>) {
            return FunctionShape::Free_NoArgs;
        }
        else {
            return FunctionShape::Unknown;
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
            static_assert(Utility::CompileTimeEvaluate::always_false(), "classifyFunction received an unsupported function pointer type.");
            return FunctionShape::Unknown;
        }
    }
};
} // namespace Nebulite::Interaction::Execution
#endif // INTERACTION_EXECUTION_SHAPECLASSIFIER_HPP
