#ifndef NEBULITE_UTILITY_COORDINATION_LAZYINIT_HPP
#define NEBULITE_UTILITY_COORDINATION_LAZYINIT_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <optional>
#include <utility>

//------------------------------------------
namespace Nebulite::Utility::Coordination {

/**
 * @brief A utility class for lazy initialization of objects.
 * @details Small inline size variant using std::unique_ptr instead of std::optional.
 * @tparam T The type of the object to be lazily initialized.
 * @tparam ConstructorArgs The types of the arguments for the constructor.
 */
template<typename T, typename... ConstructorArgs>
class LazyInit {
    mutable std::unique_ptr<T> ptr;

public:
    LazyInit() = default;

    /**
     * @brief Returns a reference to the lazily initialized object, constructing it if it doesn't exist.
     * @param args The arguments for the constructor.
     * @return A reference to the lazily initialized object.
     */
    T& get(ConstructorArgs... args) const {
        if (!ptr) {
            ptr = std::make_unique<T>(std::forward<ConstructorArgs>(args)...);
        }
        return *ptr;
    }

    /**
     * @brief Constructs or reconstructs the stored object with the given arguments.
     * @details Any object currently held by this instance is destroyed before the new
     *          object is stored.
     * @tparam Args The types of the arguments for the constructor.
     * @param args The arguments for the constructor.
     */
    template<typename... Args>
    void emplace(Args&&... args) {
        ptr = std::make_unique<T>(std::forward<Args>(args)...);
    }
};

/**
 * @brief A utility class for lazy initialization of objects.
 * @details Full inline size variant using std::optional.
 * @tparam T The type of the object to be lazily initialized.
 * @tparam ConstructorArgs The types of the arguments for the constructor.
 */
template<typename T, typename... ConstructorArgs>
class LazyInitOptional {
    mutable std::optional<T> opt;

public:
    LazyInitOptional() = default;

    /**
     * @brief Returns a reference to the lazily initialized object, constructing it if it doesn't exist.
     * @param args The arguments for the constructor.
     * @return A reference to the lazily initialized object.
     */
    T& get(ConstructorArgs... args) const {
        if (!opt) {
            opt.emplace(std::forward<ConstructorArgs>(args)...);
        }
        return opt.value();
    }

    /**
     * @brief Constructs or reconstructs the stored object with the given arguments.
     * @details Any object currently held by this instance is destroyed before the new
     *          object is stored.
     * @tparam Args The types of the arguments for the constructor.
     * @param args The arguments for the constructor.
     */
    template<typename... Args>
    void emplace(Args&&... args) {
        opt.emplace(std::forward<Args>(args)...);
    }
};

} // namespace Nebulite::Utility::Coordination
#endif // NEBULITE_UTILITY_COORDINATION_LAZYINIT_HPP
