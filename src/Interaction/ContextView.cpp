#include "Interaction/Context.hpp"
#include "Interaction/ContextView.hpp"

namespace Nebulite::Interaction {

ContextScopeView::ContextScopeView(ContextScope const& ctxScope) : self(&ctxScope.self), other(&ctxScope.other), global(&ctxScope.global) {}

std::optional<ContextScope> ContextScopeView::rebuild() const {
    if (!self || !other || !global) {
        return std::nullopt; // Invalid view, cannot rebuild
    }
    return ContextScope{
        *self,
        *other,
        *global
    };
}

ContextView::ContextView(Context const& ctx) : self(&ctx.self), other(&ctx.other), global(&ctx.global) {}

std::optional<Context> ContextView::rebuild() const {
    if (!self || !other || !global) {
        return std::nullopt; // Invalid view, cannot rebuild
    }
    return Context{
        *self,
        *other,
        *global
    };
}

} // namespace Nebulite::Interaction
