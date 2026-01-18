//------------------------------------------
// Includes

// Standard library
#include <csignal>

// Nebulite
#include "Nebulite.hpp"
#include "Core/RenderObject.hpp"
#include "DomainModule/GlobalSpace/Debug.hpp"
#include "Utility/TimedRoutine.hpp"

//------------------------------------------
#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>   // isatty, lstat
#include <sys/stat.h> // struct stat, S_ISLNK
#endif
#include <memory>
#include <fstream>

//------------------------------------------
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

//------------------------------------------
// Update
Constants::Error Debug::update() {
    //------------------------------------------
    // Memory usage


    static Utility::TimedRoutine memoryUsageUpdater(
        [this] {
            // store memory usage in global document
            double virtualMemMB = 0.0;
            double residentMemMB = 0.0;
            getMemoryUsageMB(virtualMemMB, residentMemMB);
            moduleScope.set<double>(Data::ScopedKey(moduleScope.getRootScope() + "memory.virtualMB"), virtualMemMB);
            moduleScope.set<double>(Data::ScopedKey(moduleScope.getRootScope() + "memory.residentMB"), residentMemMB);
        },
        1000 /*ms*/, // Call every second
        Utility::TimedRoutine::ConstructionMode::START_IMMEDIATELY
    );
    memoryUsageUpdater.update();

    //------------------------------------------
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Constants::Error Debug::log_global(int const argc, char** argv) const {
    std::string const serialized = moduleScope.serialize();
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            Utility::FileManagement::WriteFile(argv[i], serialized);
        }
    } else {
        Utility::FileManagement::WriteFile("global.log.jsonc", serialized);
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error Debug::log_state(int const argc, char** argv) const {
    std::string const serialized = domain.getRenderer().serialize();
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            Utility::FileManagement::WriteFile(argv[i], serialized);
        }
    } else {
        Utility::FileManagement::WriteFile("state.log.jsonc", serialized);
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error Debug::standardfileRenderobject(std::span<std::string const> const& /*args*/){
    Core::RenderObject const ro;
    Utility::FileManagement::WriteFile("./Resources/Renderobjects/standard.jsonc", ro.serialize());
    return Constants::ErrorTable::NONE();
}

Constants::Error Debug::errorlog(int const argc, char** argv) {
    // Initialize the error logging buffer
    if (!originalCerrBuf) {
        originalCerrBuf = std::cerr.rdbuf();
    }

    if (argc == 2) {
        if (!strcmp(argv[1], "on")) {
            if (!errorLogStatus) {
                if (!safe_open_log(errorFile)) {
                    Error::println("Refusing to open log file: '", logFilename, "' is a symlink or could not be opened.");
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

Constants::Error Debug::clearConsole(std::span<std::string const> const& /*args*/) {
    clear_screen();
    Utility::Capture::clear();
    return Constants::ErrorTable::NONE();
}

Constants::Error Debug::crash(std::span<std::string const> const& args) {
    // If an argument is provided, use it to select crash type
    if (args.size() > 1) {
        if (std::string const& crashType = args[1]; crashType == "segfault") {
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
            Error::println("Unknown crash type requested: ", crashType);
            Error::println("Defaulting to segmentation fault");
        }
    } else {
        // Default: segmentation fault
        raise(SIGSEGV);
    }
    // Should never reach here
    return Constants::ErrorTable::NONE();
}

Constants::Error Debug::error(std::span<std::string const> const& args) {
    auto const& argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    Error::println(argStr);
    return Constants::ErrorTable::NONE();
}

Constants::Error Debug::warn(std::span<std::string const> const& args) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    return Constants::ErrorTable::addError(argStr, Constants::Error::NON_CRITICAL);
}

Constants::Error Debug::critical(std::span<std::string const> const& args) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    return Constants::ErrorTable::addError(argStr, Constants::Error::CRITICAL);
}

Constants::Error Debug::waitForInput(std::span<std::string const> const& args) {
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string message = "Press Enter to continue...";
    if (args.size() == 2) {
        // Use the provided prompt as message
        message = args[1];
    }
    Log::println(message);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Private Methods

void Debug::setupPlatformInfo() const {
#ifdef _WIN32
    moduleScope.set<std::string>(Data::ScopedKey("platform"), "windows");
#elif __linux__
    moduleScope.set<std::string>(Data::ScopedKey("platform"), "linux");
#elif __APPLE__
    moduleScope.set<std::string>(Data::ScopedKey("platform"), "macos");
#elif __FreeBSD__
    moduleScope.set<std::string>(Data::ScopedKey("platform"), "freebsd");
#elif __unix__
    moduleScope.set<std::string>(Data::ScopedKey("platform"), "unix");
#elif __ANDROID__
    moduleScope.set<std::string>(Data::ScopedKey("platform"), "android");
#elif __TEMPLEOS__
    printf("Glory be to TempleOS!\n");
    moduleScope.set<std::string>(Data::ScopedKey("platform"), "templeos");
#else
    moduleScope.set<std::string>(Data::ScopedKey("platform"), "unknown");
#endif
}

} // namespace Nebulite::DomainModule::GlobalSpace
