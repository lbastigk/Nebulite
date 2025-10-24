/**
 * @file Capture.hpp
 * @brief Defines classes for capturing output.
 */

#include <iostream>
#include <sstream>
#include <deque>
#include <string>
#include <mutex>

#include "Utility/StringHandler.hpp"

#pragma once

namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::Capture
 * @brief Captures output to cout and cerr into an internal log.
 */
class Capture{
public:
    static std::string const endl;

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
        std::string lastLine = "";
        Capture *parent;
        std::ostream& baseStream;
        OutputLine::Type type;
        explicit CaptureStream(Capture* p, std::ostream& s, OutputLine::Type t) : parent(p), baseStream(s), type(t) {}

        template<typename T>
        CaptureStream& operator<<(T const& data) {
            baseStream << data;
            {
                std::lock_guard<std::mutex> lock(parent->outputLogMutex);

                // Combine lastLine with new data
                std::ostringstream workingBuffer;
                workingBuffer << lastLine << data;
                lastLine = "";

                // Split buffer by newlines
                std::string buf = workingBuffer.str();
                std::vector<std::string> lines = Nebulite::Utility::StringHandler::split(buf, '\n');

                // If last character is not newline, keep it in workingBuffer
                // And do not push it to outputLog yet
                if(!buf.empty() && buf.back() != '\n'){
                    lastLine = lines.back();
                    lines.pop_back();
                }

                // Push complete lines to outputLog
                for (auto const& line : lines) {
                    parent->outputLog.push_back({line, type});
                }
            }
            return *this;
        }

        CaptureStream& operator<<(char const* data) {
            // Cast to std::string, then call templated operator
            return (*this) << std::string(data);
        }
    };

    /**
     * @brief Retrieves the singleton instance of Capture.
     * @return Reference to the singleton Capture instance.
     */
    static Capture& instance() {
        static Capture singleton;
        return singleton;
    }

    /**
     * @brief Retrieves the CaptureStream for cout.
     * @return Reference to the CaptureStream for cout.
     */
    static CaptureStream& cout() { return instance().coutStream; }

    /**
     * @brief Retrieves the CaptureStream for cerr.
     * @return Reference to the CaptureStream for cerr.
     */
    static CaptureStream& cerr() { return instance().cerrStream; }

    /**
     * @brief Retrieves a pointer to the output log.
     * @return A pointer to the output log deque, const.
     */
    const std::deque<OutputLine>& getOutputLogPtr() const {
        return outputLog;
    }

    /**
     * @brief Clears the output log.
     */
    static void clear() {
        instance().outputLog.clear();
    }

private:
    // Make constructor private for singleton
    Capture() : coutStream(this, std::cout, OutputLine::COUT), cerrStream(this, std::cerr, OutputLine::CERR) {}

    CaptureStream coutStream{this, std::cout, OutputLine::COUT};
    CaptureStream cerrStream{this, std::cerr, OutputLine::CERR};

    std::deque<OutputLine> outputLog; // Log of captured output lines
    std::mutex outputLogMutex;  // Mutex for thread-safe access to outputLog
};
} // namespace Nebulite::Utility