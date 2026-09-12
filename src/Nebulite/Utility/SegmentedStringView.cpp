//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Utility/SegmentedStringView.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------

namespace {

std::size_t countCharacters(std::span<std::string_view const> strings) {
    // TODO: Consider character between each data point?
    return std::ranges::fold_left(
        strings,
        std::size_t{0},
        [](std::size_t const acc, std::string_view const s) {
            return acc + s.size();
        }
    );
}

} // namespace

namespace Nebulite::Utility {

SegmentedStringView::SegmentedStringView(std::span<std::string_view const> const sv) : data(sv), charCount(countCharacters(sv)) {}

bool SegmentedStringView::operator==(SegmentedStringView const& other) const{
    if (charCount != other.charCount) {
        return false;
    }

    // Range position
    auto itA = data.begin();
    auto itB = other.data.begin();

    // Char position in string_view
    std::size_t posA = 0;
    std::size_t posB = 0;

    while (itA != data.end() && itB != other.data.end()) {
        // Check if segment end is reached
        if (posA == itA->size()) {
            ++itA;
            posA = 0;
            continue;
        }
        if (posB == itB->size()) {
            ++itB;
            posB = 0;
            continue;
        }

        if ((*itA)[posA] != (*itB)[posB]) {
            return false;
        }
        ++posA;
        ++posB;
    }

    //return itA == data.end() && itB == other.data.end(); // Since charCount must be equal, this is unnecessary
    return true;
}

bool SegmentedStringView::operator!=(SegmentedStringView const& other) const{
    return !(*this == other);
}

// array init likely unnecessary, direct comparison would be faster
bool SegmentedStringView::operator==(std::string_view const other) const {
    std::array const otherData = {other};
    return *this == SegmentedStringView{std::span<std::string_view const>{otherData}};
}

bool SegmentedStringView::operator!=(std::string_view const other) const{
    return !(*this == other);
}

std::size_t SegmentedStringView::charactorCount() const {
    return charCount;
}

std::size_t SegmentedStringView::segmentCount() const {
    return data.size();
}

SegmentedStringView SegmentedStringView::subspan(std::size_t const index) const {
    return SegmentedStringView(data.subspan(index));
}
SegmentedStringView SegmentedStringView::subspan(std::size_t const startIndex, std::size_t const count) const {
    return SegmentedStringView(data.subspan(startIndex, count));
}

std::string SegmentedStringView::recombine() const {
    return StringHandler::recombineArgs(data);
}

} // namespace Nebulite::Utility
