#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"
#include "Module/Domain/GlobalSpace/General.hpp"
#include "Utility/IO/FileManagement.hpp"

namespace Nebulite::Module::Domain::GlobalSpace {

//------------------------------------------
// Update
Constants::Event General::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event General::exit() const {
    // Clear all task queues to prevent further execution
    domain.clearAllTaskQueues();

    // Set the renderer to quit
    domain.quitRenderer();
    return Constants::Event::Success;
}

Constants::Event General::wait(int const argc, char** argv) const {
    if (argc == 2) {
        // Standard wait acts on taskQueue "script"
        domain.getTaskQueue(Core::GlobalSpace::StandardTasks::script)->incrementWaitCounter(std::stoull(argv[1]));
        return Constants::Event::Success;
    }
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
}

Constants::Event General::task(int const argc, char** argv) const {
    domain.capture.log.println("Loading task list from file: ", argc > 1 ? std::string(argv[1]) : "none");

    // Rollback RNG, loading a task file should not change the RNG state
    domain.rngRollback();

    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (argc > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    // Warn if file ending is not .nebs
    std::string const filename = argv[1];
    if (filename.length() < 6 || !filename.ends_with(".nebs")) {
        domain.capture.error.println("Warning: unexpected file ending for task file '", filename, "'. Expected '.nebs'. Trying to load anyway.");
    }

    // Using FileManagement to load the .nebs file
    std::string file = Utility::IO::FileManagement::LoadFile(filename);
    if (file.empty()) {
        domain.capture.error.println("Error: ", argv[0], " Could not open file '", filename, "'.");
        return Constants::StandardCapture::Error::File::invalidFile(domain.capture);
    }

    // Replace all "\n " with "\n" to allow for multi-line commands with leading spaces
    while (file.find("\n ") != std::string::npos) {
        file = Utility::StringHandler::replaceAll(file, "\n ", "\n");
    }

    // Replace all " \\\n" with "\\\n" to allow for multi-line commands with trailing spaces
    while (file.find(" \\\n") != std::string::npos) {
        file = Utility::StringHandler::replaceAll(file, " \\\n", "\\\n");
    }

    // Replace all "\\n" with an empty string to allow for multi-line commands in a single line
    auto constexpr toReplace = "\\\n";
    file = Utility::StringHandler::replaceAll(file, toReplace, "");

    // Split std::string file into lines and remove comments
    std::vector<std::string> lines;
    std::istringstream stream(file);
    std::string line;
    while (std::getline(stream, line)) {
        line = Utility::StringHandler::untilSpecialChar(line, '#'); // Remove comments.
        line = Utility::StringHandler::lStrip(line, ' '); // Remove whitespaces at start
        line = Utility::StringHandler::rStrip(line, ' '); // Remove whitespaces at end
        if (line.empty()) {
            // line is empty
            continue;
        }
        // Insert line backwards, so we can process them in the order they were written later on:
        lines.insert(lines.begin(), line);
    }

    // Now insert all lines into the task queue
    for (auto const& taskLine : lines) {
        domain.getTaskQueue(Core::GlobalSpace::StandardTasks::script)->pushFront(taskLine);
    }
    return Constants::Event::Success;
}

Constants::Event General::always(int argc, char** argv) const {
    if (argc > 1) {
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i) {
            if (i > 1)
                oss << ' ';
            oss << argv[i];
        }

        // Split oss.str() on ';' and push each trimmed command
        std::string argStr = oss.str();
        std::stringstream ss(argStr);
        std::string command;

        while (std::getline(ss, command, ';')) {
            // Trim whitespace from each command
            command.erase(0, command.find_first_not_of(" \t"));
            command.erase(command.find_last_not_of(" \t") + 1);
            if (!command.empty()) {
                domain.getTaskQueue(Core::GlobalSpace::StandardTasks::always)->pushBack(command);
            }
        }
    }
    return Constants::Event::Success;
}

Constants::Event General::alwaysClear() const {
    domain.getTaskQueue(Core::GlobalSpace::StandardTasks::always)->clear();
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::GlobalSpace
