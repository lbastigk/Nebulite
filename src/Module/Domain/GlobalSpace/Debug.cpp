//------------------------------------------
// Includes

// Standard library
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <ios>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>

// Nebulite
#include "Constants/Event.hpp"
#include "Constants/StandardCapture.hpp"
#include "Constants/ThreadSettings.hpp"
#include "Core/GlobalSpace.hpp"
#include "Core/RenderObject.hpp"
#include "Interaction/Context.hpp"
#include "Math/ExpressionPrimitives.hpp"
#include "Module/Domain/Common/General.hpp"
#include "Module/Domain/GlobalSpace/Debug.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Utility/IO/FileManagement.hpp"

//------------------------------------------
#ifdef _WIN32
//#include <psapi.h>
#include <windows.h>
#else
#include <sys/stat.h> // struct stat, S_ISLNK
#include <unistd.h>   // isatty, lstat
#endif
#include <fstream>
#include <memory>

//------------------------------------------
// Platform-specific functions
namespace {
/**
 * @brief Filename for error logging
 */
auto const* logFilename = "errors.log";

/**
 * @brief Safely opens a log file for writing, ensuring it is not a symlink.
 */
bool safe_open_log(std::unique_ptr<std::ofstream>& out) {
#ifdef _WIN32
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
#ifdef _WIN32
    // Broken atm
    /*
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    virtualMemMB = static_cast<double>(pmc.PrivateUsage) / (1024.0 * 1024.0);
    residentMemMB = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    */
    virtualMemMB = std::numeric_limits<double>::quiet_NaN();
    residentMemMB = std::numeric_limits<double>::quiet_NaN();
#else
    // derived from
    // https://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-runtime-using-c

    virtualMemMB = 0.0;
    residentMemMB = 0.0;

    // 'file' stat seems to give the most reliable results
    std::ifstream stat_stream("/proc/self/stat", std::ios_base::in);

    // dummy vars for leading entries in stat that we don't care about
    //
    std::string pid;
    std::string comm;
    std::string state;
    std::string ppid;
    std::string pgrp;
    std::string session;
    std::string tty_nr;
    std::string tpgid;
    std::string flags;
    std::string minflt;
    std::string cminflt;
    std::string majflt;
    std::string cmajflt;
    std::string utime;
    std::string stime;
    std::string cutime;
    std::string cstime;
    std::string priority;
    std::string nice;
    std::string O;
    std::string itrealvalue;
    std::string starttime;

    // the two fields we want
    unsigned long vsize{};
    long rss{};

    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
        >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
        >> utime >> stime >> cutime >> cstime >> priority >> nice
        >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

    stat_stream.close();

    auto const page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    virtualMemMB = static_cast<double>(vsize) / (1024.0 * 1024.0);
    residentMemMB = static_cast<double>(rss) * static_cast<double>(page_size_kb) / 1024.0;
#endif
}
} // anonymous namespace

namespace Nebulite::Module::Domain::GlobalSpace {
//------------------------------------------
// Update
Constants::Event Debug::updateHook() {
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event Debug::log_global(int const argc, char const** argv) const {
    std::string const serialized = moduleScope.serialize();
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (!Utility::IO::FileManagement::WriteFile(argv[i], serialized)) {
                return Constants::StandardCapture::Error::File::couldNotWriteFile(domain.capture);
            }
        }
    } else {
        if (!Utility::IO::FileManagement::WriteFile("global.log.jsonc", serialized)) {
            return Constants::StandardCapture::Error::File::couldNotWriteFile(domain.capture);
        }
    }
    return Constants::Event::Success;
}

Constants::Event Debug::log_state(int const argc, char const** argv) const {
    std::string const serialized = domain.getRenderer().serialize();
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (!Utility::IO::FileManagement::WriteFile(argv[i], serialized)) {
                return Constants::StandardCapture::Error::File::couldNotWriteFile(domain.capture);
            }
        }
    } else {
        if (!Utility::IO::FileManagement::WriteFile("state.log.jsonc", serialized)) {
            return Constants::StandardCapture::Error::File::couldNotWriteFile(domain.capture);
        }
    }
    return Constants::Event::Success;
}

Constants::Event Debug::standardFileRenderObject(std::span<std::string_view const> const& /*args*/) const {
    if (Core::RenderObject const ro(domain.capture); !Utility::IO::FileManagement::WriteFile("./Resources/Renderobjects/standard.jsonc", ro.serialize())) {
        return Constants::StandardCapture::Error::File::couldNotWriteFile(domain.capture);
    }
    return Constants::Event::Success;
}

