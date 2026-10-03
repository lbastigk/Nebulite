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
// Defines

/**
 * @brief Will use memcmp if contiguous memory is available
 * @details Still slower than character-by-character-comparisons.
 */
//#define NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_COMPARE_BATCHED

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
bool constexpr bothAreStringViewIterators() {
    return std::is_same_v<A, Nebulite::Utility::Args::StringViewIterator>
        && std::is_same_v<B, Nebulite::Utility::Args::StringViewIterator>;
}

template<StringIteratorLike A, StringIteratorLike B>
bool isEqual(A itA, B itB) {
    if constexpr (bothAreStringViewIterators<A, B>()) {
        auto lA = itA.contiguousMemoryAvailable();
        auto lB = itB.contiguousMemoryAvailable();
        if (lA != lB) {
            return false;
        }
        return std::__memcmp(itA.contiguousData(), itB.contiguousData(), lA) == 0;
    }
    else {
        while (!itA.endReached() && !itB.endReached()) {
#ifdef NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_COMPARE_BATCHED
            auto n = std::min(
                itA.contiguousMemoryAvailable(),
                itB.contiguousMemoryAvailable()
            );
            if (n > 0) {
                if (std::__memcmp(itA.contiguousData(), itB.contiguousData(), n) != 0) {
                    return false;
                }
                itA.advanceBy(n);
                itB.advanceBy(n);
            }
            else {
#endif // NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_COMPARE_BATCHED
                if (itA.get() != itB.get()) {
                    return false;
                }
                ++itA;
                ++itB;
#ifdef NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_COMPARE_BATCHED
            }
#endif // NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_COMPARE_BATCHED
        }
        return itA.endReached() && itB.endReached();
    }
}

template<StringIteratorLike A, StringIteratorLike B>
bool compareUntilOneEnds(A itA, B itB) {
    if constexpr (bothAreStringViewIterators<A, B>()) {
        auto n = std::min(
            itA.contiguousMemoryAvailable(),
            itB.contiguousMemoryAvailable()
        );
        return std::__memcmp(itA.contiguousData(), itB.contiguousData(), n) == 0;
    }
    else {
        while (!itA.endReached() && !itB.endReached()) {
#ifdef NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_COMPARE_BATCHED
            auto n = std::min(
                itA.contiguousMemoryAvailable(),
                itB.contiguousMemoryAvailable()
            );
            if (n > 0) {
                if (std::__memcmp(itB.contiguousData(), itA.contiguousData(), n) != 0) {
                    return false;
                }
                itA.advanceBy(n);
                itB.advanceBy(n);
            }
            else {
#endif // NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_COMPARE_BATCHED
                if (itA.get() != itB.get()) {
                    return false;
                }
                ++itA;
                ++itB;
#ifdef NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_COMPARE_BATCHED
            }
#endif // NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_COMPARE_BATCHED
        }
        return true;
    }
}

} // namespace

namespace Nebulite::Utility::Args {

//------------------------------------------
// CharacterCount helper struct

CharacterCount::CharacterCount(std::span<std::string_view const> const strings) : count(countCharacters(strings)) {}

CharacterCount::CharacterCount(std::size_t const c) : count(c) {}

//------------------------------------------
// Constructor etc.

SegmentedStringView::SegmentedStringView() = default;

SegmentedStringView::SegmentedStringView(std::span<std::string_view const> const args) : data(args) {}

//------------------------------------------
// Operators

bool SegmentedStringView::operator==(SegmentedStringView const& other) const {
    if (data.size() != other.data.size()) {
        return false;
    }
    for (std::size_t index = 0; index < data.size(); ++index) {
        assert(!data[index].contains(' ')  && "All strings must be split properly by whitespaces!");
        assert(!other.data[index].contains(' ') && "All strings must be split properly by whitespaces!");
        if (data[index] != other[index]) {
            return false;
        }
    }
    return true;
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

[[nodiscard]] decltype(SegmentedStringView::data[0])& SegmentedStringView::operator[](std::size_t const index) const {
    return data[index];
}

//------------------------------------------
// Range

[[nodiscard]] decltype(SegmentedStringView::data.begin()) SegmentedStringView::begin() const {
    return data.begin();
}

[[nodiscard]] decltype(SegmentedStringView::data.end()) SegmentedStringView::end() const {
    return data.end();
}

//------------------------------------------
// Size

bool SegmentedStringView::empty() const {
    return data.empty();
}

std::size_t SegmentedStringView::characterCount() const {
    return charCount.get(data).count;
}

std::size_t SegmentedStringView::segmentCount() const {
    return data.size();
}

//------------------------------------------
// Span

SegmentedStringView SegmentedStringView::subspan(std::size_t const index) const {
    return SegmentedStringView(data.subspan(index));
}

SegmentedStringView SegmentedStringView::subspan(std::size_t const startIndex, std::size_t const count) const {
    return SegmentedStringView(data.subspan(startIndex, count));
}

std::string_view SegmentedStringView::getNamedArgument(std::string_view const name) const{
    assert(name.starts_with("--"));
    assert(name.size() > 2);
    if (auto it = std::ranges::find(data, name); it != data.end()) {
        std::advance(it, 1);
        if (it != data.end()) {
            return *it;
        }
    }
    return {};
}

SegmentedStringView SegmentedStringView::getNamedSpannedArgument(std::string_view const name) const {
    assert(name.starts_with("--"));
    assert(name.size() > 2);

    auto const it = std::ranges::find(data, name);
    if (it == data.end()) {
        return {};
    }

    // The named argument itself is not part of the returned span.
    auto const first = std::next(it);
    if (first == data.end()) {
        return SegmentedStringView{};
    }

    // Find the next named argument.
    auto const last = std::ranges::find_if(
        first,
        data.end(),
        [](std::string_view const arg) {
            return arg.starts_with("--");
        }
    );

    return SegmentedStringView{std::span{first, last}};
}

//------------------------------------------
// Substring

std::string SegmentedStringView::substring(std::size_t const startIndex, std::size_t const count) const {
    if (count == 0 || startIndex > characterCount()) {
        return "";
    }
    std::string result;
    result.reserve(characterCount() - startIndex);
    auto it = SpanIterator{data};
    for (std::size_t i = 0; i < startIndex + count && !it.endReached(); ++i) {
        if (i < startIndex) {
            ++it;
        }
        else {
            result += it.get();
        }
    }
    return result;
}

//------------------------------------------
// Copy

void SegmentedStringView::copy(std::vector<std::string_view>& other) const{
    std::ranges::copy(data, std::back_inserter(other));
}

void SegmentedStringView::copySubspan(std::vector<std::string_view>& other, std::size_t const index) const {
    std::ranges::copy(data.subspan(index), std::back_inserter(other));
}

void SegmentedStringView::copySubspan(std::vector<std::string_view>& other, std::size_t const startIndex, std::size_t const count) const{
    std::ranges::copy(data.subspan(startIndex, count), std::back_inserter(other));
}

//------------------------------------------
// Generate

std::string SegmentedStringView::recombine() const {
    // TODO: Once the entire FuncTree class uses SegmentedStringView, we should move the functionality purely into this class
    //       Since recombineArgs only makes sense for a SegmentedStringView at that point, having the implementation in StringHandler feels wrong.
    return StringHandler::recombineArgs(data);
}

//------------------------------------------
// Compare

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
