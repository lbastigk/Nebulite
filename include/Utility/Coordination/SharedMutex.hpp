#ifndef NEBULITE_UTILITY_SHARED_MUTEX_HPP
#define NEBULITE_UTILITY_SHARED_MUTEX_HPP

//------------------------------------------
// Includes

#include <mutex>
#include <memory>

#if !defined(_WIN32)
  #include <shared_mutex>
#endif // !defined(_WIN32)

//------------------------------------------
namespace Nebulite::Utility::Coordination {

// Perhaps using a different flag: NEBULITE_WINE_COMPATIBILITY or similar would be better
// as the shared mutax should work on native Windows builds.
#if defined(_WIN32)

/**
 * @class Nebulite::Utility::Coordination::SharedMutex
 * @brief A compatibility SharedMutex class for Windows platforms.
 * @details This class emulates the behavior of a shared mutex using a standard mutex.
 *          Shared locking operations are treated as exclusive locks.
 *          Reason: Some Wine versions have issues with std::shared_mutex.
 */
class SharedMutex {
public:
    void lock()   { m_.lock(); }
    void unlock() { m_.unlock(); }

    // treat shared operations as exclusive on Windows
    void lock_shared()   { m_.lock(); }
    void unlock_shared() { m_.unlock(); }

private:
    std::mutex m_;
};

using SharedLock = std::unique_lock<SharedMutex>;
using ReadLock   = std::unique_lock<SharedMutex>;
using WriteLock  = std::unique_lock<SharedMutex>;

#else // defined(_WIN32)

// On non-Windows platforms, use the standard shared_mutex

using SharedMutex = std::shared_mutex;
using SharedLock  = std::unique_lock<SharedMutex>;
using ReadLock    = std::shared_lock<SharedMutex>;
using WriteLock   = std::unique_lock<SharedMutex>;

#endif // defined(_WIN32)
} // namespace Nebulite::Utility::Coordination
#endif // NEBULITE_UTILITY_SHARED_MUTEX_HPP
