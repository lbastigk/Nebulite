//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint> // NOLINT
#include <numeric>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Utility {

// [GENERATE]

std::string StringHandler::createPaddedTable(std::vector<std::string> const& words, std::size_t const rowSize){
    // Find largest word
    auto maxSize = std::ranges::max_element(words, [](std::string_view const a, std::string_view const b) {
        return a.size() < b.size();
    })->length();
    return std::accumulate(words.begin(), words.end(), std::string{}, [maxSize, rowSize](std::string_view const acc, std::string_view const a) {
        std::string const paddedEntry = a + std::string(maxSize - a.length() + 1, ' ');
        if (rowSize > 0) {
            // Determine linebreaks
            auto const lastNewLinePos = acc.contains('\n') ? acc.find_last_of('\n') : 0;
            if (auto const projectedSize = acc.size() - lastNewLinePos + a.size(); projectedSize > rowSize) {
                return acc + std::string(maxSize - lastNewLinePos % maxSize + 1, ' ') + "\n" + paddedEntry;
            }
        }
        return acc + paddedEntry;
    });
}

std::string StringHandler::replaceAll(std::string_view const target, std::string_view const toReplace, std::string_view const replacer) {
    if (toReplace.empty())
        return std::string(target);
    return target
        | std::views::split(toReplace)
        | std::views::join_with(replacer)
        | std::ranges::to<std::string>();
}

// [VALIDATE]

bool StringHandler::containsAnyOf(std::string_view const str, std::string_view const chars) {
    return std::ranges::any_of(str, [&](char const c) {
        return chars.contains(c);
    });
}

bool StringHandler::isNumber(std::string_view str) {
    if (str.empty())
        return false;

    double value{};
    auto [ptr, ec] = std::from_chars(
        str.data(),
        str.data() + str.size(),
        value
    );

    return ec == std::errc() && ptr == str.data() + str.size();
}

// [STRIP]

void StringHandler::untilSpecialChar(std::string_view& str, char const& specialChar) {
    if (std::size_t const pos = str.find(specialChar); pos != std::string::npos && pos < str.size()) {
        str.remove_suffix(str.size() - pos);
    }
}

void StringHandler::afterSpecialChar(std::string_view& str, char const& specialChar) {
    if (std::size_t const pos = str.find(specialChar); pos != std::string::npos && pos + 1 < str.size()) {
        str.remove_prefix(pos + 1);
    }
}

void StringHandler::lStrip(std::string_view& str, char const& specialChar) {
    std::size_t const start = str.find_first_not_of(specialChar);
    if (start == std::string::npos && !str.empty() && str.front() == specialChar) {
        str = str.substr(0,0); // Edge case: all characters are specialChar, set to empty string
    }
    str.remove_prefix(start == std::string::npos ? 0 : start);
}

void StringHandler::rStrip(std::string_view& str, char const& specialChar) {
    std::size_t const end = str.find_last_not_of(specialChar);
    if (end == std::string::npos && !str.empty() && str.back() == specialChar) {
        str = str.substr(0,0); // Edge case: all characters are specialChar, set to empty string
    }
    str.remove_suffix(end == std::string::npos ? 0 : str.size() - end - 1);
}

void StringHandler::strip(std::string_view& str, char const& specialChar) {
    lStrip(str, specialChar);
    rStrip(str, specialChar);
}

// [ARGS]

// Helpers for parsing quoted arguments
namespace {

char singleQuote = '\'';

char doubleQuote = '"';

struct QuoteParseState {
    bool inDoubleQuote = false;
    bool inSingleQuote = false;

