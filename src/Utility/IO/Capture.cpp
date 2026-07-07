//------------------------------------------
// Includes

// Standard library
#include <deque>
#include <mutex>
#include <string>

// Nebulite
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Utility::IO {

std::deque<HistoryLine> const& Capture::getHistory() const {
    return history;
}

Capture::Capture(Capture* parent)
    : log(this, parent ? &parent->log : noParent)
    , warning(this, parent ? &parent->warning : noParent)
    , error(this, parent ? &parent->error : noParent) {}

void Capture::clear(){
    history.clear();
}

bool Capture::hasParent() const {
    // Doesn't matter what stream we check
    return log.hasParent();
}

void Capture::appendToHistory(std::string const& str, HistoryLine::Type const lineType) {
    std::scoped_lock const lock(historyMutex);
    history.push_back({.content=str, .type=lineType, .silent = outputEnabled});
}

} // namespace Nebulite::Utility::IO
