#ifndef NEBULITE_UTILITY_ARGS_CMDARGS_HPP
#define NEBULITE_UTILITY_ARGS_CMDARGS_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

//------------------------------------------
namespace Nebulite::Utility::Args {

// Command argument span types
struct CmdArgs {
    using Span = std::span<std::string_view const>;
    using SpanConstRef = std::span<std::string_view const> const&; // TODO: might be unnecessary
};

} // namespace Nebulite::Utility::Args
#endif // NEBULITE_UTILITY_ARGS_CMDARGS_HPP
