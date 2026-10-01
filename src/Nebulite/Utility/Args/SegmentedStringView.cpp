//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"
#include "Nebulite/Utility/Args/StringIterators.hpp"
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

namespace Nebulite::Utility::Args {

CharacterCount::CharacterCount(std::span<std::string_view const> strings) : count(countCharacters(strings)) {}

CharacterCount::CharacterCount(std::size_t c) : count(c) {}

// TODO: a private constructor where we can just pass charCount could be a good idea
//       Essentially: SegmentedStringView(data.subspan(), charCount - removedViewCharCountIncludingWhitespaces)
//       -> Add an assert inside that constructor that the passed count is equal to charCount(subspan), just in case.
//       This should me subspan creation much faster, as we don't have to iterate over each span member, but instead remove the count from the removed ones.
//       Since we often remove just 1 or 2 members, this offers a nice perf boost.
SegmentedStringView::SegmentedStringView(std::span<std::string_view const> const args) : data(args) {}

SegmentedStringView::SegmentedStringView(std::span<std::string_view const> args, std::size_t const characterCount) : data(args) {
    // The passed character count must match with the expected value
    // This constructor is for internal use only, to avoid recomputing the character count.
    assert(characterCount == countCharacters(args));
    charCount.emplace(characterCount);
}

bool SegmentedStringView::operator==(SegmentedStringView const& other) const {
    if (characterCount() != other.characterCount()) {
        return false;
    }

    // Range position
    auto itA = SpanIterator{data};
    auto itB = SpanIterator{other.data};

    while (!itA.endReached() && !itB.endReached()) {
        if (itA.get() != itB.get()) {
            return false;
        }
        ++itA;
        ++itB;
    }

    // Sanity check: since character count (incl. whitespaces) is the same,
    // we must have reached the end in both strings
    assert(itA.endReached() && itB.endReached());
    return true;
}

bool SegmentedStringView::operator!=(SegmentedStringView const& other) const{
    return !(*this == other); // NOLINT(readability-redundant-parentheses)
}

bool SegmentedStringView::operator==(std::string_view const other) const {
    // Range position
    auto itA = SpanIterator{data};
    auto itB = StringViewIterator{other};

    while (!itA.endReached() && !itB.endReached()) {
        if (itA.get() != itB.get()) {
            return false;
        }
        ++itA;
        ++itB;
    }

    return itA.endReached() && itB.endReached();
}

bool SegmentedStringView::operator!=(std::string_view const other) const{
    return !(*this == other); // NOLINT(readability-redundant-parentheses)
}

std::size_t SegmentedStringView::characterCount() const {
    return charCount.get(data).count;
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

void SegmentedStringView::appendSubspan(std::vector<std::string_view>& other, std::size_t const index) const {
    std::ranges::copy(data.subspan(index), std::back_inserter(other));
}

void SegmentedStringView::appendSubspan(std::vector<std::string_view>& other, std::size_t const startIndex, std::size_t const count) const{
    std::ranges::copy(data.subspan(startIndex, count), std::back_inserter(other));
}

std::string SegmentedStringView::recombine() const {
    // TODO: Once the entire FuncTree class uses SegmentedStringView, we should move the functionality purely into this class
    //       Since recombineArgs only makes sense for a SegmentedStringView at that point, having the implementation in StringHandler feels wrong.
    return StringHandler::recombineArgs(data);
}

} // namespace Nebulite::Utility::Args
