#ifndef NEBULITE_INTERACTION_EXECUTION_CMDARGS_HPP
#define NEBULITE_INTERACTION_EXECUTION_CMDARGS_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

//------------------------------------------
namespace Nebulite::Interaction::Execution {

// Command argument span types
struct CmdArgs {
    using Span = std::span<std::string_view const>;
    using SpanConstRef = std::span<std::string_view const> const&; // TODO: might be unnecessary
};

} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_CMDARGS_HPP
