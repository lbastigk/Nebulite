//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <unordered_map>

// Nebulite
#include "Utility/StringHandler.hpp"
#include "Utility/Capture.hpp"



//------------------------------------------
namespace Nebulite::Utility {

bool StringHandler::containsAnyOf(std::string const& str, std::string const& chars){
    return std::ranges::any_of(str, [&](char const c){
        return chars.find(c) != std::string::npos;
    });
}

bool StringHandler::isNumber(std::string const& str){
    // Check if all characters are digits, +, -, or .
    // Then check if count of . is at most 1
    // Then check if + or - is only at the start
    return std::ranges::all_of(str, [](char const c){
        return std::isdigit(c) || c == '+' || c == '-' || c == '.';
    }) &&
    (std::ranges::count(str, '.') <= 1) &&
    (str.find_first_of("+-") <= 0 || str.find_first_of("+-") == std::string::npos) &&
    !str.empty();
}

std::string StringHandler::replaceAll(std::string target, std::string const& toReplace, std::string const& replacer){
    std::string::size_type pos = 0u;
    while ((pos = target.find(toReplace, pos)) != std::string::npos){
        target.replace(pos, toReplace.length(), replacer);
        pos += toReplace.length();
    }
    return target;
}

std::string StringHandler::untilSpecialChar(std::string const& input, char const& specialChar){
    if (size_t const pos = input.find(specialChar); pos != std::string::npos && pos < input.size()){
        return input.substr(0, pos);
    }
    return input;
}

std::string StringHandler::afterSpecialChar(std::string const& input, char const& specialChar){
    if (size_t const pos = input.find(specialChar); pos != std::string::npos && pos + 1 < input.size()){
        return input.substr(pos+1);
    }
    return input;
}

std::string StringHandler::lStrip(std::string const& input, char const& specialChar){
    size_t const start = input.find_first_not_of(specialChar);
    return start == std::string::npos ? "" : input.substr(start);
}

std::string StringHandler::rStrip(std::string const& input, char const& specialChar){
    size_t const end = input.find_last_not_of(specialChar);
    return end == std::string::npos ? "" : input.substr(0, end + 1);
}

std::vector<std::string> StringHandler::split(std::string const& input, char const& delimiter, bool const& keepDelimiter){
    std::vector<std::string> tokens;
    
    if (!keepDelimiter){
        // Original behavior - split and exclude delimiter
        size_t start = 0;
        size_t end = 0;
        while ((end = input.find(delimiter, start)) != std::string::npos){
            tokens.push_back(input.substr(start, end - start));
            start = end + 1;
        }
        if (start < input.length()){
            tokens.push_back(input.substr(start));
        }
    } else {
        // New behavior - split and keep delimiter at start of tokens
        size_t pos = 0;
        size_t start = 0;
        
        while ((pos = input.find(delimiter, start)) != std::string::npos){
            // Add everything before the delimiter as a token (if not empty)
            if (pos > start){
                tokens.push_back(input.substr(start, pos - start));
            }
            
            // Find the next delimiter to determine where this token ends
            size_t const nextPos = input.find(delimiter, pos + 1);
            if (nextPos == std::string::npos){
                // No more delimiters, take rest of string
                tokens.push_back(input.substr(pos));
                break;
            }
            // Take from current delimiter to next delimiter
            tokens.push_back(input.substr(pos, nextPos - pos));
            start = nextPos;
        }
        
        // If we never found a delimiter, add the whole string
        if (tokens.empty() && !input.empty()){
            tokens.push_back(input);
        }
    }
    
    return tokens;
}

std::vector<std::string> StringHandler::splitOnSameDepth(std::string const& input, char const& delimiter){
    std::vector<std::string> result;

    // Map opening delimiters to their closing ones
    static std::unordered_map<char, char> const pairs = {
        {'(', ')'}, {'[', ']'}, {'{', '}'}
    };

    // Find the matching closing delimiter
    char closing = 0;
    for (auto const& [potentialOpeningCharacter, potentialClosingCharacter] : pairs){
        if (potentialOpeningCharacter == delimiter){
            closing = potentialClosingCharacter;
            break;
        }
    }

    if (closing == 0){
        // Invalid delimiter
        result.push_back(input);
        return result;
    }

    int depth = 0;
    std::string current;

    for (size_t i = 0; i < input.size(); ++i){
        char const c = input[i];
        current.push_back(c);

        if (c == delimiter){
            depth++;
        } else if (c == closing){
            depth--;
            if (depth == 0){
                // We just closed a top-level pair
                result.push_back(current);
                current.clear();
            }
        } else if (depth == 0){
            // Outside delimiter groups
            // If the current string is complete text before a new delimiter
            if (i + 1 < input.size() && input[i+1] == delimiter){
                result.push_back(current);
                current.clear();
            }
        }
    }

    // If there's any leftover text
    if (!current.empty()){
        result.push_back(current);
    }

    return result;
}


// Helpers for parsing quoted arguments
namespace {
    struct QuoteParseState {
        bool inDoubleQuote = false;
        bool inSingleQuote = false;
        