    [[nodiscard]] bool inAnyQuote() const {
        return inDoubleQuote || inSingleQuote;
    }
};

void handleEmptyToken(QuoteParseState const& state, std::vector<std::string>& result) {
    if (!state.inAnyQuote()) {
        // If not in quotes, add empty token with whitespace
        result.emplace_back(" ");
    } else {
        // In quotes, append space to last token
        if (!result.empty()) {
            result.back() += ' ';
        }
    }
}

std::string processQuoteToken(std::string_view const token, char const quoteChar, bool& quoteState) {
    auto cleanToken = token.substr(1); // Remove opening quote
    quoteState = true;

    // Check if quote closes in same token
    if (!cleanToken.empty() && cleanToken.back() == quoteChar) {
        quoteState = false;
        cleanToken.remove_suffix(1); // Remove closing quote
    }

    return std::string(cleanToken);
}

void handleQuoteStart(std::string_view const token, QuoteParseState& state, std::vector<std::string>& result) {
    if (token[0] == doubleQuote) {
        std::string const cleanToken = processQuoteToken(token, doubleQuote, state.inDoubleQuote);
        result.push_back(cleanToken);
    } else if (token[0] == singleQuote) {
        std::string const cleanToken = processQuoteToken(token, singleQuote, state.inSingleQuote);
        result.push_back(cleanToken);
    } else {
        // Regular unquoted token
        result.emplace_back(token);
    }
}

void handleQuoteEnd(std::string_view const token, char const quoteChar, bool& quoteState, std::vector<std::string>& result) {
    quoteState = false;
    auto cleanToken = std::string(token);

    // Remove the closing quote if it matches the expected quote character
    if (!cleanToken.empty() && cleanToken.back() == quoteChar) {
        cleanToken.pop_back();
    }

    if (!result.empty()) {
        result.back() += ' ' + cleanToken;
    }
}

void handleQuotedToken(std::string_view const token, QuoteParseState& state, std::vector<std::string>& result) {
    if (state.inDoubleQuote && !token.empty() && token.back() == doubleQuote) {
        handleQuoteEnd(token, doubleQuote, state.inDoubleQuote, result);
    } else if (state.inSingleQuote && !token.empty() && token.back() == singleQuote) {
        handleQuoteEnd(token, singleQuote, state.inSingleQuote, result);
    } else {
        // Still in quotes, append to last token
        if (!result.empty()) {
            result.back() += ' ' + std::string(token);
        }
    }
}
} // namespace

// [Args]

StringHandler::ParseResult StringHandler::parseQuotedArguments(std::string_view const cmd) {
    std::vector<std::string_view> const tokens = split(cmd, ' ');
    std::vector<std::string> result;
    QuoteParseState state;

    for (auto const& token : tokens) {
        // Keep empty tokens as extra whitespace - important for preserving user formatting
        // e.g. for text: "eval echo Value: {global:myVal}  |  Expected: {global:expected}"
        // Without this, the double spaces around | would be lost, which can be important for readability of the command
        // or simply to preserve the user's intended formatting.
        if (token.empty()) {
            handleEmptyToken(state, result);
            continue;
        }

        if (!state.inAnyQuote()) {
            handleQuoteStart(token, state, result);
        } else {
            handleQuotedToken(token, state, result);
        }
    }

    return {.args = std::move(result), .unclosedQuote = state.inAnyQuote()};
}

std::string StringHandler::recombineArgs(std::span<std::string_view const> const& args) {
    std::string result;
    for (std::size_t i = 0; i < args.size(); ++i) {
        result += args[i];
        // Don't add a whitespace if it's the last argument
        if (i < args.size() - 1) {
            // Important: don't add a whitespace if the argument already is a whitespace!
            // This is due to how parseQuotedArguments handles multiple spaces. They are treated as one arg per space.
            if (!args[i].empty() && args[i][0] != ' ') {
                result += ' ';
            }
        }
    }
    return result;
}

// [SPLIT]

