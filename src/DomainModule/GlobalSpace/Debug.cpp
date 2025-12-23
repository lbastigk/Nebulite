#include "DomainModule/GlobalSpace/Debug.hpp"

#include <csignal>

#include "Nebulite.hpp"

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>   // isatty, lstat
#include <sys/stat.h> // struct stat, S_ISLNK
#endif
#include <memory>
#include <fstream>

// Platform-specific functions
namespace {
/**
 * @brief Filename for error logging
 */
auto const* logFilename = "errors.log";

/**
 * @brief Safely opens a log file for writing, ensuring it is not a symlink.
 *
 * @todo Move this functionality to globalspace
 */
bool safe_open_log(std::unique_ptr<std::ofstream>& out) {

#if defined(_WIN32)
    DWORD attrs = GetFileAttributesA(logFilename);
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_REPARSE_POINT)) {
        return false;
    }
#else
    struct stat sb = {};
    if (lstat(logFilename, &sb) == 0 && S_ISLNK(sb.st_mode)) {
        return false;
    }
#endif
    out = std::make_unique<std::ofstream>(logFilename);
    return out->is_open();
}

void getMemoryUsageMB(double& virtualMemMB, double& residentMemMB) {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    virtualMemMB = static_cast<double>(pmc.PrivateUsage) / (1024.0 * 1024.0);
    residentMemMB = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
#else
    // derived from
    // https://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-runtime-using-c

    virtualMemMB = 0.0;
    residentMemMB = 0.0;

    // 'file' stat seems to give the most reliable results
    std::ifstream stat_stream("/proc/self/stat", std::ios_base::in);

    // dummy vars for leading entries in stat that we don't care about
    //
    std::string pid, comm, state, ppid, pgrp, session, tty_nr;
    std::string tpgid, flags, minflt, cminflt, majflt, cmajflt;
    std::string utime, stime, cutime, cstime, priority, nice;
    std::string O, itrealvalue, starttime;

    // the two fields we want
    unsigned long vsize;
    long rss;

    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
        >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
        >> utime >> stime >> cutime >> cstime >> priority >> nice
        >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

    stat_stream.close();

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    virtualMemMB = static_cast<double>(vsize) / (1024.0 * 1024.0);
    residentMemMB = static_cast<double>(rss) * static_cast<double>(page_size_kb) / 1024.0;
#endif
}
} // anonymous namespace

namespace Nebulite::DomainModule::GlobalSpace {
std::string const Debug::log_name = "log";
std::string const Debug::log_desc = R"(Functions to log various data to files)";

std::string const Debug::standardfile_name = "standardfile";
std::string const Debug::standardfile_desc = R"(Functions to generate standard files)";

//------------------------------------------
// Update
Constants::Error Debug::update() {
    //------------------------------------------
    // Memory usage

    // store memory usage in global document

    // Call every second
    static Utility::TimeKeeper memoryUsagePoller;
    if (memoryUsagePoller.projected_dt() > 1000 || !memoryUsagePoller.is_running()) {
        double virtualMemMB = 0.0;
        double residentMemMB = 0.0;
        getMemoryUsageMB(virtualMemMB, residentMemMB);
        domain->getDoc()->set<double>("debug.memory.virtualMB", virtualMemMB);
        domain->getDoc()->set<double>("debug.memory.residentMB", residentMemMB);
    }
    if (!memoryUsagePoller.is_running()) {
        memoryUsagePoller.start();
    }

    //------------------------------------------
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

// NOLINTNEXTLINE
Constants::Error Debug::log_global(int argc, char** argv) {
    std::string const serialized = domain->getDoc()->serialize();
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            Utility::FileManagement::WriteFile(argv[i], serialized);
        }
    } else {
        Utility::FileManagement::WriteFile("global.log.jsonc", serialized);
    }
    return Constants::ErrorTable::NONE();
}

std::string const Debug::log_global_name = "log global";
std::string const Debug::log_global_desc = R"(Logs the global document to a file.

Usage: log global [<filenames>...]

- <filenames>: Optional. One or more filenames to log the global document to.
                If no filenames are provided, defaults to 'global.log.jsonc'.
)";

// NOLINTNEXTLINE
Constants::Error Debug::log_state(int argc, char** argv) {
    std::string const serialized = domain->getRenderer()->serialize();
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            Utility::FileManagement::WriteFile(argv[i], serialized);
        }
    } else {
        Utility::FileManagement::WriteFile("state.log.jsonc", serialized);
    }
    return Constants::ErrorTable::NONE();
}

