#include "Utility/IO/Capture.hpp"

namespace Nebulite::Utility::IO {

std::deque<HistoryLine> const& Capture::getHistory() const {
    return history;
}

Capture::Capture(Capture* parent)
    : log(this, parent ? &parent->log : noParent),
      warning(this, parent ? &parent->warning : noParent),
      error(this, parent ? &parent->error : noParent)
{}

void Capture::clear(){
    history.clear();
}

bool Capture::hasParent() const {
    // Doesn't matter what stream we check
    return log.hasParent();
}

void Capture::appendInput(std::string const& str) {
    std::scoped_lock const lock(historyMutex);
    history.push_back({str, HistoryLine::Type::Input});
}


} // namespace Nebulite::Utility::IO
