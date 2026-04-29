#ifndef UTILITY_IO_CAPTURE_TPP
#define UTILITY_IO_CAPTURE_TPP

//------------------------------------------
// Includes

// Standard library
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Utility::IO {

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

template<std::ostream* BaseStream, HistoryLine::Type LineType>
template<typename... Args>
void HierarchicalStream<BaseStream, LineType>::print(Args&&... args){
    if (parent) {
        // Pass to parent stream for retention
        parent->print(args...);
    }
    // Only print to console if this is the root stream, to avoid duplicate prints
    coutStream.print(!parent, std::forward<Args>(args)...);
}

template<std::ostream* BaseStream, HistoryLine::Type LineType>
template<typename... Args>
void HierarchicalStream<BaseStream, LineType>::println(Args&&... args){
    if (parent) {
        // Pass to parent stream for retention
        parent->println(args...);
    }
    // Only print to console if this is the root stream, to avoid duplicate prints
    coutStream.println(!parent, std::forward<Args>(args)...);
}

} // namespace Nebulite::Utility::IO
#endif // UTILITY_IO_CAPTURE_TPP
