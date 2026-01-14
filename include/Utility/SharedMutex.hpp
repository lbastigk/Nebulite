// cpp
#ifndef NEBULITE_COMPAT_SHARED_MUTEX_HPP
#define NEBULITE_COMPAT_SHARED_MUTEX_HPP

#include <mutex>
#include <memory>

#if !defined(_WIN32)
  #include <shared_mutex>
#endif

namespace Nebulite::Utility {

// Perhaps using a different flag: NEBULITE_WINE_COMPATIBILITY or similar would be better
// as the shared mutax should work on native Windows builds.
#if defined(_WIN32)

// On Windows/Wine fall back to exclusive mutex.
// Provides lock/unlock/lock_shared/unlock_shared with same semantics needed.
/**
 * @brief A compatibility SharedMutex class for Windows platforms.
 * @details This class emulates the behavior of a shared mutex using a standard mutex.
 *          Shared locking operations are treated as exclusive locks.
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

#else

// On non-Windows platforms, use the standard shared_mutex

using SharedMutex = std::shared_mutex;
using SharedLock  = std::unique_lock<SharedMutex>;
using ReadLock    = std::shared_lock<SharedMutex>;
using WriteLock   = std::unique_lock<SharedMutex>;

#endif

} // namespace nebulite::compat

#endif // NEBULITE_COMPAT_SHARED_MUTEX_HPP
