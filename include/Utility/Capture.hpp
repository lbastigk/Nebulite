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
 * @struct OutputLine
 * @brief Represents a line of captured output, either to cout or cerr.
 */
struct OutputLine{
    std::string content;
    enum class Type : uint8_t {
        COUT,
        CERR
        // TODO: add more types: input, info, warn, error, debug, etc. Unify with textInput class
    } type;
};

/**
 * @class Stream
 * @brief Stream class for capturing output and redirecting it to an ostream and internal log.
 */
template<std::ostream* /*BaseStream*/, OutputLine::Type /*LineType*/>
class Stream {
    Capture *parent;                                    // Main capture reference so we can lock its mutex, so cout/cerr don't interfere with each other
    void putStr(std::string const& str, bool const& printToConsole) const ;
public:
    //friend class Capture;
    explicit Stream(Capture* p) : parent(p) {}

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
 * @tparam LineType The type of the output line (e.g. COUT or CERR).
 */
template<std::ostream* BaseStream, OutputLine::Type LineType>
class HierarchicalStream {
    Stream<BaseStream, LineType> coutStream;
    HierarchicalStream* parent;

public:
    explicit HierarchicalStream(Capture* cap, HierarchicalStream* par = nullptr)
        : coutStream(cap), parent(par) {}

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
    template<std::ostream* BaseStream, OutputLine::Type LineType>
    friend class Stream;

    static auto constexpr noParent = nullptr;

    explicit Capture(Capture* parent)
    : log(this, parent ? &parent->log : noParent),
      error(this, parent ? &parent->error : noParent)
    {}

    HierarchicalStream<&std::cout, OutputLine::Type::COUT> log; // Stream for capturing cout output
    HierarchicalStream<&std::cerr, OutputLine::Type::CERR> error; // Stream for capturing cerr output

    /**
     * @brief Retrieves a pointer to the output log.
     * @return A pointer to the output log deque, const.
     */
    [[nodiscard]] std::deque<OutputLine> const& getOutputList() const {
        return outputList;
    }

    /**
     * @brief Clears the output log.
     */
    void clear(){
        outputList.clear();
    }

private:
    std::deque<OutputLine> outputList; // List of captured output lines
    std::mutex outputListMutex;  // Mutex for thread-safe access to outputList
};

template<std::ostream* BaseStream, OutputLine::Type LineType>
void Stream<BaseStream, LineType>::putStr(std::string const& str, bool const& printToConsole) const {
    if (printToConsole) {
        *BaseStream << str;
    }

    std::scoped_lock const lock(parent->outputListMutex);
    std::istringstream iss(str);
    std::string line;
    while (std::getline(iss, line)) {
        parent->outputList.push_back({line, LineType});
    }
}

template<std::ostream* BaseStream, OutputLine::Type LineType>
template<typename... Args>
void Stream<BaseStream, LineType>::print(bool const& printToConsole, Args&&... args) {
    std::ostringstream workingBuffer;
    if constexpr (sizeof...(args) != 0) {
        (workingBuffer << ... << args);
    }
    putStr(workingBuffer.str(), printToConsole);
}

template<std::ostream* BaseStream, OutputLine::Type LineType>
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
