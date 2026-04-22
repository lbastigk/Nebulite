#ifndef NEBULITE_INTERACTION_CONTEXT_VIEW_HPP
#define NEBULITE_INTERACTION_CONTEXT_VIEW_HPP

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

namespace Nebulite::Interaction::Logic {
class Expression; // For Context demotion to ContextScope
} // namespace Nebulite::Interaction::Logic

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

//------------------------------------------
// Includes

// Standard library
#include <optional>

namespace Nebulite::Interaction {

class ContextScopeView {
    Data::JsonScope* self;
    Data::JsonScope* other;
    Data::JsonScope* global;

public:
    ContextScopeView() = default;
    explicit ContextScopeView(ContextScope const& ctxScope);

    [[nodiscard]] std::optional<ContextScope>  rebuild() const ;
};

class ContextView {
    Execution::Domain* self = nullptr;
    Execution::Domain* other = nullptr;
    Execution::Domain* global = nullptr;

public:
    ContextView() = default;
    explicit ContextView(Context const& ctx);

    [[nodiscard]] std::optional<Context> rebuild() const ;
};

} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_CONTEXT_VIEW_HPP
