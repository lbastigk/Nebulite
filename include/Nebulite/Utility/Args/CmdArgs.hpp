#ifndef NEBULITE_UTILITY_ARGS_CMDARGS_HPP
#define NEBULITE_UTILITY_ARGS_CMDARGS_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef> // NOLINT
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"

//------------------------------------------
namespace Nebulite::Utility::Args {

class ArgsTransitionCompatibilityLayer {
    SegmentedStringView ssv;
public:
    explicit ArgsTransitionCompatibilityLayer(std::span<std::string_view> const args) : ssv(args) {}

    auto& operator[](size_t const index) const {
        return ssv[index];
    }

    auto subspan(size_t const index) const {
        return ssv.subspan(index);
    }

    auto subspan(size_t const index, size_t const count) const {
        return ssv.subspan(index, count);
    }

    auto size() const {
        return ssv.segmentCount();
    }

    auto empty() const {
        return ssv.empty();
    }
};

// Replace with SegmentedStringView later on...
using SSV = std::span<std::string_view const>;

} // namespace Nebulite::Utility::Args
#endif // NEBULITE_UTILITY_ARGS_CMDARGS_HPP
