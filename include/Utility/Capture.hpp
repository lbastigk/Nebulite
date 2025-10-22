/**
 * @file Capture.hpp
 * @brief Defines classes for capturing output.
 */

#include <iostream>
#include <sstream>
#include <deque>
#include <string>
#include <mutex>

#pragma once

namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::Capture
 * @brief Captures output to cout and cerr into an internal log.
 */
class Capture{
public:
    static const std::string endl;

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
     * @class CaptureStream
     * @brief Stream class for capturing output and redirecting it to an ostream and internal log.
     */
    struct CaptureStream{
        Capture* parent;
        std::ostream& baseStream;
        explicit CaptureStream(Capture* p, std::ostream& s) : parent(p), baseStream(s) {}

        template<typename T>
        CaptureStream& operator<<(const T& data) {
            std::ostringstream oss;
            oss << data;
            {
                std::lock_guard<std::mutex> lock(parent->outputLogMutex);
                parent->outputLog.push_back({oss.str(), OutputLine::COUT});
            }
            baseStream << data;
            return *this;
        }

        CaptureStream& operator<<(const char* data) {
            std::ostringstream oss;
            oss << data;
            {
                std::lock_guard<std::mutex> lock(parent->outputLogMutex);
                parent->outputLog.push_back({oss.str(), OutputLine::COUT});
            }
            baseStream << data;
            return *this;
        }
    };

    CaptureStream cout{this, std::cout};
    CaptureStream cerr{this, std::cerr}; 

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