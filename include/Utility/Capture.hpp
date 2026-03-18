/**
 * @file Capture.hpp
 * @brief Defines classes for capturing output.
 * @todo Modify Capture to be part of any domain, and every domain forwards captured output to a unified capture class as well.
 *       This way we may have domain-specific logging, as well as unified logging.
 *       This is useful if we wish to expand the GlobalSpaceViewer to show be a domain-agnostic tool for
 *       - viewing state
 *       - viewing logs
 *       As we'll be able to see only the logs relevant to a specific domain, or all logs together.
 */

#ifndef NEBULITE_UTILITY_CAPTURE_HPP
#define NEBULITE_UTILITY_CAPTURE_HPP

//------------------------------------------
// Includes

// Standard library
#include <deque>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

//------------------------------------------
namespace Nebulite::Utility {

// Predeclaration of Capture
class Capture;

/**
 * @struct HistoryLine
 * @brief Represents a line of captured output and its type
 */
struct HistoryLine{
    std::string content;
    enum class Type : uint8_t {
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
    void putStr(std::string const& str, bool const& printToConsole) const ;
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
    void print(bool const& printToConsole, Args&&... args);

    /**
     * @brief Prints the provided arguments followed by a newline to the stream and captures them in a list.
     * @tparam Args The types of the arguments to print.
     * @param printToConsole Whether to print to the console or just capture in the list.
     * @param args The arguments to print.
     */
    template<typename... Args>
    void println(bool const& printToConsole, Args&&... args);
};

/**
 * @brief HierarchicalStream class that allows for hierarchical capturing of output, where child streams can forward input to parents for unified listing,
 *        while proper printing is reserved to only the root stream to avoid duplicates. This is useful for domain-specific logging that still gets captured in a unified log.
 * @tparam BaseStream The base stream to print to (e.g. std::cout or std::cerr).
 * @tparam LineType The type of the output line
 */
template<std::ostream* BaseStream, HistoryLine::Type LineType>
class HierarchicalStream {
    Stream<BaseStream, LineType> coutStream;
    HierarchicalStream* parent;

public:
    explicit HierarchicalStream(Capture* cap, HierarchicalStream* par = nullptr)
        : coutStream(cap), parent(par) {}

    [[nodiscard]] bool hasParent() const {
        return parent != nullptr;
    }

    template<typename... Args>
    void print(Args&&... args){
        if (parent) {
            // Pass to parent stream for retention
            parent->print(std::forward<Args>(args)...);
        }
        // Only print to console if this is the root stream, to avoid duplicate prints
        coutStream.print(!parent, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void println(Args&&... args){
        if (parent) {
            // Pass to parent stream for retention
            parent->println(std::forward<Args>(args)...);
        }
        // Only print to console if this is the root stream, to avoid duplicate prints
        coutStream.println(!parent, std::forward<Args>(args)...);
    }
};

/**
 * @class Nebulite::Utility::Capture
 * @brief Unified capture class providing multiple streams with a potential hierarchy, allowing for both domain-specific logging and unified logging,
 *        while ensuring thread-safe access to the captured output.
 */
class Capture{
public:
    template<std::ostream* BaseStream, HistoryLine::Type LineType>
    friend class Stream;

    static auto constexpr noParent = nullptr;

    explicit Capture(Capture* parent)
    : log(this, parent ? &parent->log : noParent),
      warning(this, parent ? &parent->warning : noParent),
      error(this, parent ? &parent->error : noParent)
    {}

    HierarchicalStream<&std::cout, HistoryLine::Type::Info> log;
    HierarchicalStream<&std::cerr, HistoryLine::Type::Warning> warning;
    HierarchicalStream<&std::cerr, HistoryLine::Type::Error> error;

    /**
     * @brief Retrieves a pointer to the history.
     * @return A pointer to the output log deque, const.
     */
    [[nodiscard]] std::deque<HistoryLine> const& getHistory() const {
        return history;
    }

    /**
     * @brief Clears the output log.
     */
    void clear(){
        history.clear();
    }

    [[nodiscard]] bool hasParent() const {
        // Doesn't matter what stream we check
        return log.hasParent();
    }

    /**
     * @brief Appends input to the output log with thread safety.
     * @param str The string to append to the log.
     */
    void appendInput(std::string const& str) {
        std::scoped_lock const lock(historyMutex);
        history.push_back({str, HistoryLine::Type::Input});
    }

private:
    std::deque<HistoryLine> history; // List of captured output lines
    std::mutex historyMutex;  // Mutex for thread-safe access to outputList
};

template<std::ostream* BaseStream, HistoryLine::Type LineType>
void Stream<BaseStream, LineType>::putStr(std::string const& str, bool const& printToConsole) const {
    if (printToConsole) {
        *BaseStream << str;
    }

    std::scoped_lock const lock(capture->historyMutex);
    std::istringstream iss(str);
    std::string line;
    while (std::getline(iss, line)) {
        capture->history.push_back({line, LineType});
    }
}

template<std::ostream* BaseStream, HistoryLine::Type LineType>
template<typename... Args>
void Stream<BaseStream, LineType>::print(bool const& printToConsole, Args&&... args) {
    std::ostringstream workingBuffer;
    if constexpr (sizeof...(args) != 0) {
        (workingBuffer << ... << args);
    }
    putStr(workingBuffer.str(), printToConsole);
}

template<std::ostream* BaseStream, HistoryLine::Type LineType>
template<typename... Args>
void Stream<BaseStream, LineType>::println(bool const& printToConsole, Args&&... args) {
    std::ostringstream workingBuffer;
    if constexpr (sizeof...(args) != 0) {
        (workingBuffer << ... << args);
    }
    workingBuffer << '\n';
    putStr(workingBuffer.str(), printToConsole);
}

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_CAPTURE_HPP
