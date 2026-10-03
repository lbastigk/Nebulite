#ifndef NEBULITE_UTILITY_TESTING_HPP
#define NEBULITE_UTILITY_TESTING_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string_view>

// Nebulite
#include "Nebulite/Utility/Io/Capture.hpp"
#include "Nebulite/Utility/StringHandler.hpp"
#include "Nebulite/Utility/Time.hpp"

//------------------------------------------
namespace Nebulite::Utility::Testing {
/**
 * @brief Checks if a condition is true, throws a runtime error with a custom message if not.
 * @tparam MessageArgs The Argument types forming the message
 * @param condition The condition to check
 * @param args The Arguments forming the message
 */
template <typename... MessageArgs>
void assume(bool const condition, MessageArgs&&... args) {
    if (!condition) {
        auto message = StringHandler::combineWithNewline(std::forward<MessageArgs>(args)...);
        throw std::runtime_error(message);
    }
}

template <typename T>
void benchmarkDoNotOptimize(T const& value) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "g"(value) : "memory");
#elifdef _MSC_VER
    (void)value;
    _ReadWriteBarrier();
#else
    volatile T const* sink = &value;
    (void)sink;
#endif
}

template<typename F>
void timeBenchmark(F&& f, std::size_t const n, std::string_view const description, Io::Capture& capture) {
    auto const start = Time::getTime();
    for (std::size_t i = 0; i < n; ++i) {
        benchmarkDoNotOptimize(std::invoke(std::forward<F>(f))); // NOLINT
    }
    auto const duration = Time::getTime() - start;
    capture.log.println("------------------------------------------------------------------------");
    capture.log.println(description," benchmark done.");
    capture.log.println("Took ", duration, "ms for ", n, " iterations. ");
    capture.log.println("About ", static_cast<double>(duration)* 1000.0 * 1000.0 / static_cast<double>(n), "ns per iteration.");
}

} // namespace Nebulite::Utility::Testing
#endif // NEBULITE_UTILITY_TESTING_HPP
