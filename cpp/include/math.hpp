/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of Idle.

    Idle is free software: you can study it, redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Idle is distributed in the hope that it will be fun and useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Idle. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <stdint.h>
#include <type_traits>
#include <array>
#include <algorithm>

#if __has_include(<numbers>)
#include <numbers>
#endif

#ifndef  __clang__  // gcc is love
#include <cmath>
#endif

#include "almost_cpp20.hpp"

namespace math
{
#ifdef __cpp_concepts
template<typename T>
concept floating = std::is_floating_point_v<T>;
#else
#define floating typename
#endif

#if __has_include(<numbers>)
template<floating T>
inline constexpr T pi = std::numbers::pi_v<T>;
#else
template<typename T>
inline constexpr T pi = T(3.14159265358979323846);
#endif

inline constexpr float tau_2 = pi<float>;
inline constexpr float tau = tau_2 * 2;
inline constexpr float tau_4 = tau_2 / 2;
inline constexpr float tau_8 = tau_2 / 4;

template<typename T>
constexpr T sqr(T x) noexcept
{
    return x * x;
}


namespace ce
{

template<typename T>
constexpr T abs(T x) noexcept
{
    return x >= 0 ? x : -x;
}


namespace detail
{
    template<floating T>
    constexpr bool epsilon_equal(T x, T y) noexcept
    {
        return abs(x - y) <= std::numeric_limits<T>::epsilon();
    }

    template<typename N, floating F>
    constexpr F power(F x, N nth) noexcept
    {
        if (nth == 0)
            return {1};
        if (nth == 1)
            return x;
        if (nth > 1)
        {
            if (nth & 1)
                return x * power<N, F>(x, nth - 1);
            return power<N, F>(x, nth / 2);
        }
        else
            return F{1} / power<N, F>(x, -nth);
    }

    template<floating T>
    constexpr T sqrt(const T x, T guess) noexcept
    {
        while (true)
        {
            const T new_guess = (guess + x / guess) / T{2};
            if (epsilon_equal(guess, new_guess))
                return guess;
            guess = new_guess;
        }
    }
}  // namespace detail

template<floating T>
constexpr T sqrt(T x) noexcept
{
    if (x == 0)
        return 0.f;

    return detail::sqrt(x, x);
}

namespace detail
{
    template<floating T>
    constexpr T cbrt(const T x, T guess) noexcept
    {
        while (true)
        {
            const T new_guess = (T{2} * guess + x / (guess * guess)) / T{3};
            if (epsilon_equal(guess, new_guess))
                return guess;
            guess = new_guess;
        }
    }
}  // namespace detail

template<floating T>
constexpr T cbrt(T x) noexcept
{
    return detail::cbrt(x, T{1});
}

template<floating T>
constexpr T hypot(T x, T y) noexcept
{
    return sqrt(x * x + y * y);
}

namespace detail
{
    template<floating T>
    constexpr T exp(const T x, T sum, T n, int i, T t) noexcept
    {
        while (true)
        {
            const T new_sum = sum + t / n;
            if (epsilon_equal(sum, new_sum))
                return sum;
            sum = new_sum;
            n *= i++;
            t *= x;
        }
    }
}  // namespace detail

template<floating T>
constexpr T exp(T x) noexcept
{
    return detail::exp(x, T{1}, T{1}, 2, x);
}

namespace detail
{
    template<floating T>
    constexpr T trigonometry(const T x, T sum, T n, int i, int s, T t) noexcept
    {
        while (true)
        {
            const T new_sum = sum + t * s / n;
            if (epsilon_equal(sum, new_sum))
                return sum;
            sum = new_sum;
            n *= i * (i + 1);
            i += 2;
            s = -s;
            t *= x * x;
        }
    }
}  // namespace detail

template<floating T>
constexpr T sin(T x) noexcept
{
    return detail::trigonometry(x, x, T{6}, 4, -1, x * x * x);
}

template<floating T>
constexpr T cos(T x) noexcept
{
    return detail::trigonometry(x, T{1}, T{2}, 3, -1, x * x);
}

template<floating T>
constexpr T tan(T x) noexcept
{
    if (const auto c = cos(x); c != 0)
        return sin(x) / c;
    return std::numeric_limits<T>::infinity();
}

namespace detail
{
    template<floating T>
    constexpr T atan_term(T x2, int k) noexcept
    {
        return (T{2} * static_cast<T>(k) * x2)
            / ((T{2} * static_cast<T>(k) + T{1}) * (T{1} + x2));
    }

