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
    template<typename... Args>
    void print(Args&&... args);

    template<typename... Args>
    void println(Args&&... args);
};

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
            parent->print(std::forward<Args>(args)...);
        }
        coutStream.print(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void println(Args&&... args){
        if (parent) {
            parent->println(std::forward<Args>(args)...);
        }
        coutStream.println(std::forward<Args>(args)...);
    }
};

/**
 * @class Nebulite::Utility::Capture
 * @brief Captures output to cout and cerr into an internal log.
 * 
 * Both outputs logs cout and cerr are threadsafe.
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
    [[nodiscard]] std::deque<OutputLine> const& getOutputLog() const {
        return outputLog;
    }

    /**
     * @brief Clears the output log.
     */
    void clear(){
        outputLog.clear();
    }

private:
    std::deque<OutputLine> outputLog; // Log of captured output lines
    std::mutex outputLogMutex;  // Mutex for thread-safe access to outputLog
};

template<std::ostream* BaseStream, OutputLine::Type LineType>
void Stream<BaseStream, LineType>::putStr(std::string const& str, bool const& printToConsole) const {
    if (printToConsole) {
        *BaseStream << str;
    }

    std::scoped_lock const lock(parent->outputLogMutex);
    std::istringstream iss(str);
    std::string line;
    while (std::getline(iss, line)) {
        parent->outputLog.push_back({line, LineType});
    }
}

template<std::ostream* BaseStream, OutputLine::Type LineType>
template<typename... Args>
void Stream<BaseStream, LineType>::print(Args&&... args) {
    std::ostringstream workingBuffer;
    if constexpr (sizeof...(args) != 0) {
        (workingBuffer << ... << args);
    }
    putStr(workingBuffer.str(), true);
}

template<std::ostream* BaseStream, OutputLine::Type LineType>
template<typename... Args>
void Stream<BaseStream, LineType>::println(Args&&... args) {
    std::ostringstream workingBuffer;
    if constexpr (sizeof...(args) != 0) {
        (workingBuffer << ... << args);
    }
    workingBuffer << '\n';
    putStr(workingBuffer.str(), true);
}

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_CAPTURE_HPP
