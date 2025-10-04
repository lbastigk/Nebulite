#include "Utility/StringHandler.hpp"

bool Nebulite::Utility::StringHandler::containsAnyOf(const std::string& str, const std::string& chars) {
    return std::any_of(str.begin(), str.end(), [&](char c) {
        return chars.find(c) != std::string::npos;
    });
}

bool Nebulite::Utility::StringHandler::isNumber(std::string str) {
    // Trim leading/trailing whitespace
    str.erase(0, str.find_first_not_of(" \t\n\r"));
    str.erase(str.find_last_not_of(" \t\n\r") + 1);

    if (str.empty()) return false;

    size_t i = 0;

    // Optional sign
    if (str[i] == '+' || str[i] == '-') i++;

    bool hasDigits = false;
    bool hasDot = false;

    for (; i < str.length(); ++i) {
        if (std::isdigit(str[i])) {
            hasDigits = true;
        } else if (str[i] == '.') {
            if (hasDot) return false; // Only one dot allowed
            hasDot = true;
        } else {
            return false; // Invalid character
        }
    }

    return hasDigits;
}

std::string Nebulite::Utility::StringHandler::replaceAll(std::string target, const std::string& toReplace, const std::string& replacer) {
    std::string::size_type pos = 0u;
    while ((pos = target.find(toReplace, pos)) != std::string::npos) {
        target.replace(pos, toReplace.length(), replacer);
        pos += toReplace.length();
    }
    return target;
}

std::string Nebulite::Utility::StringHandler::untilSpecialChar(std::string input, char specialChar){
    size_t pos = input.find(specialChar);
    if (pos != std::string::npos && pos < input.size()) {
        return input.substr(0, pos);;
    }
    return input;
}

std::string Nebulite::Utility::StringHandler::afterSpecialChar(std::string input, char specialChar){
    size_t pos = input.find(specialChar);
    if (pos != std::string::npos && pos + 1 < input.size()) {
        return input.substr(pos+1);
    }
    return input;
}

std::string Nebulite::Utility::StringHandler::lstrip(const std::string& input, char specialChar) {
    size_t start = input.find_first_not_of(specialChar);
    return (start == std::string::npos) ? "" : input.substr(start);
}

std::string Nebulite::Utility::StringHandler::rstrip(const std::string& input, char specialChar) {
    size_t end = input.find_last_not_of(specialChar);
    return (end == std::string::npos) ? "" : input.substr(0, end + 1);
}

std::vector<std::string> Nebulite::Utility::StringHandler::split(const std::string& input, char delimiter, bool keepDelimiter) {
    std::vector<std::string> tokens;
    
    if (!keepDelimiter) {
        // Original behavior - split and exclude delimiter
        size_t start = 0;
        size_t end = 0;
        while ((end = input.find(delimiter, start)) != std::string::npos) {
            tokens.push_back(input.substr(start, end - start));
            start = end + 1;
        }
        if (start < input.length()) {
            tokens.push_back(input.substr(start));
        }
    } else {
        // New behavior - split and keep delimiter at start of tokens
        size_t pos = 0;
        size_t start = 0;
        
        while ((pos = input.find(delimiter, start)) != std::string::npos) {
            // Add everything before the delimiter as a token (if not empty)
            if (pos > start) {
                tokens.push_back(input.substr(start, pos - start));
            }
            
            // Find the next delimiter to determine where this token ends
            size_t nextPos = input.find(delimiter, pos + 1);
            if (nextPos == std::string::npos) {
                // No more delimiters, take rest of string
                tokens.push_back(input.substr(pos));
                break;
            } else {
                // Take from current delimiter to next delimiter
                tokens.push_back(input.substr(pos, nextPos - pos));
                start = nextPos;
            }
        }
        
        // If we never found a delimiter, add the whole string
        if (tokens.empty() && !input.empty()) {
            tokens.push_back(input);
        }
    }
    
    return tokens;
}

