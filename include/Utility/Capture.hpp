/**
 * @file Capture.hpp
 * @brief Defines classes for capturing output.
 */

#include <deque>
#include <string>
#include <mutex>

#pragma once

namespace Nebulite::Utility{
/**
 * @class Nebulite::Utility::Capture
 * @brief Captures output to cout and cerr into an internal log.
 */
class Capture{
    /**
     * @struct OutputLine
     * @brief Represents a line of captured output, either to cout or cerr.
     */
    struct OutputLine{
        std::string content;
        enum Type{
            COUT,
            CERR
        } type;
    };

    /**
     * @brief Captures output to cout into the global space's output log.
     */
    struct CoutCapture{
        Capture* parent;
        explicit CoutCapture(Capture* p) : parent(p) {}
        CoutCapture& operator<<(const std::string& str){
            std::lock_guard<std::mutex> lock(parent->outputLogMutex);
            parent->outputLog.push_back({str, OutputLine::COUT});
            return *this;
        }
    };
    CoutCapture cout{this};

    /**
     * @brief Captures output to cerr into the global space's output log.
     */
    struct CerrCapture{
        Capture* parent;
        explicit CerrCapture(Capture* p) : parent(p) {}
        CerrCapture& operator<<(const std::string& str){
            std::lock_guard<std::mutex> lock(parent->outputLogMutex);
            parent->outputLog.push_back({str, OutputLine::CERR});
            return *this;
        }
    };
    CerrCapture cerr{this};

    /**
     * @brief Retrieves a pointer to the output log.
     * @return A pointer to the output log deque, const.
     */
    const std::deque<OutputLine>& getOutputLogPtr() const {
        return outputLog;
    }

private:
    std::deque<OutputLine> outputLog; // Log of captured output lines
    std::mutex outputLogMutex;  // Mutex for thread-safe access to outputLog
};
} // namespace Nebulite::Utility