#include "Constants/ErrorTypes.hpp"

namespace Nebulite::Constants {

ErrorTable& ErrorTable::getInstance() {
    static ErrorTable instance;
    return instance;
}

Error ErrorTable::addErrorImpl(std::string const& description, Error::Type type) {
    if (count == UINT16_MAX) {
        // Too many errors, exit entirely with message
        throw std::runtime_error("ErrorTable has reached its maximum capacity of errors. Make sure that new errors added are removed after some time if they are not needed anymore.");
    }
    // Store the description in the owned container and reference it from
    // the Error object. Use deque to guarantee stable element addresses.
    localDescriptions.push_back(description);
    errors.emplace_back(&localDescriptions.back(), type);
    count++;
    return errors.back();
}

} // namespace Nebulite::Constants