std::vector<std::string_view> StringHandler::split(std::string_view const input, char const delimiter, bool const keepDelimiter){
    std::vector<std::string_view> tokens;
    if (!keepDelimiter) {
        std::size_t start = 0;
        while (start <= input.size()) {
            std::size_t end = input.find(delimiter, start);

            if (end == std::string_view::npos) {
                end = input.size();
            }
            tokens.emplace_back(input.substr(start, end - start));
            if (end == input.size()) {
                break;
            }
            start = end + 1;
        }
        return tokens;
    }

    // keep delimiter at start of token
    std::size_t start = 0;

    while (start < input.size()) {
        std::size_t const pos = input.find(delimiter, start);
        if (pos == std::string_view::npos) {
            tokens.emplace_back(input.substr(start));
            break;
        }

        // text before delimiter
        if (pos > start) {
            tokens.emplace_back(input.substr(start, pos - start));
        }

        std::size_t const next = input.find(delimiter, pos + 1);
        if (next == std::string_view::npos) {
            tokens.emplace_back(input.substr(pos));
            break;
        }
        tokens.emplace_back(input.substr(pos, next - pos));
        start = next;
    }

    return tokens;
}

namespace {

std::array constexpr pairs = {
    std::make_pair('(',  ')'),
    std::make_pair('[',  ']'),
    std::make_pair('{',  '}')
};

int depth(std::string_view const input) {
    int count = 0;
    for (auto const& c : input) {
        for (auto const& [opening, closing] : pairs) {
            if (c == opening) {
                count++;
            } else if (c == closing) {
                count--;
            }
        }
    }
    return count;
}

int depthOf(std::string_view const input, char const& delimiter) {
    int count = 0;
    for (auto const& c : input) {
        for (auto const& [opening, closing] : pairs) {
            if (opening != delimiter) continue;
            if (c == opening) {
                count++;
            } else if (c == closing) {
                count--;
            }
        }
    }
    return count;
}

} // namespace

std::vector<std::string_view>
StringHandler::splitOnSameDepth(std::string_view const input, char const delimiter){
    auto const basicSplitResult = split(input, delimiter, true);
    std::vector<std::string_view> result;
    std::string_view current;
    int currentDepth = 0;
    for (auto const& part : basicSplitResult) {
        if (current.empty()) {
            current = part;
        } else {
            // extend existing view
            current = {
                // NOLINTNEXTLINE
                current.data(),
                static_cast<std::size_t>(part.data() + part.size() - current.data())
            };
        }
        currentDepth = depth(current);
        if (currentDepth == 0) {
            result.push_back(current);
            current = {};
        }
    }
    if (!current.empty()) {
        result.push_back(current);
    }
    return result;
}

char StringHandler::delimiterToOpeningChar(Delimiter const delimiter) {
    switch (delimiter) {
    case Delimiter::parentheses: return '(';
    case Delimiter::brace: return '{';
    case Delimiter::bracket: return '[';
    default: std::unreachable();
    }
}

char StringHandler::delimiterToClosingChar(Delimiter const delimiter) {
    switch (delimiter) {
    case Delimiter::parentheses: return ')';
    case Delimiter::brace: return '}';
    case Delimiter::bracket: return ']';
    default: std::unreachable();
    }
}

std::vector<std::string_view>
StringHandler::splitOnSameDepthOf(std::string_view const input, Delimiter const delimiter) {
    auto const openingChar = delimiterToOpeningChar(delimiter);
    auto const closingChar = delimiterToClosingChar(delimiter);
    auto const basicSplitResult = split(input, openingChar, true);
    std::vector<std::string_view> result;
    std::string_view current;
    for (auto const& part : basicSplitResult) {
        if (current.empty()) {
            current = part;
        } else {
            // extend current view to include part
            current = {
                // NOLINTNEXTLINE
                current.data(),
                static_cast<std::size_t>(part.data() + part.size() - current.data())
            };
        }
        if (depthOf(current, openingChar) == 0) {
            if (auto const pos = current.find_last_of(closingChar); pos != std::string_view::npos) {
                // [current.begin(), closingChar]
                result.emplace_back(current.substr(0, pos + 1));

                // trailing remainder after closingChar
                if (auto const remainder = current.substr(pos + 1); !remainder.empty()) {
                    result.emplace_back(remainder);
                }
            } else {
                result.emplace_back(current);
            }
            current = {};
        }
    }
    if (!current.empty()) {
        result.emplace_back(current);
    }
    return result;
}

} // namespace Nebulite::Utility