    template<floating T>
    constexpr T atan_product(const T val, int lim) noexcept
    {
        auto ret = T{1};
        do
        {
            ret *= atan_term(val * val, lim);
        }
        while (--lim > 0);
        return ret;
    }

    template<floating T>
    constexpr T atan_sum(const T x, T sum, int n) noexcept
    {
        while (true)
        {
            const T new_sum = sum + atan_product(x, n);
            if (epsilon_equal(sum, new_sum))
                return sum;
            sum = new_sum;
            ++n;
        }
    }

    template<floating T>
    constexpr T inverse_trigonometry(const T x, T sum, int n, T t) noexcept
    {
        while (true)
        {
            const T new_sum = sum + t * n / (n + 2);
            if (epsilon_equal(sum, new_sum))
                return sum;
            sum = new_sum;
            n += 2;
            t *= x * x * n / (n + 3);
        }
    }

}  // namespace detail

template<floating T>
constexpr T asin(T x) noexcept
{
    if (x <= -1)
        return tau_2 / -2.f;
    if (x >= 1)
        return tau_4;

    return detail::inverse_trigonometry(x, x, 1, x * x * x / 2.f);
}

template<floating T>
constexpr T acos(T x) noexcept
{
    if (x <= -1)
        return tau_2;
    if (x >= 1)
        return 0.f;

    return tau_4 - asin(x);
}

template<floating T>
constexpr T atan(T x) noexcept
{
    return x / (T{1} + x * x) * detail::atan_sum(x, T{1}, 1);
}

template<floating T>
constexpr T atan2(T x, T y) noexcept
{
    if (x > 0)
        return atan(y/x);
    if (y >= 0 && x < 0)
        return atan(y/x) + static_cast<T>(tau_2);
    if (y < 0 && x < 0)
        return atan(y/x) - static_cast<T>(tau_2);
    if (y > 0 && x == 0)
        return static_cast<T>(tau_4);
    return -static_cast<T>(tau_4);
}

}  // namespace ce

template<typename T>
constexpr bool is_power_of_2(const T a) noexcept idle_weak_requires(std::is_integral_v<T>)
{
    for (T it = 1; it <= a && it > 0; it *= 2)
        if (it == a)
            return true;
    return false;
}

template<floating T>
constexpr T degtorad(T a) noexcept
{
    return a * pi<T> / T{180};
}


#ifdef __clang__
namespace const_math = ce;
#else
namespace const_math = ::std;
#endif


template<typename T>
struct point2
{
    static_assert(std::is_arithmetic_v<T>);

    using value_type = T;
    value_type x, y;
    constexpr point2() noexcept : x(0), y(0) {};
    constexpr point2(value_type a, value_type b) noexcept : x(a), y(b) {}

    constexpr point2& operator+=(const point2& other) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr point2& operator-=(const point2& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr point2& operator*=(const point2& other) noexcept
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    constexpr point2& operator/=(const point2& other) noexcept
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    constexpr friend point2 operator*(point2 first, const point2& second) noexcept { return first *= second; }

    constexpr friend point2 operator/(point2 first, const point2& second) noexcept { return first /= second; }

    constexpr friend point2 operator+(point2 first, const point2& second) noexcept { return first += second; }

    constexpr friend point2 operator-(point2 first, const point2& second) noexcept { return first -= second; }

    constexpr friend point2 operator*(point2 first, value_type second) noexcept { return first *= second; }

    constexpr friend point2 operator/(point2 first, value_type second) noexcept { return first /= second; }

    constexpr point2& operator*=(value_type f) noexcept
    {
        x *= f;
        y *= f;
        return *this;
    }

    constexpr point2& operator/=(value_type f) noexcept
    {
        x /= f;
        y /= f;
        return *this;
    }

    constexpr value_type product(const point2& other) const noexcept
    {
        return x * other.x + y * other.y;
    }

    constexpr value_type determinant(const point2& other) const noexcept
    {
        return x * other.y - y * other.x;
    }

    constexpr value_type distance(const point2& other) const noexcept
    {
        return const_math::hypot(x - other.x, y - other.y);
    }
};

template<class T, class A>
constexpr point2<T> point_cast(const point2<A> pt) noexcept
{
    return { static_cast<T>(pt.x), static_cast<T>(pt.y) };
}


template<typename T>
struct point3
{
    static_assert(std::is_arithmetic_v<T>);

