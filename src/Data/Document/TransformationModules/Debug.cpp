#include "Nebulite.hpp"
#include "Data/Document/TransformationModules/Debug.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::Data::TransformationModules {

void Debug::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Debug::echo, echoName, echoDesc);
    BIND_TRANSFORMATION_STATIC(&Debug::print, printName, printDesc);
}

bool Debug::echo(std::span<std::string const> const& args) {
    // Echo args to cout
    bool first = true;
    std::ranges::for_each(args | std::views::drop(1), [&](auto const& s) {
        if (!first) Log::print(" ");
        first = false;
        Log::print(s);
    });
    Log::println();
    return true;
}

// NOLINTNEXTLINE
bool Debug::print(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    // Print to cout, no modifications
    if (args.size() > 1) {
        Log::println(jsonDoc->serialize(valueKey + args[1]));
    } else {
        Log::println(jsonDoc->serialize());
    }
    return true;
}

} // namespace Nebulite::Data::TransformationModules
