#ifndef NEBULITE_MATH_VEC2_HPP
#define NEBULITE_MATH_VEC2_HPP

//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <type_traits>

// Nebulite
#include "Nebulite/Math/Coordinates.hpp"

//------------------------------------------
// Concepts

template<typename T>
concept VectorType = std::is_arithmetic_v<T> && std::is_trivially_copyable_v<T>;

//------------------------------------------
namespace Nebulite::Math {

//------------------------------------------
// Vector implementations

template<VectorType T, CoordinateType C = CoordinateType::xy>
struct Vec2;

template<VectorType T>
struct Vec2<T,CoordinateType::xy> {
    T x;
    T y;

    template<VectorType U>
    Vec2 operator+(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return Vec2{
            .x=x + other.x,
            .y=y + other.y,
        };
    }

    template<VectorType U>
    Vec2 operator-(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return Vec2{
            .x=x - other.x,
            .y=y - other.y,
        };
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
    friend H AbslHashValue(H hv, Vec2 const& coord) { // NOLINT
        return H::combine(std::move(hv), coord.x, coord.y);
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
    [[nodiscard]] T euclideanDistance(Vec2 const& other) const requires(std::is_floating_point_v<T>) {
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
    [[nodiscard]] T manhattanDistance(Vec2 const& other) const requires(!std::is_floating_point_v<T>) {
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
    [[nodiscard]] T chebyshevDistance(Vec2 const& other) const requires(!std::is_floating_point_v<T>) {
        T dx = std::abs(x - other.x);
        T dy = std::abs(y - other.y);
        return std::max(dx, dy);
    }
};

template<VectorType T>
struct Vec2<T, CoordinateType::wh> {
    T w;
    T h;

    template<VectorType U>
    Vec2 operator+(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return Vec2{
            .w=w + other.w,
            .h=h + other.h,
        };
    }

    template<VectorType U>
    Vec2 operator-(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return Vec2{
            .w = w - other.w,
            .h = h - other.h,
        };
    }

    template<VectorType U>
    bool operator==(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return w == other.w && h == other.h;
    }

    template<VectorType U>
    bool operator!=(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return w != other.w || h != other.h;
    }

    template <typename H>
    friend H AbslHashValue(H hv, Vec2 const& coord) { // NOLINT
        return H::combine(std::move(hv), coord.w, coord.h);
    }

    /**
     * @brief Converts this Vec2 to a Vec2 of a different type.
     * @tparam NewType The new type to convert to.
     * @return A Vec2 of the specified NewType with the same w and h values, cast to NewType.
     */
    template<VectorType NewType>
    Vec2<NewType> convertTo() const {
        return Vec2<NewType>(static_cast<NewType>(w), static_cast<NewType>(h));
    }

    /**
     * @brief Calculates the Euclidean distance between this vector and another vector.
     * @param other The other vector to calculate the distance to.
     * @return The Euclidean distance between the two vectors.
     * @note This function is only available for floating-point types.
     */
    [[nodiscard]] T euclideanDistance(Vec2 const& other) const requires(std::is_floating_point_v<T>) {
        T dx = w - other.w;
        T dy = h - other.h;
        return std::sqrt(dx * dx + dy * dy);
    }

    /**
     * @brief Calculates the Manhattan distance between this vector and another vector.
     * @param other The other vector to calculate the distance to.
     * @return The Manhattan distance between the two vectors.
     * @note This function is only available for non-floating-point types.
     */
    [[nodiscard]] T manhattanDistance(Vec2 const& other) const requires(!std::is_floating_point_v<T>) {
        T dx = w - other.w;
        T dy = h - other.h;
        return std::abs(dx) + std::abs(dy);
    }

    /**
     * @brief Calculates the Chebyshev distance between this vector and another vector.
     * @param other The other vector to calculate the distance to.
     * @return The Chebyshev distance between the two vectors.
     * @note This function is only available for non-floating-point types.
     */
    [[nodiscard]] T chebyshevDistance(Vec2 const& other) const requires(!std::is_floating_point_v<T>) {
        T dx = std::abs(w - other.w);
        T dy = std::abs(h - other.h);
        return std::max(dx, dy);
    }
};

} // namespace Nebulite::Math
#endif // NEBULITE_MATH_VEC2_HPP