std::vector<std::string> Nebulite::Utility::StringHandler::splitOnSameDepth(const std::string& input, char delimiter) {
    std::vector<std::string> result;

    // Map opening delimiters to their closing ones
    static const std::unordered_map<char, char> pairs = {
        {'(', ')'}, {'[', ']'}, {'{', '}'}
    };

    // Find the matching closing delimiter
    char closing = 0;
    for (auto &p : pairs) {
        if (p.first == delimiter) {
            closing = p.second;
            break;
        }
    }

    if (closing == 0) {
        // Invalid delimiter
        result.push_back(input);
        return result;
    }

    int depth = 0;
    std::string current;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        current.push_back(c);

        if (c == delimiter) {
            depth++;
        } else if (c == closing) {
            depth--;
            if (depth == 0) {
                // We just closed a top-level pair
                result.push_back(current);
                current.clear();
            }
        } else if (depth == 0) {
            // Outside delimiter groups
            // If the current string is complete text before a new delimiter
            if (!current.empty() && (i + 1 < input.size() && input[i+1] == delimiter)) {
                result.push_back(current);
                current.clear();
            }
        }
    }

    // If there's any leftover text
    if (!current.empty()) {
        result.push_back(current);
    }

    return result;
}


// Helpers for parsing quoted arguments
namespace {
    struct QuoteParseState {
        bool inDoubleQuote = false;
        bool inSingleQuote = false;
        
        bool inAnyQuote() const {
            return inDoubleQuote || inSingleQuote;
        }
    };

    void handleEmptyToken(const QuoteParseState& state, std::vector<std::string>& result) {
        if (!state.inAnyQuote()) {
            // If not in quotes, add empty token with whitespace
            result.push_back(" ");
        } else {
            // In quotes, append space to last token
            if (!result.empty()) {
                result.back() += " ";
            }
        }
    }

    std::string processQuoteToken(const std::string& token, char quoteChar, bool& quoteState) {
        std::string cleanToken = token.substr(1); // Remove opening quote
        quoteState = true;
        
        // Check if quote closes in same token
        if (!cleanToken.empty() && cleanToken.back() == quoteChar) {
            quoteState = false;
            cleanToken.pop_back(); // Remove closing quote
        }
        
        return cleanToken;
    }

    void handleQuoteStart(const std::string& token, QuoteParseState& state, std::vector<std::string>& result) {
        if (token[0] == '"') {
            std::string cleanToken = processQuoteToken(token, '"', state.inDoubleQuote);
            result.push_back(cleanToken);
        } else if (token[0] == '\'') {
            std::string cleanToken = processQuoteToken(token, '\'', state.inSingleQuote);
            result.push_back(cleanToken);
        } else {
            // Regular unquoted token
            result.push_back(token);
        }
    }

    void handleQuoteEnd(const std::string& token, char quoteChar, bool& quoteState, std::vector<std::string>& result) {
        quoteState = false;
        std::string cleanToken = token.substr(0, token.length() - 1);
        if (!result.empty()) {
            result.back() += " " + cleanToken;
        }
    }

    void handleQuotedToken(const std::string& token, QuoteParseState& state, std::vector<std::string>& result) {
        if (state.inDoubleQuote && !token.empty() && token.back() == '"') {
            handleQuoteEnd(token, '"', state.inDoubleQuote, result);
        } else if (state.inSingleQuote && !token.empty() && token.back() == '\'') {
            handleQuoteEnd(token, '\'', state.inSingleQuote, result);
        } else {
            // Still in quotes, append to last token
            if (!result.empty()) {
                result.back() += " " + token;
            }
        }
    }
}

std::vector<std::string> Nebulite::Utility::StringHandler::parseQuotedArguments(const std::string& cmd) {
    std::vector<std::string> tokens = Nebulite::Utility::StringHandler::split(cmd, ' ');
    std::vector<std::string> result;
    QuoteParseState state;
    
    for (const auto& token : tokens) {
        // Keep empty tokens as extra whitespace - important for preserving user formatting
        // e.g. for text: "eval echo Value: {global.myVal}  |  Expected: {global.expected}"
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
    
    // Warning for unclosed quotes
    if (state.inAnyQuote()) {
        std::cerr << "Warning: Unclosed quote in command: " << cmd << std::endl;
    }
    
    return result;
}