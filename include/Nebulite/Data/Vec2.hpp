#ifndef NEBULITE_DATA_VEC2_HPP
#define NEBULITE_DATA_VEC2_HPP

//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <type_traits>

//------------------------------------------
// Concepts

template<typename T>
concept VectorType = std::is_arithmetic_v<T> && std::is_trivially_copyable_v<T>;

//------------------------------------------
namespace Nebulite::Data {
template<VectorType T>
struct Vec2 {
    T x;
    T y;

    template<VectorType X, VectorType Y>
    explicit Vec2(const X& x, const Y& y) : x(x), y(y) {
        static_assert(std::is_same_v<T, std::remove_cv_t<std::remove_reference_t<X>>>, "Type of x must match this Vec2 type");
        static_assert(std::is_same_v<T, std::remove_cv_t<std::remove_reference_t<Y>>>, "Type of y must match this Vec2 type");
    }

    template<VectorType U>
    Vec2 operator+(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return Vec2{x + other.x, y + other.y};
    }

    template<VectorType U>
    Vec2 operator-(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return Vec2{x - other.x, y - other.y};
    }

    template<VectorType U>
    bool operator==(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return x == other.x && y == other.y;
    }

    template<VectorType U>
    bool operator!=(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return x != other.x || y != other.y;
    }

    template <typename H>
    friend H AbslHashValue(H h, const Vec2& coord) {
        return H::combine(std::move(h), coord.x, coord.y);
    }

    /**
     * @brief Converts this Vec2 to a Vec2 of a different type.
     * @tparam NewType The new type to convert to.
     * @return A Vec2 of the specified NewType with the same x and y values, cast to NewType.
     */
    template<VectorType NewType>
    Vec2<NewType> convertTo() {
        return Vec2<NewType>(static_cast<NewType>(x), static_cast<NewType>(y));
    }

    /**
     * @brief Calculates the Euclidean distance between this vector and another vector.
     * @param other The other vector to calculate the distance to.
     * @return The Euclidean distance between the two vectors.
     * @note This function is only available for floating-point types.
     */
    T euclideanDistance(Vec2 const& other) const requires(std::is_floating_point_v<T>) {
        T dx = x - other.x;
        T dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    /**
     * @brief Calculates the Manhattan distance between this vector and another vector.
     * @param other The other vector to calculate the distance to.
     * @return The Manhattan distance between the two vectors.
     * @note This function is only available for non-floating-point types.
     */
    T manhattanDistance(Vec2 const& other) const requires(!std::is_floating_point_v<T>) {
        T dx = x - other.x;
        T dy = y - other.y;
        return std::abs(dx) + std::abs(dy);
    }

    /**
     * @brief Calculates the Chebyshev distance between this vector and another vector.
     * @param other The other vector to calculate the distance to.
     * @return The Chebyshev distance between the two vectors.
     * @note This function is only available for non-floating-point types.
     */
    T chebyshevDistance(Vec2 const& other) const requires(!std::is_floating_point_v<T>) {
        T dx = std::abs(x - other.x);
        T dy = std::abs(y - other.y);
        return std::max(dx, dy);
    }
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_VEC2_HPP
