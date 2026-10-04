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

    explicit ArgsTransitionCompatibilityLayer(std::span<std::string_view const> const args) : ssv(args) {}

    template <typename Data, typename Size>
    explicit ArgsTransitionCompatibilityLayer(Data data, Size size) : ssv(data, size) {}

    explicit ArgsTransitionCompatibilityLayer(std::vector<std::string_view> const& argsData) : ssv(argsData) {}

    ~ArgsTransitionCompatibilityLayer() = default;

    ArgsTransitionCompatibilityLayer(ArgsTransitionCompatibilityLayer const &) = default;
    ArgsTransitionCompatibilityLayer(ArgsTransitionCompatibilityLayer &&) = default;
    ArgsTransitionCompatibilityLayer& operator=(ArgsTransitionCompatibilityLayer const& other) = default;
    ArgsTransitionCompatibilityLayer& operator=(ArgsTransitionCompatibilityLayer &&) = default;

    //------------------------------------------
    // Operators

    auto operator[](size_t const index) const {
        return ssv[index];
    }

    //------------------------------------------
    // Range

    auto at(size_t const index) const {
        return ssv[index];
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

    //------------------------------------------
    // Size

    auto empty() const {
        return ssv.empty();
    }

    auto size() const {
        return ssv.size();
    }

    //------------------------------------------
    // Span

    auto subspan(size_t const index) const {
        return ArgsTransitionCompatibilityLayer{ssv.subspan(index).data};
    }

    auto subspan(size_t const index, size_t const count) const {
        return ArgsTransitionCompatibilityLayer{ssv.subspan(index, count).data};
    }

    //------------------------------------------
    // Copy

    void copy(std::vector<std::string_view>& data) const {
        ssv.copy(data);
    }

    void copySubspan(std::vector<std::string_view>& data, size_t const startIndex) const {
        ssv.copySubspan(data, startIndex);
    }

    void copySubspan(std::vector<std::string_view>& data, size_t const startIndex, size_t const count) const {
        ssv.copySubspan(data, startIndex, count);
    }

    //------------------------------------------
    // Generate

    auto recombine() const {
        return ssv.recombine();
    }

    auto recombineSubspan(std::size_t const startIndex) const {
        return ssv.recombineSubspan(startIndex);
    }

    auto recombineSubspan(size_t const startIndex, size_t const count) const {
        return ssv.recombineSubspan(startIndex, count);
    }
};

// Replace with SegmentedStringView later on...
using SSV = ArgsTransitionCompatibilityLayer; //std::span<std::string_view const>; //

} // namespace Nebulite::Utility::Args
#endif // NEBULITE_UTILITY_ARGS_CMDARGS_HPP
