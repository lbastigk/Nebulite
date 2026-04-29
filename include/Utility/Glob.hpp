/**
 * @file Glob.hpp
 * @brief Cross-platform glob pattern matching utility.
 */

#ifndef NEBULITE_UTILITY_GLOB_HPP
#define NEBULITE_UTILITY_GLOB_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#ifdef _WIN32
#include <shlwapi.h>
#else
#include <fnmatch.h>
#endif

//------------------------------------------
namespace Nebulite::Utility {

#ifdef _WIN32

inline bool globMatch(std::string const& pattern, std::string const& str) {
    // TODO: PathMatchSpecA isn't a full glob-pattern-replacement, for example \[1*\] does not work!
    //       For now, we do some pseudo-escaping of brackets, which is likely not the only issue...
    //       This is a hack and should be replaced with a proper implementation if needed
    auto newPattern = Utility::StringHandler::replaceAll(pattern, "\\[", "[");
    newPattern = Utility::StringHandler::replaceAll(newPattern, "\\]", "]");
    return PathMatchSpecA(str.c_str(), newPattern.c_str()) != 0;
}

#else

inline bool globMatch(std::string const& pattern, std::string const& str) {
    return fnmatch(pattern.c_str(), str.c_str(), 0) == 0;
}

#endif

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_GLOB_HPP