std::string const Debug::log_state_name = "log state";
std::string const Debug::log_state_desc = R"(Logs the current state of the renderer to a file.

Usage: log state [<filenames>...]

- <filenames>: Optional. One or more filenames to log the renderer state to.
                If no filenames are provided, defaults to 'state.log.jsonc'.
)";

// NOLINTNEXTLINE
Constants::Error Debug::standardfile_renderobject(int argc, char** argv) {
    Core::RenderObject ro;
    Utility::FileManagement::WriteFile("./Resources/Renderobjects/standard.jsonc", ro.serialize());
    return Constants::ErrorTable::NONE();
}

std::string const Debug::standardfile_renderobject_name = "standardfile renderobject";
std::string const Debug::standardfile_renderobject_desc = R"(Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.

Usage: standardfile renderobject

Note: This function creates or overwrites the file 'standard.jsonc' in the './Resources/Renderobjects/' directory.
)";

// NOLINTNEXTLINE
Constants::Error Debug::errorlog(int argc, char** argv) {
    // Initialize the error logging buffer
    if (!originalCerrBuf) {
        originalCerrBuf = std::cerr.rdbuf();
    }

    if (argc == 2) {
        if (!strcmp(argv[1], "on")) {
            if (!errorLogStatus) {
                if (!safe_open_log(errorFile)) {
                    Nebulite::cerr() << "Refusing to open log file: '" << logFilename << "' is a symlink or could not be opened." << Nebulite::endl;
                    return Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
                }
                originalCerrBuf = std::cerr.rdbuf();
                std::cerr.rdbuf(errorFile->rdbuf());
                errorLogStatus = true;
            }
        } else if (!strcmp(argv[1], "off")) {
            if (errorLogStatus) {
                std::cerr.flush();
                std::cerr.rdbuf(originalCerrBuf);
                if (errorFile) {
                    errorFile->close();
                    errorFile.reset();
                }
                errorLogStatus = false;
            }
        }
    } else {
        if (argc > 2) {
            return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
        }
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    return Constants::ErrorTable::NONE();
}

std::string const Debug::errorlog_name = "errorlog";
std::string const Debug::errorlog_desc = R"(Activates or deactivates error logging to a file.

Usage: errorlog <on/off>

- on:  Activates error logging to 'error.log' in the working directory.
- off: Deactivates error logging, reverting to standard error output.
Note: Ensure you have write permissions in the working directory when activating error logging.
)";

inline void clear_screen() {
#if defined(_WIN32)
    // Use Win32 API to clear the console buffer and move cursor to home (0,0).
    HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStd == INVALID_HANDLE_VALUE)
        return;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hStd, &csbi))
        return;

    DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
    DWORD written = 0;
    COORD home = {0, 0};

    // Fill the entire buffer with spaces
    FillConsoleOutputCharacterA(hStd, ' ', cells, home, &written);
    // Reset attributes
    FillConsoleOutputAttribute(hStd, csbi.wAttributes, cells, home, &written);
    // Move the cursor home
    SetConsoleCursorPosition(hStd, home);

#else
    std::system("clear");

    // This might work better, if static analyzers criticize system() usage:
    /*
    std::cout << "\033[H\033[2J\033[3J" ;
    //*/

    // Old version, buggy in some terminals:
    /*
    // If stdout is a terminal, use ANSI escapes to clear the screen and move cursor to top-left.
    if (isatty(fileno(stdout))){
        // ESC[2J clears screen; ESC[H moves cursor to 1;1
        std::cout << "\x1b[2J\x1b[H" << std::flush;

    } else {
        // Not a TTY (redirected output). We can optionally print newlines or do nothing.
        // Printing newlines keeps behavior similar to clearing for plain output.
        std::cout << std::endl;
    }
    //*/
#endif
}

// NOLINTNEXTLINE
Constants::Error Debug::clearConsole(int argc, char** argv) {
    if (argc > 1) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    clear_screen();
    Utility::Capture::clear();
    return Constants::ErrorTable::NONE();
}

std::string const Debug::clearConsole_name = "clear";
std::string const Debug::clearConsole_desc = R"(Clears the console screen.

Usage: clear

Note: This function attempts to clear the console screen using system-specific commands.
        It may not work in all environments or IDEs.
)";

