//------------------------------------------
// Includes

// Nebulite
#include "DomainModule/Renderer/Console.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

Constants::Error Console::consoleOpen() {
    consoleMode = true;
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleClose() {
    consoleMode = false;
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Autotype functions

Constants::Error Console::consoleAutotypeText(std::span<std::string const> const& args) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    autoType.queue.push(AutoType::Command{AutoType::Command::Type::TEXT, Utility::StringHandler::recombineArgs(args.subspan(1))});
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeEnter() {
    autoType.queue.push({AutoType::Command::Type::ENTER, ""});
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeExecute() {
    // add queue to active queue
    while (!autoType.queue.empty()) {
        autoType.activeQueue.push(autoType.queue.front());
        autoType.queue.pop();
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeWait(std::span<std::string const> const& args) {
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    autoType.queue.push({AutoType::Command::Type::WAIT, args[1]});
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeClose() {
    autoType.queue.push({AutoType::Command::Type::CLOSE, ""});
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeHistoryUp() {
    autoType.queue.push({AutoType::Command::Type::HISTORY_UP, ""});
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeHistoryDown() {
    autoType.queue.push({AutoType::Command::Type::HISTORY_DOWN, ""});
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeClear() {
    // Clear both queues
    while (!autoType.queue.empty()) {
        autoType.queue.pop();
    }
    while (!autoType.activeQueue.empty()) {
        autoType.activeQueue.pop();
    }
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::Renderer
