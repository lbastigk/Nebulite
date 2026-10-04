#ifndef NEBULITE_UTILITY_ARGS_CMDARGS_HPP
#define NEBULITE_UTILITY_ARGS_CMDARGS_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef> // NOLINT
#include <span>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"

//------------------------------------------
namespace Nebulite::Utility::Args {

class ArgsTransitionCompatibilityLayer {
    SegmentedStringView ssv;

public:
    ArgsTransitionCompatibilityLayer() = default;

    template <typename Data, typename Size>
    explicit ArgsTransitionCompatibilityLayer(Data data, Size size) : ssv(std::span<std::string_view const>(data, size)) {}

    explicit ArgsTransitionCompatibilityLayer(std::span<std::string_view const> const args) : ssv(args) {}

    //explicit ArgsTransitionCompatibilityLayer(std::span<std::span<std::string_view const>::element_type> const sub) : ssv(sub) {}

    explicit ArgsTransitionCompatibilityLayer(std::vector<std::string_view> const& args) : ssv(std::span(args)) {}

    ~ArgsTransitionCompatibilityLayer() = default;

    ArgsTransitionCompatibilityLayer(ArgsTransitionCompatibilityLayer const &) = default;
    ArgsTransitionCompatibilityLayer(ArgsTransitionCompatibilityLayer &&) = default;

    ArgsTransitionCompatibilityLayer& operator=(ArgsTransitionCompatibilityLayer const& other) = default;
    ArgsTransitionCompatibilityLayer& operator=(ArgsTransitionCompatibilityLayer &&) = default;

    auto operator[](size_t const index) const {
        return ssv[index];
    }

    auto at(size_t const index) const {
        return ssv[index];
    }

    ArgsTransitionCompatibilityLayer subspan(size_t const index) const {
        return ArgsTransitionCompatibilityLayer{ssv.subspan(index).data};
    }

    auto subspan(size_t const index, size_t const count) const {
        return ArgsTransitionCompatibilityLayer{ssv.subspan(index, count).data};
    }

    auto size() const {
        return ssv.segmentCount();
    }

    auto empty() const {
        return ssv.empty();
    }

    auto begin() const {
        return ssv.begin();
    }

    auto end() const {
        return ssv.end();
    }

    auto front() const {
        return ssv.front();
    }

    auto back() const {
        return ssv.back();
    }
};

// Replace with SegmentedStringView later on...
using SSV = std::span<std::string_view const>; // ArgsTransitionCompatibilityLayer; //

} // namespace Nebulite::Utility::Args
#endif // NEBULITE_UTILITY_ARGS_CMDARGS_HPP