        [[nodiscard]] bool inAnyQuote() const {
            return inDoubleQuote || inSingleQuote;
        }
    };

    void handleEmptyToken(QuoteParseState const& state, std::vector<std::string>& result){
        if (!state.inAnyQuote()){
            // If not in quotes, add empty token with whitespace
            result.emplace_back(" ");
        } else {
            // In quotes, append space to last token
            if (!result.empty()){
                result.back() += " ";
            }
        }
    }

    std::string processQuoteToken(std::string const& token, char const quoteChar, bool& quoteState){
        std::string cleanToken = token.substr(1); // Remove opening quote
        quoteState = true;
        
        // Check if quote closes in same token
        if (!cleanToken.empty() && cleanToken.back() == quoteChar){
            quoteState = false;
            cleanToken.pop_back(); // Remove closing quote
        }
        
        return cleanToken;
    }

    void handleQuoteStart(std::string const& token, QuoteParseState& state, std::vector<std::string>& result){
        if (token[0] == '"'){
            std::string const cleanToken = processQuoteToken(token, '"', state.inDoubleQuote);
            result.push_back(cleanToken);
        } else if (token[0] == '\''){
            std::string const cleanToken = processQuoteToken(token, '\'', state.inSingleQuote);
            result.push_back(cleanToken);
        } else {
            // Regular unquoted token
            result.push_back(token);
        }
    }

    void handleQuoteEnd(std::string const& token, char const quoteChar, bool& quoteState, std::vector<std::string>& result){
        quoteState = false;
        std::string cleanToken = token;
        
        // Remove the closing quote if it matches the expected quote character
        if (!cleanToken.empty() && cleanToken.back() == quoteChar){
            cleanToken.pop_back();
        }
        
        if (!result.empty()){
            result.back() += " " + cleanToken;
        }
    }

    void handleQuotedToken(std::string const& token, QuoteParseState& state, std::vector<std::string>& result){
        if (state.inDoubleQuote && !token.empty() && token.back() == '"'){
            handleQuoteEnd(token, '"', state.inDoubleQuote, result);
        } else if (state.inSingleQuote && !token.empty() && token.back() == '\''){
            handleQuoteEnd(token, '\'', state.inSingleQuote, result);
        } else {
            // Still in quotes, append to last token
            if (!result.empty()){
                result.back() += " " + token;
            }
        }
    }
}

std::vector<std::string> StringHandler::parseQuotedArguments(std::string const& cmd){
    std::vector<std::string> const tokens = split(cmd, ' ');
    std::vector<std::string> result;
    QuoteParseState state;
    
    for (auto const& token : tokens){
        // Keep empty tokens as extra whitespace - important for preserving user formatting
        // e.g. for text: "eval echo Value: {global.myVal}  |  Expected: {global.expected}"
        if (token.empty()){
            handleEmptyToken(state, result);
            continue;
        }
        
        if (!state.inAnyQuote()){
            handleQuoteStart(token, state, result);
        } else {
            handleQuotedToken(token, state, result);
        }
    }
    
    // Warning for unclosed quotes
    if (state.inAnyQuote()){
        Capture::cerr() << "Warning: Unclosed quote in command: " << cmd << Capture::endl;
    }
    
    return result;
}

// cppcheck-suppress constParameter
std::string StringHandler::recombineArgs(int const argc, char* argv[]){
    std::string result;
    for (int i = 0; i < argc; ++i){
        result += argv[i];
        // Don't add a whitespace if it's the last argument
        if (i < argc - 1){
            // Important: don't add a whitespace if the argument already is a whitespace!
            // This is due to how parseQuotedArguments handles multiple spaces. They are treated as one arg per space.
            if(argv[i][0] != ' ') result += " ";
        }
    }
    return result;
}

} // namespace Nebulite::Utility