Constants::Event Debug::errorLog(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& /*ctxScope*/) {
    // Initialize the error logging buffer
    if (!originalCerrBuf) {
        originalCerrBuf = std::cerr.rdbuf();
    }

    if (args.size() == 2) {
        if (args[1] == "on") {
            if (!errorLogStatus) {
                if (!safe_open_log(errorFile)) {
                    ctx.self.capture.error.println("Refusing to open log file: '", logFilename, "' is a symlink or could not be opened.");
                    return Constants::StandardCapture::Error::File::invalidFile(domain.capture);
                }
                originalCerrBuf = std::cerr.rdbuf();
                std::cerr.rdbuf(errorFile->rdbuf());
                errorLogStatus = true;
            }
        } else if (args[1] == "off") {
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
        if (args.size() > 2) {
            return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
        }
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    return Constants::Event::Success;
}

namespace {
void clear_screen() {
#ifdef _WIN32
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
#endif
}
} // namespace


Constants::Event Debug::clearConsole(std::span<std::string_view const> const& /*args*/){
    clear_screen();
    Global::capture().clear();
    return Constants::Event::Success;
}

Constants::Event Debug::crash(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& /*ctxScope*/) {
    // If an argument is provided, use it to select crash type
    if (args.size() > 1) {
        if (auto const& crashType = args[1]; crashType == "segfault") {
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
            ctx.self.capture.error.println("Unknown crash type requested: ", crashType);
            ctx.self.capture.error.println("Defaulting to segmentation fault");
        }
    } else {
        // Default: segmentation fault
        raise(SIGSEGV);
    }
    // Should never reach here
    return Constants::Event::Success;
}

Constants::Event Debug::waitForInput(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& /*ctxScope*/) {
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    std::string message = "Press Enter to continue...";
    if (args.size() == 2) {
        // Use the provided prompt as message
        message = args[1];
    }
    ctx.self.capture.log.println(message);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return Constants::Event::Success;
}

Constants::Event Debug::listExpressionFunctions(std::span<std::string_view const> const& args) {
    // Forward to ExpressionPrimitives::help
    Math::ExpressionPrimitives::help(args);
    return Constants::Event::Success;
}

//------------------------------------------
// Private Methods

void Debug::addRoutines() {
    // Memory usage monitoring routine
    addRoutine(
        Utility::Coordination::TimedRoutine(
            [this] {
                // store memory usage in global document
                double virtualMemMB = 0.0;
                double residentMemMB = 0.0;
                getMemoryUsageMB(virtualMemMB, residentMemMB);
                moduleScope.set<double>(Key::memoryVirtualMB, virtualMemMB);
                moduleScope.set<double>(Key::memoryResidentMB, residentMemMB);
            },
            1000 /*ms*/, // Call every second
            Utility::Coordination::TimedRoutine::ConstructionMode::START_IMMEDIATELY
        ),
        RoutineUpdateMode::BEFORE_UPDATE_HOOK
    );

    addRoutine(
        Utility::Coordination::TimedRoutine(
            [this] {
                // store worker count in global document
                std::size_t const invokeWorkerCount = Constants::ThreadSettings::getInvokeWorkerCount();
                std::size_t const rendererWorkerCount = Constants::ThreadSettings::getRendererWorkerCount();
                std::size_t const workerCount = invokeWorkerCount + rendererWorkerCount;

                moduleScope.set<size_t>(Key::workerInvokeUsed, invokeWorkerCount);
                moduleScope.set<size_t>(Key::workerRendererUsed, rendererWorkerCount);
                moduleScope.set<size_t>(Key::workerTotalUsed, workerCount);

                moduleScope.set<size_t>(Key::workerInvokeMax, Constants::ThreadSettings::Maximum::invokeWorkerCount);
                moduleScope.set<size_t>(Key::workerRendererMax, Constants::ThreadSettings::Maximum::rendererWorkerCount);
                moduleScope.set<size_t>(Key::workerTotalMax, Constants::ThreadSettings::Maximum::totalThreadCount);
            },
            5000 /*ms*/, // Call every 5 seconds
            Utility::Coordination::TimedRoutine::ConstructionMode::START_IMMEDIATELY
        ),
        RoutineUpdateMode::BEFORE_UPDATE_HOOK
    );
}

void Debug::setupPlatformInfo() const {
#ifdef _WIN32
    moduleScope.set<std::string>(Key::platform, "windows");
#elif __linux__
    moduleScope.set<std::string>(Key::platform, "linux");
#elif __APPLE__
    moduleScope.set<std::string>(Key::platform, "macos");
#elif __FreeBSD__
    moduleScope.set<std::string>(Key::platform, "freebsd");
#elif __unix__
    moduleScope.set<std::string>(Key::platform, "unix");
#elif __ANDROID__
    moduleScope.set<std::string>(Key::platform, "android");
#elif __TEMPLEOS__
    printf("Glory be to TempleOS!\n");
    moduleScope.set<std::string>(Key::platform, "templeos");
#else
    moduleScope.set<std::string>(Key::platform, "unknown");
#endif
}

void Debug::setupDebugInfo() const {
    // Set build type based on preprocessor definitions
#if defined(COVERAGE) || defined(ENABLE_COVERAGE)
    moduleScope.set<std::string>(Key::buildType, "coverage");
#elif defined(_GLIBCXX_DEBUG) || defined(_ITERATOR_DEBUG_LEVEL) || !defined(NDEBUG)
    moduleScope.set<std::string>(Key::buildType, "debug");
#else
    moduleScope.set<std::string>(Key::buildType, "release");
#endif

    // Show debug window if in debug build
    if (moduleScope.get<std::string>(Key::buildType).value_or("") == "debug") {
        Interaction::Context ctx{domain, domain, domain};
        Interaction::ContextScope ctxScope{moduleScope, moduleScope, moduleScope};
        if (auto const event = domain.parseStr(__FUNCTION__ + std::string(" ") + Common::General::imguiView_Enable, ctx, ctxScope); event != Constants::Event::Success) {
            domain.capture.error.println("Error enabling ImGui view for GlobalSpace");
        }
    }
}

} // namespace Nebulite::Module::Domain::GlobalSpace
