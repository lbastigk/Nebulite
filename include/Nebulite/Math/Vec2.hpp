#ifndef NEBULITE_MATH_VEC2_HPP
#define NEBULITE_MATH_VEC2_HPP

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
namespace Nebulite::Math {

//------------------------------------------
// Types

enum class CoordinateType : bool {
    XY, // Coordinate names are x and y
    WH // Coordinate names are w and h
};

//------------------------------------------
// Vector implementations

template<VectorType T, CoordinateType C = CoordinateType::XY>
struct Vec2;

template<VectorType T>
struct Vec2<T,CoordinateType::XY> {
    T x;
    T y;

    Vec2() : x(0), y(0) {}

    template<VectorType X, VectorType Y>
    explicit Vec2(X const xVal, Y const yVal) : x(xVal), y(yVal) {
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
    friend H AbslHashValue(H hv, const Vec2& coord) {
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

template<VectorType T>
struct Vec2<T, CoordinateType::WH> {
    T w;
    T h;

    Vec2() : w(0), h(0) {}

    template<VectorType W, VectorType H>
    explicit Vec2(W const wVal, H const hVal) : w(wVal), h(hVal) {
        static_assert(std::is_same_v<T, std::remove_cv_t<std::remove_reference_t<W>>>, "Type of w must match this Vec2 type");
        static_assert(std::is_same_v<T, std::remove_cv_t<std::remove_reference_t<H>>>, "Type of h must match this Vec2 type");
    }

    template<VectorType U>
    Vec2 operator+(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return Vec2{w + other.w, h + other.h};
    }

    template<VectorType U>
    Vec2 operator-(Vec2<U> const& other) const {
        static_assert(std::is_same_v<T, U>, "Type of other Vec2 must match this Vec2 type");
        return Vec2{w - other.w, h - other.h};
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
    friend H AbslHashValue(H hv, const Vec2& coord) {
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
    T euclideanDistance(Vec2 const& other) const requires(std::is_floating_point_v<T>) {
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
    T manhattanDistance(Vec2 const& other) const requires(!std::is_floating_point_v<T>) {
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
    T chebyshevDistance(Vec2 const& other) const requires(!std::is_floating_point_v<T>) {
        T dx = std::abs(w - other.w);
        T dy = std::abs(h - other.h);
        return std::max(dx, dy);
    }
};

} // namespace Nebulite::Math
#endif // NEBULITE_MATH_VEC2_HPP
