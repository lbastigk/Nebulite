//------------------------------------------
// Includes

// Standard library
#include <deque>
#include <mutex>
#include <numeric>
#include <string>

// Nebulite
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Utility::IO {

std::deque<HistoryLine> const& Capture::getHistory() const {
    return localHistory.getLines();
}

Capture::Capture(Capture* parent)
    : log(this, parent ? &parent->log : noParent)
    , warning(this, parent ? &parent->warning : noParent)
    , error(this, parent ? &parent->error : noParent) {}

void Capture::clear(){
    localHistory.clear();
}

bool Capture::hasParent() const {
    // Doesn't matter what stream we check
    return log.hasParent();
}

void Capture::appendToHistory(std::string const& str, HistoryLine::Type const lineType) {
    std::scoped_lock const lock(historyMutex);
    if (redirectorStack.empty()) {
        localHistory.addHistoryLine(str, lineType);
    }
    else {
        redirectorStack.back().addHistoryLine(str, lineType);
    }
}

// Capture History

bool Capture::History::appendableToLastLine(HistoryLine::Type const lineType){
    if (lines.empty()) {
        return false;
    }
    auto& back = lines.back();
    return !startNewLine && back.type == lineType;
}

[[nodiscard]] std::deque<HistoryLine> const& Capture::History::getLines() const {
    return lines;
}

void Capture::History::clear() {
    lines.clear();
}

std::string Capture::History::toString() {
    return std::accumulate(std::begin(lines), std::end(lines), std::string{}, [](std::string const& acc, HistoryLine const& line) {
        return acc + line.content;
    });
}

void Capture::History::addHistoryLine(std::string const& str, HistoryLine::Type lineType){
    if (appendableToLastLine(lineType)) {
        lines.back().content.append(str);
    }
    else {
        lines.push_back({.content=str, .type=lineType});
    }
    startNewLine = false;

    // Handle newlines
    if (lines.back().content.ends_with('\n')) {
        lines.back().content.pop_back();
        startNewLine = true;
    }
}

} // namespace Nebulite::Utility::IO
