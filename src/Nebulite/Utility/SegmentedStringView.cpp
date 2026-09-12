//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <array>
#include <cassert>
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
    auto const whitespaceCount = strings.empty() ? 0 : strings.size() - 1;
    return whitespaceCount + std::ranges::fold_left(
        strings,
        std::size_t{0},
        [](std::size_t const acc, std::string_view const s) {
            return acc + s.size();
        }
    );
}

} // namespace

namespace Nebulite::Utility {

// TODO: a private constructor where we can just pass charCount could be a good idea
//       Essentially: SegmentedStringView(data.subspan(), charCount - removedViewCharCountIncludingWhitespaces)
//       -> Add an assert inside that constructor that the passed count is equal to charCount(subspan), just in case.
//       This should me subspan creation much faster, as we don't have to iterate over each span member, but instead remove the count from the removed ones.
//       Since we often remove just 1 or 2 members, this offers a nice perf boost.
SegmentedStringView::SegmentedStringView(std::span<std::string_view const> const sv) : data(sv), charCount(countCharacters(sv)) {}

bool SegmentedStringView::operator==(SegmentedStringView const& other) const{
    if (charCount != other.charCount) {
        return false;
    }

    // Range position
    auto itA = data.begin();
    auto itB = other.data.begin();

    // Iterator advancement checker
    bool advancedA = false;
    bool advancedB = false;

    // Char position in string_view
    std::size_t posA = 0;
    std::size_t posB = 0;

    // Compare each character
    while (itA != data.end() && itB != other.data.end()) {
        // Check if segment end is reached
        if (posA == itA->size()) {
            ++itA;
            posA = 0;
            advancedA = true;
            continue;
        }
        if (posB == itB->size()) {
            ++itB;
            posB = 0;
            advancedB = true;
            continue;
        }

        // Compare character (either from itX or the whitespace inbetween each view)
        char const currentA = advancedA ? ' ' : (*itA)[posA];
        char const currentB = advancedB ? ' ' : (*itB)[posB];
        if (currentA != currentB) { // NOLINT(readability-redundant-parentheses)
            return false;
        }

        // Increment position
        if (!advancedA) {
            ++posA;
        }
        if (!advancedB) {
            ++posB;
        }
        advancedA = false;
        advancedB = false;
    }
    assert(itA == data.end() && itB == other.data.end());
    return true;
}

bool SegmentedStringView::operator!=(SegmentedStringView const& other) const{
    return !(*this == other); // NOLINT(readability-redundant-parentheses)
}

// array init likely unnecessary, direct comparison would be faster
bool SegmentedStringView::operator==(std::string_view const other) const {
    std::array const otherData = {other};
    return *this == SegmentedStringView{std::span<std::string_view const>{otherData}};
}

bool SegmentedStringView::operator!=(std::string_view const other) const{
    return !(*this == other); // NOLINT(readability-redundant-parentheses)
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
    // TODO: Once the entire FuncTree class uses SegmentedStringView, we should move the functionality purely into this class
    //       Since recombineArgs only makes sense for a SegmentedStringView at that point, having the implementation in StringHandler feels wrong.
    return StringHandler::recombineArgs(data);
}

} // namespace Nebulite::Utility
