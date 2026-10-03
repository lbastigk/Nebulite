#ifndef NEBULITE_UTILITY_ARGS_CMDARGS_HPP
#define NEBULITE_UTILITY_ARGS_CMDARGS_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

//------------------------------------------
namespace Nebulite::Utility::Args {

// Replace with SegmentedStringView later on...
using SSV = std::span<std::string_view const>;

} // namespace Nebulite::Utility::Args
#endif // NEBULITE_UTILITY_ARGS_CMDARGS_HPP