// NOLINTNEXTLINE
Constants::Error Debug::crash(int argc, char** argv) {
    // If an argument is provided, use it to select crash type
    if (argc > 1 && argv[1]) {
        if (std::string const crashType = argv[1]; crashType == "segfault") {
            // Cause a segmentation fault
            raise(SIGSEGV);
        } else if (crashType == "abort") {
            // Abort the program
            std::abort();
        } else if (crashType == "terminate") {
            // Terminate with std::terminate
            std::terminate();
        } else if (crashType == "throw") {
            // Throw an uncaught exception
            throw std::runtime_error("Intentional crash: uncaught exception");
        } else {
            Nebulite::cerr() << "Unknown crash type requested: " << crashType << Nebulite::endl;
            Nebulite::cerr() << "Defaulting to segmentation fault" << Nebulite::endl;
        }
    } else {
        // Default: segmentation fault
        raise(SIGSEGV);
    }
    // Should never reach here
    return Constants::ErrorTable::NONE();
}

std::string const Debug::crash_name = "crash";
std::string const Debug::crash_desc = R"(Crashes the program, useful for checking if the testing suite can catch crashes.

Usage: crash [<type>]

- <type>: Optional. The type of crash to induce. Options are:
    - segfault   : Causes a segmentation fault (default)
    - abort      : Calls std::abort()
    - terminate  : Calls std::terminate()
    - throw      : Throws an uncaught exception
)";

// NOLINTNEXTLINE
Constants::Error Debug::error(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        Nebulite::cerr() << argv[i];
        if (i < argc - 1) {
            Nebulite::cerr() << " ";
        }
    }
    Nebulite::cerr() << Nebulite::endl;

    // No further error to return
    return Constants::ErrorTable::NONE();
}

std::string const Debug::error_name = "error";
std::string const Debug::error_desc = R"(Echoes all arguments as string to the standard error.

Usage: error <string...>

- <string...>: One or more strings to echo to the standard error.
)";

// NOLINTNEXTLINE
Constants::Error Debug::warn(int argc, char** argv) {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string const args = Utility::StringHandler::recombineArgs(argc - 1, argv + 1);
    return Constants::ErrorTable::addError(args, Constants::Error::NON_CRITICAL);
}

std::string const Debug::warn_name = "warn";
std::string const Debug::warn_desc = R"(Returns a warning: a custom, noncritical error.

Usage: warn <string>

- <string>: The warning message.
)";

// NOLINTNEXTLINE
Constants::Error Debug::critical(int argc, char** argv) {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string const args = Utility::StringHandler::recombineArgs(argc - 1, argv + 1);
    return Constants::ErrorTable::addError(args, Constants::Error::CRITICAL);
}

std::string const Debug::critical_name = "critical";
std::string const Debug::critical_desc = R"(Returns a critical error.

Usage: critical <string>

- <string>: The critical error message.
)";

// NOLINTNEXTLINE
Constants::Error Debug::waitForInput(int argc, char** argv) {
    if (argc > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string message = "Press Enter to continue...";
    if (argc == 2) {
        // Use the provided prompt as message
        message = argv[1];
    }
    Nebulite::cout() << message << Nebulite::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return Constants::ErrorTable::NONE();
}

std::string const Debug::waitForInput_name = "input-wait";
std::string const Debug::waitForInput_desc = R"(Waits for user input before continuing.

Usage: input-wait [prompt]

Note: This function pauses execution until the user presses Enter.
)";

//------------------------------------------
// Private Methods

void Debug::setupPlatformInfo() {
#ifdef _WIN32
    domain->getDoc()->set<std::string>("platform", "windows");
#elif __linux__
    domain->getDoc()->set<std::string>("platform", "linux");
#elif __APPLE__
    domain->getDoc()->set<std::string>("platform", "macos");
#elif __FreeBSD__
    domain->getDoc()->set<std::string>("platform", "freebsd");
#elif __unix__
    domain->getDoc()->set<std::string>("platform", "unix");
#elif __ANDROID__
    domain->getDoc()->set<std::string>("platform", "android");
#elif __TEMPLEOS__
    printf("Glory be to TempleOS!\n");
    domain->getDoc()->set<std::string>("platform", "templeos");
#else
    domain->getDoc()->set<std::string>("platform", "unknown");
#endif
}

} // namespace Nebulite::DomainModule::GlobalSpace
