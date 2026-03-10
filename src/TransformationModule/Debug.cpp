#include "Nebulite.hpp"
#include "Data/Document/JsonScope.hpp"
#include "TransformationModule/Debug.hpp"


namespace Nebulite::TransformationModule {

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
bool Debug::print(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    // Print to cout, no modifications
    if (args.size() > 1) {
        for (auto const& arg : args | std::views::drop(1)) {
            if (std::string const value = jsonDoc->serialize(rootKey + arg); value.ends_with('\n')) {
                Log::print(value);
            } else {
                Log::println(value);
            }
        }
    } else {
        if (std::string const value = jsonDoc->serialize(); value.ends_with('\n')) {
            Log::print(value);
        } else {
            Log::println(value);
        }
    }
    return true;
}

} // namespace Nebulite::TransformationModule