    using value_type = T;
    value_type x, y, z;
    constexpr point3() noexcept : x(0), y(0), z(0) {};
    constexpr point3(value_type a, value_type b, value_type c) noexcept : x(a), y(b), z(c) {}

    constexpr point3& operator+=(const point3& other) noexcept
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr point3& operator-=(const point3& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr point3& operator*=(const point3& other) noexcept
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    constexpr point3& operator/=(const point3& other) noexcept
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    constexpr friend point3 operator*(point3 first, const point3 & second) noexcept { return first *= second; }

    constexpr friend point3 operator/(point3 first, const point3 & second) noexcept { return first /= second; }

    constexpr friend point3 operator+(point3 first, const point3 & second) noexcept { return first += second; }

    constexpr friend point3 operator-(point3 first, const point3 & second) noexcept { return first -= second; }

    constexpr friend point3 operator*(point3 first, value_type second) noexcept { return first *= second; }

    constexpr friend point3 operator/(point3 first, value_type second) noexcept { return first /= second; }

    constexpr point3& operator*=(value_type f) noexcept
    {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    constexpr point3& operator/=(value_type f) noexcept
    {
        x /= f;
        y /= f;
        z /= f;
        return *this;
    }

    constexpr value_type product(const point3& other) noexcept { return x * other.x + y * other.y + z * other.z; }

    // Distance
    friend float operator^(const point3& first, const point3& second)
    {
        return const_math::hypot(first.x - second.x, first.y - second.y, first.z - second.z);
    }
};


template<typename T>
constexpr bool is_inside(const point2<T> (&rect)[4], const point2<T> &p) noexcept
{
    const std::array<point2<T>, 2> lines {
            { rect[0].x - rect[1].x, rect[0].y - rect[1].y },
            { rect[0].x - rect[3].x, rect[0].y - rect[3].y }
    };

    for (auto axis : lines)
    {
        T s{}, min_a{}, max_a{};

        for (unsigned i = 0; i < 4; ++i)
        {
            s = (((rect[i] * axis) / (axis.x * axis.x + axis.y * axis.y))
                 * axis) % axis;
            if (i == 0)
                min_a = max_a = s;
            else if (s < min_a)
                min_a = s;
            else if (s > max_a)
                max_a = s;
        }

        s = (((p * axis) / (axis.x * axis.x + axis.y * axis.y))
             * axis) % axis;

        if (s > max_a || s < min_a)
            return false;
    }
    return true;
}

template<typename T>
constexpr bool collision(const point2<T> (&a)[4], const point2<T> (&b)[4]) noexcept
{
    std::array<point2<T>, 4> lines {
            { a[0].x - a[1].x, a[0].y - a[1].y },
            { a[0].x - a[3].x, a[0].y - a[3].y },
            { b[0].x - b[1].x, b[0].y - b[1].y },
            { b[0].x - b[3].x, b[0].y - b[3].y }
    };

    for (auto axis : lines)
    {
        unsigned i{};
        T s{}, min_a{}, min_b{}, max_a{}, max_b{};

        for (i = 0; i < 4; ++i)
        {
            s = (((a[i] * axis) / (axis.x * axis.x + axis.y * axis.y))
                 * axis) % axis;
            if (i == 0)
                min_a = max_a = s;
            else if (s < min_a)
                min_a = s;
            else if (s > max_a)
                max_a = s;
        }

        for (i = 0; i < 4; ++i)
        {
            s = (((b[i] * axis) / (axis.x * axis.x + axis.y * axis.y))
                 * axis) % axis;
            if (i == 0)
                min_b = max_b = s;
            else if (s < min_b)
                min_b = s;
            else if (s > max_b)
                max_b = s;
        }

        if (min_b > max_a || max_b < min_a)
            return false;
    }
    return true;
}




template<typename T>
struct color
{
    static_assert(std::is_scalar_v<T>);

    using value_type = T;
    value_type r, g, b, a;
    constexpr color() noexcept = default;
    constexpr color(value_type _r, value_type _g, value_type _b) noexcept : r(_r), g(_g), b(_b), a(1) {}
    constexpr color(value_type _r, value_type _g, value_type _b, value_type _a) noexcept : r(_r), g(_g), b(_b), a(_a) {}

    constexpr static color greyscale(value_type x, value_type a = 1) noexcept
    {
        return { x, x, x, a };
    }

    constexpr color& operator*=(const color &other) noexcept
    {
        r *= other.r;
        g *= other.g;
        b *= other.b;
        a *= other.a;
        return *this;
    }

    constexpr friend color operator*(color first, const color & second) noexcept { return first *= second; }
};

template<typename T>
struct rect {
    typedef T value_type;

    value_type left, top, right, bottom;

    constexpr rect() noexcept = default;
    constexpr rect(value_type _l, value_type _t, value_type _r, value_type _b) noexcept
        : left(_l), top(_t), right(_r), bottom(_b) {}
    constexpr rect(point2<value_type> _a, point2<value_type> _b) noexcept
        : left(_a.x), top(_a.y), right(_b.x), bottom(_b.y) {}
};

namespace meta
{

template<typename table_type>
constexpr table_type multiplication_2d(const table_type& first, const table_type& second) noexcept
{
    return {
        first[0] * second[0] + first[1] * second[4],
        first[0] * second[1] + first[1] * second[5],
        0,
        0,

        first[4] * second[0] + first[5] * second[4],
        first[4] * second[1] + first[5] * second[5],
        0,
        0,

        0,
        0,
        1,
        0,

        first[12] * second[0] + first[13] * second[4] + second[12],
        first[12] * second[1] + first[13] * second[5] + second[13],
        0,
        1
    };
}

template<typename table_type>
constexpr table_type multiplication(const table_type& first, const table_type& second) noexcept
{
    table_type dest{};
    for (int i = 0; i < 16; ++i)
    {
        const int x = i % 4;
        const int y = i - x;
        dest[i] = first[y] * second[x]
            + first[y + 1] * second[x + 4]
            + first[y + 2] * second[x + 8]
            + first[y + 3] * second[x + 12];
    }
    return dest;
}

template<typename table_type>
constexpr static bool invert_matrix(table_type &mat_) noexcept
{
    const table_type inv
    {
        mat_[5]  * mat_[10] * mat_[15] -
        mat_[5]  * mat_[11] * mat_[14] -
        mat_[9]  * mat_[6]  * mat_[15] +
        mat_[9]  * mat_[7]  * mat_[14] +
        mat_[13] * mat_[6]  * mat_[11] -
        mat_[13] * mat_[7]  * mat_[10],

        -mat_[1]  * mat_[10] * mat_[15] +
        mat_[1]  * mat_[11] * mat_[14] +
        mat_[9]  * mat_[2] * mat_[15] -
        mat_[9]  * mat_[3] * mat_[14] -
        mat_[13] * mat_[2] * mat_[11] +
        mat_[13] * mat_[3] * mat_[10],

        mat_[1]  * mat_[6] * mat_[15] -
        mat_[1]  * mat_[7] * mat_[14] -
        mat_[5]  * mat_[2] * mat_[15] +
        mat_[5]  * mat_[3] * mat_[14] +
        mat_[13] * mat_[2] * mat_[7] -
        mat_[13] * mat_[3] * mat_[6],

        -mat_[1] * mat_[6] * mat_[11] +
        mat_[1] * mat_[7] * mat_[10] +
        mat_[5] * mat_[2] * mat_[11] -
        mat_[5] * mat_[3] * mat_[10] -
        mat_[9] * mat_[2] * mat_[7] +
        mat_[9] * mat_[3] * mat_[6],

        -mat_[4]  * mat_[10] * mat_[15] +
        mat_[4]  * mat_[11] * mat_[14] +
        mat_[8]  * mat_[6]  * mat_[15] -
        mat_[8]  * mat_[7]  * mat_[14] -
        mat_[12] * mat_[6]  * mat_[11] +
        mat_[12] * mat_[7]  * mat_[10],

        mat_[0]  * mat_[10] * mat_[15] -
        mat_[0]  * mat_[11] * mat_[14] -
        mat_[8]  * mat_[2] * mat_[15] +
        mat_[8]  * mat_[3] * mat_[14] +
        mat_[12] * mat_[2] * mat_[11] -
        mat_[12] * mat_[3] * mat_[10],

        -mat_[0]  * mat_[6] * mat_[15] +
        mat_[0]  * mat_[7] * mat_[14] +
        mat_[4]  * mat_[2] * mat_[15] -
        mat_[4]  * mat_[3] * mat_[14] -
        mat_[12] * mat_[2] * mat_[7] +
        mat_[12] * mat_[3] * mat_[6],

        mat_[0] * mat_[6] * mat_[11] -
        mat_[0] * mat_[7] * mat_[10] -
        mat_[4] * mat_[2] * mat_[11] +
        mat_[4] * mat_[3] * mat_[10] +
        mat_[8] * mat_[2] * mat_[7] -
        mat_[8] * mat_[3] * mat_[6],

        mat_[4]  * mat_[9] * mat_[15] -
        mat_[4]  * mat_[11] * mat_[13] -
        mat_[8]  * mat_[5] * mat_[15] +
        mat_[8]  * mat_[7] * mat_[13] +
        mat_[12] * mat_[5] * mat_[11] -
        mat_[12] * mat_[7] * mat_[9],

        -mat_[0]  * mat_[9] * mat_[15] +
        mat_[0]  * mat_[11] * mat_[13] +
        mat_[8]  * mat_[1] * mat_[15] -
        mat_[8]  * mat_[3] * mat_[13] -
        mat_[12] * mat_[1] * mat_[11] +
        mat_[12] * mat_[3] * mat_[9],

        mat_[0]  * mat_[5] * mat_[15] -
        mat_[0]  * mat_[7] * mat_[13] -
        mat_[4]  * mat_[1] * mat_[15] +
        mat_[4]  * mat_[3] * mat_[13] +
        mat_[12] * mat_[1] * mat_[7] -
        mat_[12] * mat_[3] * mat_[5],

        -mat_[0] * mat_[5] * mat_[11] +
        mat_[0] * mat_[7] * mat_[9] +
        mat_[4] * mat_[1] * mat_[11] -
        mat_[4] * mat_[3] * mat_[9] -
        mat_[8] * mat_[1] * mat_[7] +
        mat_[8] * mat_[3] * mat_[5],

        -mat_[4]  * mat_[9] * mat_[14] +
        mat_[4]  * mat_[10] * mat_[13] +
        mat_[8]  * mat_[5] * mat_[14] -
        mat_[8]  * mat_[6] * mat_[13] -
        mat_[12] * mat_[5] * mat_[10] +
        mat_[12] * mat_[6] * mat_[9],

        mat_[0]  * mat_[9] * mat_[14] -
        mat_[0]  * mat_[10] * mat_[13] -
        mat_[8]  * mat_[1] * mat_[14] +
        mat_[8]  * mat_[2] * mat_[13] +
        mat_[12] * mat_[1] * mat_[10] -
        mat_[12] * mat_[2] * mat_[9],

        -mat_[0]  * mat_[5] * mat_[14] +
        mat_[0]  * mat_[6] * mat_[13] +
        mat_[4]  * mat_[1] * mat_[14] -
        mat_[4]  * mat_[2] * mat_[13] -
        mat_[12] * mat_[1] * mat_[6] +
        mat_[12] * mat_[2] * mat_[5],

        mat_[0] * mat_[5] * mat_[10] -
        mat_[0] * mat_[6] * mat_[9] -
        mat_[4] * mat_[1] * mat_[10] +
        mat_[4] * mat_[2] * mat_[9] +
        mat_[8] * mat_[1] * mat_[6] -
        mat_[8] * mat_[2] * mat_[5]
    };

    if (const auto det = mat_[0] * inv[0] + mat_[1] * inv[4] + mat_[2] * inv[8] + mat_[3] * inv[12])
    {
        std::transform(std::begin(inv), std::end(inv), std::begin(mat_), [det](auto a) { return a / det; });
        return true;
    }
    return false;
}

template<floating T>
struct matrix4x4_bare
{
    using value_type = T;
    using table_type = std::array<value_type, 16>;

    table_type values;

    constexpr matrix4x4_bare() noexcept : values{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1} {}

    constexpr matrix4x4_bare(const table_type& table) noexcept : values{table} {}

    constexpr explicit operator const value_type *() const noexcept { return values.data(); }

    constexpr decltype(auto) operator [](const unsigned i) const noexcept { return values[i]; }

    constexpr decltype(auto) operator [](const unsigned i) noexcept { return values[i]; }
};

}  // namespace meta

template<floating T, unsigned Level = 0>
struct matrix4x4 : meta::matrix4x4_bare<T>
{
    using bare_type = meta::matrix4x4_bare<T>;
    using bare_type::matrix4x4_bare;

    constexpr matrix4x4& invert(void) noexcept
    {
        meta::invert_matrix(this->values);
        return *this;
    }

    constexpr matrix4x4& reverse_multiply(const matrix4x4& other) noexcept
    {
        this->values = meta::multiplication(other.values, this->values);
        return *this;
    }
};

template<floating U, unsigned O1, unsigned O2>
constexpr matrix4x4<U, std::max(O1, O2)> operator*(const matrix4x4<U, O1>& lhs, const matrix4x4<U, O2>& rhs) noexcept
{
    if constexpr (O1 || O2)
        return meta::multiplication(lhs.values, rhs.values);
    else
        return meta::multiplication_2d(lhs.values, rhs.values);
}

template<floating T>
constexpr point2<T> operator*(const meta::matrix4x4_bare<T>& mat, const point2<T> p) noexcept
{
    return { mat[0] * p.x + mat[4] * p.y + mat[12],
             mat[1] * p.x + mat[5] * p.y + mat[13] };
}

template<floating T>
constexpr point3<T> operator*(const meta::matrix4x4_bare<T>& mat, const point3<T> p) noexcept
{
    return { mat[0] * p.x + mat[4] * p.y + mat[8] * p.z + mat[12],
             mat[1] * p.x + mat[5] * p.y + mat[9] * p.z + mat[13],
             mat[2] * p.x + mat[6] * p.y + mat[10] * p.z + mat[14] };
}

namespace matrices
{

template<floating T>
constexpr auto translate(const point2<T> &p) noexcept
{
    matrix4x4<T, 0> mat;
    mat[12] = p.x;
    mat[13] = p.y;
    return mat;
}

template<floating T>
constexpr matrix4x4<T, 1> translate(const point3<T> &p) noexcept
{
    matrix4x4<T, 1> mat;
    mat[12] = p.x;
    mat[13] = p.y;
    mat[14] = p.z;
    return mat;
}

template<floating T>
constexpr auto scale(const point2<T> sc) noexcept
{
    matrix4x4<T, 0> mat;
    mat[0] = sc.x;
    mat[5] = sc.y;
    return mat;
}

template<floating T>
constexpr matrix4x4<T, 1> scale(const point3<T> sc) noexcept
{
    matrix4x4<T, 1> mat;
    mat[0] = sc.x;
    mat[5] = sc.y;
    mat[10] = sc.z;
    return mat;
}

template<floating T>
constexpr auto scale(const point2<T> sc, const point2<T> p) noexcept
{
    matrix4x4<T, 0> mat;
    mat[0] = sc.x;
    mat[5] = sc.y;
    mat[12] = -p.x * sc.x + p.x;
    mat[13] = -p.y * sc.y + p.y;
    return mat;
}

template<floating T>
constexpr auto uniform_scale(const T sc) noexcept
{
    matrix4x4<T, 0> mat;
    mat[0] = sc;
    mat[5] = sc;
    return mat;
}

template<floating T>
constexpr auto uniform_scale(const T sc, const point2<T> p) noexcept
{
    matrix4x4<T, 0> mat;
    mat[0] = sc;
    mat[5] = sc;
    mat[12] = -p.x * sc + p.x;
    mat[13] = -p.y * sc + p.y;
    return mat;
}

template<floating T>
constexpr matrix4x4<T, 1> rotate_x(const T rad) noexcept
{
    matrix4x4<T, 1> mat;
    mat[10] = mat[5] = const_math::cos(rad);
    mat[9] = -(mat[6] = const_math::sin(rad));
    return mat;
}

template<floating T>
constexpr matrix4x4<T, 1> rotate_y(const T rad) noexcept
{
    matrix4x4<T, 1> mat;
    mat[10] = mat[0] = const_math::cos(rad);
    mat[2] = -(mat[8] = const_math::sin(rad));
    return mat;
}

template<floating T>
constexpr auto rotate(const T rad) noexcept
{
    matrix4x4<T, 0> mat;
    mat[5] = mat[0] = const_math::cos(rad);
    mat[4] = -(mat[1] = const_math::sin(rad));
    return mat;
}

template<floating T>
constexpr auto rotate(const T rad, const point2<T> &p) noexcept
{
    matrix4x4<T, 0> mat;
    mat[5] = mat[0] = const_math::cos(rad);
    mat[4] = -(mat[1] = const_math::sin(rad));

    mat[12] = -p.x * mat[0] + -p.y * mat[4] + p.x;
    mat[13] = -p.x * mat[1] + -p.y * mat[5] + p.y;
    return mat;
}

template<floating T>
constexpr matrix4x4<T, 1> orthof(const T left, const T right, const T top, const T bottom, const T near, const T far) noexcept
{
    matrix4x4<T, 1> mat;

    if (right != left && top != bottom && far != near)
    {
        mat[0] = 2 / (right - left);
        mat[5] = 2 / (top - bottom);
        mat[10] = -2 / (far - near);
        mat[15] = 1.f;
        mat[14] = -((far + near) / (far - near));
        mat[13] = -((top + bottom) / (top - bottom));
        mat[12] = -((right + left) / (right - left));
    }
    return mat;
}

template<int Near, int Far, floating T>
constexpr matrix4x4<T, 1> orthof_static(const T left, const T right, const T top, const T bottom) noexcept idle_weak_requires(Near != Far)
{
    matrix4x4<T, 1> mat;

    if (right != left && top != bottom)
    {
        mat[0] = 2 / (right - left);
        mat[5] = 2 / (top - bottom);
        mat[10] = -2 / static_cast<T>(Far - Near);
        mat[15] = 1.f;
        mat[14] = -(static_cast<T>(Far + Near) / static_cast<T>(Far - Near));
        mat[13] = -((top + bottom) / (top - bottom));
        mat[12] = -((right + left) / (right - left));
    }
    return mat;
}

}  // namespace matrices

namespace transform
{

template<floating T>
constexpr void translate(meta::matrix4x4_bare<T>& m, const point2<T> &p) noexcept
{
    const auto co = m;
    m[0] += co[3] * p.x;
    m[1] += co[3] * p.y;

    m[4] += co[7] * p.x;
    m[5] += co[7] * p.y;

    m[8] += co[11] * p.x;
    m[9] += co[11] * p.y;

    m[12] += co[15] * p.x;
    m[13] += co[15] * p.y;
}

template<floating T>
constexpr void translate(meta::matrix4x4_bare<T>& m, const point3<T> &p) noexcept
{
    const auto co = m;
    m[0] += co[3] * p.x;
    m[1] += co[3] * p.y;
    m[2] += co[3] * p.z;

    m[4] += co[7] * p.x;
    m[5] += co[7] * p.y;
    m[6] += co[7] * p.z;

    m[8] += co[11] * p.x;
    m[9] += co[11] * p.y;
    m[10] += co[11] * p.z;

    m[12] += co[15] * p.x;
    m[13] += co[15] * p.y;
    m[14] += co[15] * p.z;
}

template<floating T>
constexpr void scale(meta::matrix4x4_bare<T>& m, const point2<T> &sc) noexcept
{
    m[0] *= sc.x;
    m[1] *= sc.y;

    m[4] *= sc.x;
    m[5] *= sc.y;

    m[8] *= sc.x;
    m[9] *= sc.y;

    m[12] *= sc.x;
    m[13] *= sc.y;
}

template<floating T>
constexpr void scale(meta::matrix4x4_bare<T>& m, const point3<T> &sc) noexcept
{
    m[0] *= sc.x;
    m[1] *= sc.y;
    m[2] *= sc.z;

    m[4] *= sc.x;
    m[5] *= sc.y;
    m[6] *= sc.z;

    m[8] *= sc.x;
    m[9] *= sc.y;
    m[10] *= sc.z;

    m[12] *= sc.x;
    m[13] *= sc.y;
    m[14] *= sc.z;
}

template<floating T>
constexpr void uniform_scale(meta::matrix4x4_bare<T>& m, const T sc) noexcept
{
    m[0] *= sc;
    m[1] *= sc;

    m[4] *= sc;
    m[5] *= sc;

    m[8] *= sc;
    m[9] *= sc;

    m[12] *= sc;
    m[13] *= sc;
}

template<floating T>
constexpr void uniform_scale_3d(meta::matrix4x4_bare<T>& m, const T sc) noexcept
{
    m[0] *= sc;
    m[1] *= sc;
    m[2] *= sc;

    m[4] *= sc;
    m[5] *= sc;
    m[6] *= sc;

    m[8] *= sc;
    m[9] *= sc;
    m[10] *= sc;

    m[12] *= sc;
    m[13] *= sc;
    m[14] *= sc;
}

template<floating T>
constexpr void rotate_x(meta::matrix4x4_bare<T>& m, const T rad) noexcept
{
    const auto co = m;
    const auto c = const_math::cos(rad);
    const auto s = const_math::sin(rad);

    m[1] = co[1] * c + co[2] * -s;
    m[2] = co[1] * s + co[2] * c;

    m[5] = co[5] * c + co[6] * -s;
    m[6] = co[5] * s + co[6] * c;

    m[9] = co[9] * c + co[10] * -s;
    m[10] = co[9] * s + co[10] * c;

    m[13] = co[13] * c + co[14] * -s;
    m[14] = co[13] * s + co[14] * c;
}

template<floating T>
constexpr void rotate_y(meta::matrix4x4_bare<T>& m, const T rad) noexcept
{
    const auto co = m;
    const auto c = const_math::cos(rad);
    const auto s = const_math::sin(rad);

    m[0] = co[0] * c + co[2] * s;
    m[2] = co[0] * -s + co[2] * c;

    m[4] = co[4] * c + co[6] * s;
    m[6] = co[4] * -s + co[6] * c;

    m[8] = co[8] * c + co[10] * s;
    m[10] = co[8] * -s + co[10] * c;

    m[12] = co[12] * c + co[14] * s;
    m[14] = co[12] * -s + co[14] * c;
}

template<floating T>
constexpr void rotate_z(meta::matrix4x4_bare<T>& m, const T rad) noexcept
{
    const auto co = m;
    const auto c = const_math::cos(rad);
    const auto s = const_math::sin(rad);

    m[0] = co[0] * c + co[1] * -s;
    m[1] = co[0] * s + co[1] * c;

    m[4] = co[4] * c + co[5] * -s;
    m[5] = co[4] * s + co[5] * c;

    m[8] = co[8] * c + co[9] * -s;
    m[9] = co[8] * s + co[9] * c;

    m[12] = co[12] * c + co[13] * -s;
    m[13] = co[12] * s + co[13] * c;
}

}  // namespace transform

}  // namespace math

#ifdef floating
#undef floating
#endif

