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

template<StringIteratorLike A, StringIteratorLike B>
bool isEqual(A itA, B itB) {
    while (!itA.endReached() && !itB.endReached()) {
        if (itA.get() != itB.get()) {
            return false;
        }
        ++itA;
        ++itB;
    }
    return itA.endReached() && itB.endReached();
}

template<StringIteratorLike A, StringIteratorLike B>
bool compareUntilOneEnds(A itA, B itB) {
    while (!itA.endReached() && !itB.endReached()) {
        if (itA.get() != itB.get()) {
            return false;
        }
        ++itA;
        ++itB;
    }
    return true;
}

} // namespace

namespace Nebulite::Utility::Args {

CharacterCount::CharacterCount(std::span<std::string_view const> const strings) : count(countCharacters(strings)) {}

CharacterCount::CharacterCount(std::size_t const c) : count(c) {}

SegmentedStringView::SegmentedStringView(std::span<std::string_view const> const args) : data(args) {}

bool SegmentedStringView::operator==(SegmentedStringView const& other) const {
    return isEqual(SpanIterator{data}, SpanIterator{other.data});
}

bool SegmentedStringView::operator!=(SegmentedStringView const& other) const{
    return !(*this == other); // NOLINT(readability-redundant-parentheses)
}

bool SegmentedStringView::operator==(std::string_view const other) const {
    return isEqual(SpanIterator{data}, StringViewIterator{other});
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

bool SegmentedStringView::beginsWith(std::string_view const other) const {
    if (other.size() > characterCount()) {
        return false;
    }
    return compareUntilOneEnds(SpanIterator{data}, StringViewIterator{other});
}

bool SegmentedStringView::beginsWith(SegmentedStringView const& other) const {
    if (other.characterCount() > characterCount()) {
        return false;
    }
    return compareUntilOneEnds(SpanIterator{data}, SpanIterator{other.data});
}

bool SegmentedStringView::endsWith(std::string_view const other) const {
    if (other.size() > characterCount()) {
        return false;
    }
    auto itSelf = SpanIterator{data};
    auto const itOther = StringViewIterator{other};

    auto const sizeDiff = characterCount() - other.size();
    for (std::size_t i = 0; i < sizeDiff; ++i) {
        ++itSelf;
    }
    return isEqual(itOther, itSelf);
}

bool SegmentedStringView::endsWith(SegmentedStringView const& other) const {
    if (other.characterCount() > characterCount()) {
        return false;
    }
    auto itSelf = SpanIterator{data};
    auto const itOther = SpanIterator{other.data};

    auto const sizeDiff = characterCount() - other.characterCount();
    for (std::size_t i = 0; i < sizeDiff; ++i) {
        ++itSelf;
    }
    return isEqual(itOther, itSelf);
}

bool SegmentedStringView::contains(std::string_view const other) const {
    if (other.size() > characterCount()) {
        return false;
    }
    if (other.empty()){
        return true;
    }
    auto itSelf = SpanIterator{data};
    auto const itOther = StringViewIterator{other};
    auto const sizeDiff = characterCount() - other.size();
    if (sizeDiff == 0) {
        return isEqual(itOther, itSelf);
    }
    for (std::size_t i = 0; i <= sizeDiff; ++i) {
        if (compareUntilOneEnds(itOther, itSelf)) {
            return true;
        }
        ++itSelf;
    }
    return false;
}

bool SegmentedStringView::contains(SegmentedStringView const& other) const {
    if (other.characterCount() > characterCount()) {
        return false;
    }
    if (other.empty()) {
        return true;
    }
    auto itSelf = SpanIterator{data};
    auto const itOther = SpanIterator{other.data};
    auto const sizeDiff = characterCount() - other.characterCount();
    if (sizeDiff == 0) {
        return isEqual(itOther, itSelf);
    }
    for (std::size_t i = 0; i <= sizeDiff; ++i) {
        if (compareUntilOneEnds(itOther, itSelf)) {
            return true;
        }
        ++itSelf;
    }
    return false;
}

} // namespace Nebulite::Utility::Args
