/**
 * @file Capture.hpp
 * @brief Defines classes for capturing output.
 */

#ifndef NEBULITE_UTILITY_IO_CAPTURE_HPP
#define NEBULITE_UTILITY_IO_CAPTURE_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

//------------------------------------------
// Forward declarations

namespace Nebulite::Utility::IO {
class Capture;
} // namespace Nebulite::Utility::IO

//------------------------------------------
namespace Nebulite::Utility::IO {

/**
 * @struct HistoryLine
 * @brief Represents a line of captured output and its type
 */
struct HistoryLine{
    std::string content;
    enum class Type : std::uint8_t {
        Input,
        Info,
        Warning,
        Error
    } type;
};

/**
 * @class Stream
 * @brief Stream class for capturing output and redirecting it to an ostream and internal log.
 */
template<std::ostream* /*BaseStream*/, HistoryLine::Type /*LineType*/>
class Stream {
    Capture* capture; // Main capture reference so we can lock its mutex, so cout/cerr don't interfere with each other
    void putStr(std::string const& str, bool printToConsole) const ;

    template<typename T>
    decltype(auto) logArg(T&& t) {
        using U = std::remove_reference_t<T>;

        if constexpr (std::is_array_v<U>) {
            return std::string_view(t);
        } else {
            return std::forward<T>(t);
        }
    }

public:
    explicit Stream(Capture* c) : capture(c) {}

    //------------------------------------------
    // Printing helpers

    /**
     * @brief Prints the provided arguments to the stream and captures them in a list.
     * @tparam Args The types of the arguments to print.
     * @param printToConsole Whether to print to the console or just capture in the list.
     * @param args The arguments to print.
     */
    template<typename... Args>
    void print(bool printToConsole, Args&&... args);

    /**
     * @brief Prints the provided arguments followed by a newline to the stream and captures them in a list.
     * @tparam Args The types of the arguments to print.
     * @param printToConsole Whether to print to the console or just capture in the list.
     * @param args The arguments to print.
     */
    template<typename... Args>
    void println(bool printToConsole, Args&&... args);
};

/**
 * @brief HierarchicalStream class that allows for hierarchical capturing of output, where child streams can forward input to parents for unified listing,
 *        while proper printing is reserved to only the root stream to avoid duplicates. This is useful for domain-specific logging that still gets captured in a unified log.
 * @tparam BaseStream The base stream to print to (e.g. std::cout or std::cerr).
 * @tparam LineType The type of the output line
 */
template<std::ostream* BaseStream, HistoryLine::Type LineType>
class HierarchicalStream {
    friend class Capture;

    Stream<BaseStream, LineType> coutStream;
    HierarchicalStream* parent;

    bool outputEnabled = true;

public:
    explicit HierarchicalStream(Capture* cap, HierarchicalStream* par = nullptr)
        : coutStream(cap), parent(par) {}

    [[nodiscard]] bool hasParent() const {
        return parent != nullptr;
    }

    template<typename... Args>
    void print(Args&&... args);

    template<typename... Args>
    void println(Args&&... args);
};

/**
 * @class Nebulite::Utility::IO::Capture
 * @brief Unified capture class providing multiple streams with a potential hierarchy, allowing for both domain-specific logging and unified logging,
 *        while ensuring thread-safe access to the captured output.
 */
class Capture{
public:
    static auto constexpr noParent = nullptr;

    explicit Capture(Capture* parent);

    HierarchicalStream<&std::cout, HistoryLine::Type::Info> log;
    HierarchicalStream<&std::cerr, HistoryLine::Type::Warning> warning;
    HierarchicalStream<&std::cerr, HistoryLine::Type::Error> error;

    /**
     * @brief Retrieves a pointer to the history.
     * @return A pointer to the output log deque, const.
     */
    [[nodiscard]] std::deque<HistoryLine> const& getHistory() const ;

    /**
     * @brief Clears the output log.
     */
    void clear();

    [[nodiscard]] bool hasParent() const ;

    /**
     * @brief Appends input to the history
     * @param str The string to append to the log.
     * @param lineType The type of line to add.
     */
    void appendToHistory(std::string const& str, HistoryLine::Type lineType);

    // TODO: instead of enable/disable output, we should redirect:
    // auto str = capture.redirect(lambda) // string
    // auto history = capture.redirectHistory(lambda) // Vector of historylines
    // This would automatically enable/disable output, and send to the proper stream
    // We can add a vector of redirectors, and push/pop new redirectors
    // + threadsafety by only adding if thread matches?
    // map[threadid]->vector<redirects>
    // Could get complicated ...

    template<typename F>
    std::string redirect(F f) {
        std::scoped_lock const lock(historyMutex);
        redirectorStack.emplace_back();
        disableOutput();
        std::invoke(f);
        auto const history = redirectorStack.back().toString();
        redirectorStack.pop_back();
        if (redirectorStack.empty()) {
            enableOutput();
        }
        return history;
    }

    template<typename F>
    std::deque<HistoryLine> redirectHistory(F f) {
        std::scoped_lock const lock(historyMutex);
        redirectorStack.emplace_back();
        disableOutput();
        std::invoke(f);
        auto const history = redirectorStack.back().getLines();
        redirectorStack.pop_back();
        if (redirectorStack.empty()) {
            enableOutput();
        }
        return history;
    }

private:
    /**
     * @brief Disables the output temporarily, preventing any further output from being printed to the console.
     * @details Output is still captured by the log!
     * @return True if the output was successfully disabled, false if it was already disabled.
     */
    bool disableOutput() {
        if (outputEnabled) {
            outputEnabled = false;

            // Disable for each stream
            log.outputEnabled = false;
            warning.outputEnabled = false;
            error.outputEnabled = false;
            return true;
        }
        return false;
    }

    /**
     * @brief Re-enables the output
     */
    void enableOutput() {
        outputEnabled = true;

        // Enable for each stream
        log.outputEnabled = true;
        warning.outputEnabled = true;
        error.outputEnabled = true;
    }

    bool outputEnabled = true;

    class History {
        std::deque<HistoryLine> lines;

        bool appendableToLastLine(HistoryLine::Type lineType);
    public:
        History() = default;
        ~History() = default;

        History(History const&) = default;
        History& operator=(History const&) = default;
        History(History&&) = default;
        History& operator=(History&&) = default;

        [[nodiscard]] std::deque<HistoryLine> const& getLines() const ;

        void clear();

        std::string toString();

        void addHistoryLine(std::string const& str, HistoryLine::Type lineType);
    };

    History localHistory;
    std::recursive_mutex historyMutex;  // Mutex for thread-safe access to outputList

    std::vector<History> redirectorStack;
};

} // namespace Nebulite::Utility::IO
#include "Nebulite/Utility/IO/Capture.tpp" // NOLINT
#endif // NEBULITE_UTILITY_IO_CAPTURE_HPP
