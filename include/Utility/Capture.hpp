/**
 * @file Capture.hpp
 * @brief Defines classes for capturing output.
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

// Nebulite
#include "Nebulite.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::Capture
 * @brief Captures output to cout and cerr into an internal log.
 * 
 * Both outputs logs cout and cerr are threadsafe.
 */
class Capture{
public:
    static std::string const endl;

    /**
     * @struct OutputLine
     * @brief Represents a line of captured output, either to cout or cerr.
     */
    struct alignas(CACHE_LINE_SIZE) OutputLine{
        std::string content;
        enum class Type : uint8_t {
            COUT,
            CERR
        } type;
    };

    /**
     * @class CaptureStream
     * @brief Stream class for capturing output and redirecting it to an ostream and internal log.
     */
    struct alignas(CACHE_LINE_SIZE) CaptureStream{
        std::string lastLine;
        Capture *parent;                                    // Parent reference so we can lock its mutex, so cout/cerr don't interfere
        std::reference_wrapper<std::ostream> baseStream;    // ostream outlives CaptureStream, so reference is safe
        OutputLine::Type type;
        explicit CaptureStream(Capture* p, std::ostream& s, OutputLine::Type t) : parent(p), baseStream(s), type(t){}

        template<typename T>
        CaptureStream& operator<<(T const& data){
            baseStream.get() << data;
            {
                std::scoped_lock<std::mutex> const lock(parent->outputLogMutex);

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
                for (auto const& line : lines){
                    parent->outputLog.push_back({line, type});
                }
            }
            return *this;
        }

        CaptureStream& operator<<(char const* data){
            // Cast to std::string, then call templated operator
            return (*this) << std::string(data);
        }
    };

    /**
     * @brief Retrieves the singleton instance of Capture.
     * @return Reference to the singleton Capture instance.
     */
    static Capture& instance(){
        static Capture singleton;
        return singleton;
    }

    /**
     * @brief Retrieves the CaptureStream for cout.
     * @return Reference to the CaptureStream for cout.
     */
    static CaptureStream& cout(){ return instance().coutStream; }

    /**
     * @brief Retrieves the CaptureStream for cerr.
     * @return Reference to the CaptureStream for cerr.
     */
    static CaptureStream& cerr(){ return instance().cerrStream; }

    /**
     * @brief Retrieves a pointer to the output log.
     * @return A pointer to the output log deque, const.
     */
    std::deque<OutputLine> const& getOutputLogPtr() const {
        return outputLog;
    }

    /**
     * @brief Clears the output log.
     */
    static void clear(){
        instance().outputLog.clear();
    }

private:
    // Make constructor private for singleton
    Capture() : coutStream(this, std::cout, OutputLine::Type::COUT), cerrStream(this, std::cerr, OutputLine::Type::CERR){}

    CaptureStream coutStream{this, std::cout, OutputLine::Type::COUT};
    CaptureStream cerrStream{this, std::cerr, OutputLine::Type::CERR};

    std::deque<OutputLine> outputLog; // Log of captured output lines
    std::mutex outputLogMutex;  // Mutex for thread-safe access to outputLog
};
} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_CAPTURE_HPP