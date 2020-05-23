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

#ifdef __GNUC__          // gcc is love
#include <cmath>
#endif

#include <memory.h> // memcpy

#define F_E         2.7182818284590452354f  /* e */
#define F_LOG2E     1.4426950408889634074f  /* log 2e */
#define F_LOG10E    0.43429448190325182765f /* log 10e */
#define F_LN2       0.69314718055994530942f /* log e2 */
#define F_LN10      2.30258509299404568402f /* log e10 */
#define F_PI        3.14159265358979323846f /* pi */
#define F_PI_2      1.57079632679489661923f /* pi/2 */
#define F_PI_4      0.78539816339744830962f /* pi/4 */
#define F_1_PI      0.31830988618379067154f /* 1/pi */
#define F_2_PI      0.63661977236758134308f /* 2/pi */
#define F_2_SQRTPI  1.12837916709551257390f /* 2/sqrt(pi) */
#define F_SQRT2     1.41421356237309504880f /* sqrt(2) */
#define F_SQRT1_2   0.70710678118654752440f /* 1/sqrt(2) */

#define F_TAU       6.28318530717958647692f /* tau */
#define F_TAU_2     F_PI    /* tau/2 */
#define F_TAU_4     F_PI_2  /* tau/4 */
#define F_TAU_8     F_PI_4  /* tau/4 */

#ifndef M_PI
#define M_PI F_PI
#endif

#ifndef M_PI_2
#define M_PI_2 F_PI_2
#endif

namespace math
{

template <typename T>
constexpr T sqr(T x)
{
    return x * x;
}


namespace ce
{

template <typename T>
constexpr T abs(T x)
{
    return x >= 0 ? x : -x;
}

namespace detail
{
    template <typename T>
    constexpr bool epsilon_equal(T x, T y)
    {
        return abs(x - y) <= std::numeric_limits<T>::epsilon();
    }

    template <typename N, typename F>
    constexpr F power(F x, N nth)
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

    template <typename T>
    constexpr T sqrt(const T x, T guess)
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

template <typename T>
constexpr auto sqrt(T x)
{
    if (x == 0)
        return 0.f;

    if constexpr (std::is_floating_point_v<T>)
        return detail::sqrt(x, x);
    else
        return detail::sqrt<float>(x, x);
}

namespace detail
{
    template <typename T>
    constexpr T cbrt(const T x, T guess)
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

template <typename T>
constexpr auto cbrt(T x)
{
    if constexpr (std::is_floating_point_v<T>)
        return detail::cbrt(x, T{1});
    else
        return detail::cbrt<float>(x, 1.f);
}

template <typename T>
constexpr auto hypot(T x, T y)
{
    return sqrt(x * x + y * y);
}

namespace detail
{
    template <typename T>
    constexpr T exp(const T x, T sum, T n, int i, T t)
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

template <typename T>
constexpr auto exp(T x)
{
    if constexpr (std::is_floating_point_v<T>)
        return detail::exp(x, T{1}, T{1}, 2, x);
    else
        return detail::exp<float>(x, 1.f, 1.f, 2, x);
}

namespace detail
{
    template <typename T>
    constexpr T trigonometry(const T x, T sum, T n, int i, int s, T t)
    {
        while (true)
        {
            const T new_sum = sum + t * s / n;
            if (epsilon_equal(sum, new_sum))
                return sum;
            sum = new_sum;
            n = n * i * (i + 1);
            i += 2;
            s = -s;
            t *= x * x;
        }
    }
}  // namespace detail

template <typename T>
constexpr auto sin(T x)
{
    if constexpr (std::is_floating_point_v<T>)
        return detail::trigonometry(x, x, T{6}, 4, -1, x * x * x);
    else
        return detail::trigonometry<float>(x, x, 6.f, 4, -1, x * x * x);
}

template <typename T>
constexpr auto cos(T x)
{
    if constexpr (std::is_floating_point_v<T>)
        return detail::trigonometry(x, T{1}, T{2}, 3, -1, x * x);
    else
        return detail::trigonometry<float>(x, 1.f, 2.f, 3, -1, x * x);
}

template <typename T>
constexpr auto tan(T x)
{
    static_assert(std::is_floating_point_v<T>);
    const auto c = cos(x);
    if (c == 0)
        return std::numeric_limits<T>::infinity();
    return sin(x) / c;
}

namespace detail
{
    template <typename T>
    constexpr T atan_term(T x2, int k)
    {
        return (T{2} * static_cast<T>(k) * x2)
            / ((T{2} * static_cast<T>(k) + T{1}) * (T{1} + x2));
    }

    template <typename T>
    constexpr T atan_product(T x, int k)
    {
        return k == 1 ? atan_term(x * x, k) :
            atan_term(x * x, k) * atan_product(x, k-1);
    }

    template <typename T>
    constexpr T atan_sum(T x, T sum, int n)
    {
        return sum + atan_product(x, n) == sum ? sum :
            atan_sum(x, sum + atan_product(x, n), n+1);
    }

}  // namespace detail

template <typename T>
constexpr auto atan(T x)
{
    if constexpr (std::is_floating_point_v<T>)
        return x / (T{1} + x * x) * detail::atan_sum(x, T{1}, 1);
    else
        return x / (1.f + x * x) * detail::atan_sum<float>(x, 1.f, 1);
}

template <typename T>
constexpr T atan2(T x, T y)
{
    return x > 0 ? atan(y/x) :
        y >= 0 && x < 0 ? atan(y/x) + static_cast<T>(M_PI) :
        y < 0 && x < 0 ? atan(y/x) - static_cast<T>(M_PI) :
        y > 0 && x == 0 ? static_cast<T>(M_PI_2) :
        -static_cast<T>(M_PI_2);
}

namespace detail
{
    template <typename T>
    constexpr T floor2(T x, T guess, T inc)
    {
        return guess + inc <= x ? floor2(x, guess + inc, inc) :
            inc <= T{1} ? guess : floor2(x, guess, inc/T{2});
    }

    template <typename T>
    constexpr T floor(T x, T guess, T inc)
    {
        return
            inc < T{8} ? floor2(x, guess, inc) :
            guess + inc <= x ? floor(x, guess + inc, inc) :
            guess + (inc/T{8})*T{7} <= x ? floor(x, guess + (inc/T{8})*T{7}, inc/T{8}) :
            guess + (inc/T{8})*T{6} <= x ? floor(x, guess + (inc/T{8})*T{6}, inc/T{8}) :
            guess + (inc/T{8})*T{5} <= x ? floor(x, guess + (inc/T{8})*T{5}, inc/T{8}) :
            guess + (inc/T{8})*T{4} <= x ? floor(x, guess + (inc/T{8})*T{4}, inc/T{8}) :
            guess + (inc/T{8})*T{3} <= x ? floor(x, guess + (inc/T{8})*T{3}, inc/T{8}) :
            guess + (inc/T{8})*T{2} <= x ? floor(x, guess + (inc/T{8})*T{2}, inc/T{8}) :
            guess + inc/T{8} <= x ? floor(x, guess + inc/T{8}, inc/T{8}) :
            floor(x, guess, inc/T{8});
    }

    template <typename T>
    constexpr T ceil2(T x, T guess, T dec)
    {
        return guess - dec >= x ? ceil2(x, guess - dec, dec) :
            dec <= T{1} ? guess : ceil2(x, guess, dec/T{2});
    }

    template <typename T>
    constexpr T ceil(T x, T guess, T dec)
    {
        return
            dec < T{8} ? ceil2(x, guess, dec) :
            guess - dec >= x ? ceil(x, guess - dec, dec) :
            guess - (dec/T{8})*T{7} >= x ? ceil(x, guess - (dec/T{8})*T{7}, dec/T{8}) :
            guess - (dec/T{8})*T{6} >= x ? ceil(x, guess - (dec/T{8})*T{6}, dec/T{8}) :
            guess - (dec/T{8})*T{5} >= x ? ceil(x, guess - (dec/T{8})*T{5}, dec/T{8}) :
            guess - (dec/T{8})*T{4} >= x ? ceil(x, guess - (dec/T{8})*T{4}, dec/T{8}) :
            guess - (dec/T{8})*T{3} >= x ? ceil(x, guess - (dec/T{8})*T{3}, dec/T{8}) :
            guess - (dec/T{8})*T{2} >= x ? ceil(x, guess - (dec/T{8})*T{2}, dec/T{8}) :
            guess - dec/T{8} >= x ? ceil(x, guess - dec/T{8}, dec/T{8}) :
            ceil(x, guess, dec/T{8});
    }
}  // namespace detail

constexpr float ceil(float x);
constexpr double ceil(double x);

template <typename T>
constexpr float ceil(T x);

constexpr float floor(float x)
{
    return x < 0 ? -ceil(-x) :
        detail::floor(
            x, 0.0f,
            detail::power<int>(2.0f, std::numeric_limits<float>::max_exponent-1)
        );
}
constexpr double floor(double x)
{
    return x < 0 ? -ceil(-x) :
        detail::floor(
            x, 0.0,
            detail::power<int>(2.0, std::numeric_limits<double>::max_exponent-1)
        );
}

template<typename T>
constexpr std::enable_if_t<std::is_integral_v<T>, double> floor(T x)
{
    return static_cast<double>(x);
}

constexpr float ceil(float x)
{
    return x < 0 ? -floor(-x) :
        detail::ceil(
            x, detail::power<int>(2.f, std::numeric_limits<float>::max_exponent - 1),
            detail::power<int>(2.f, std::numeric_limits<float>::max_exponent - 1)
        );
}

constexpr double ceil(double x)
{
    return x < 0 ? -floor(-x) :
        detail::ceil(
            x, detail::power<int>(2., std::numeric_limits<double>::max_exponent - 1),
            detail::power<int>(2., std::numeric_limits<double>::max_exponent - 1)
        );
}

template <typename T>
constexpr std::enable_if_t<std::is_integral_v<T>, float> ceil(T x)
{
    return static_cast<float>(x);
}

constexpr float trunc(float x)
{
    return x >= 0 ? floor(x) : -floor(-x);
}

constexpr double trunc(double x)
{
    return x >= 0 ? floor(x) : -floor(-x);
}

constexpr float round(float x)
{
    return x >= 0 ? floor(x + 0.5f) :
        ceil(x - 0.5f);
}
constexpr double round(double x)
{
    return x >= 0 ? floor(x + 0.5) :
        ceil(x - 0.5);
}

constexpr float fmod(float x, float y)
{
    return x - trunc(x / y) * y;
}

constexpr double fmod(double x, double y)
{
    return x - trunc(x / y) * y;
}

constexpr float remainder(float x, float y)
{
    return x - y * round(x / y);
}

constexpr double remainder(double x, double y)
{
    return x - y * round(x / y);
}

}  // namespace ce

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr bool is_power_of_2(const T a)
{
    using int_t = std::conditional_t<sizeof(T) >= sizeof(int_fast32_t), T, std::conditional_t<std::is_signed_v<T>, int_fast32_t, uint_fast32_t>>;
    for (int_t it = 1, max = static_cast<int_t>(a); it <= max && it > 0; it *= 2)
        if (it == max)
            return true;
    return false;
}

template<typename T>
constexpr std::enable_if_t<std::is_floating_point_v<T>, float> degtorad(T a)
{
    return a * T{M_PI} / T{180};
}


#ifdef __GNUC__
namespace const_math = ::std;
#else
namespace const_math = ce;
#endif


template<typename T>
struct point2
{
    static_assert(std::is_arithmetic_v<T>);

    using value_type = T;
    value_type x, y;
    constexpr point2() = default;
    constexpr point2(value_type a, value_type b) : x(a), y(b) {}

    constexpr point2& operator+=(const point2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr point2& operator-=(const point2& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr point2& operator*=(const point2& other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    constexpr point2& operator/=(const point2& other)
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    constexpr friend point2 operator*(point2 first, const point2& second) { return first *= second; }

    constexpr friend point2 operator/(point2 first, const point2& second) { return first /= second; }

    constexpr friend point2 operator+(point2 first, const point2& second) { return first += second; }

    constexpr friend point2 operator-(point2 first, const point2& second) { return first -= second; }

    constexpr friend point2 operator*(point2 first, value_type second) { return first *= second; }

    constexpr friend point2 operator/(point2 first, value_type second) { return first /= second; }

    constexpr point2& operator*=(value_type f)
    {
        x *= f;
        y *= f;
        return *this;
    }

    constexpr point2& operator/=(value_type f)
    {
        x /= f;
        y /= f;
        return *this;
    }

    constexpr value_type product(const point2& other) { return x * other.x + y * other.y; }

    // Distance
    friend float operator^(const point2& first, const point2& second)
    {
        return const_math::hypot(first.x - second.x, first.y - second.y);
    }
};

template<class T, class A>
constexpr point2<T> point_cast(const point2<A> pt)
{
    return { static_cast<T>(pt.x), static_cast<T>(pt.y) };
}


template<typename T>
struct point3
{
    static_assert(std::is_arithmetic_v<T>);

    using value_type = T;
    value_type x, y, z;
    constexpr point3() = default;
    constexpr point3(value_type a, value_type b, value_type c) : x(a), y(b), z(c) {}

    constexpr point3& operator+=(const point3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr point3& operator-=(const point3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr point3& operator*=(const point3& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    constexpr point3& operator/=(const point3& other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    constexpr friend point3 operator*(point3 first, const point3 & second) { return first *= second; }

    constexpr friend point3 operator/(point3 first, const point3 & second) { return first /= second; }

    constexpr friend point3 operator+(point3 first, const point3 & second) { return first += second; }

    constexpr friend point3 operator-(point3 first, const point3 & second) { return first -= second; }

    constexpr friend point3 operator*(point3 first, value_type second) { return first *= second; }

    constexpr friend point3 operator/(point3 first, value_type second) { return first /= second; }

    constexpr point3& operator*=(value_type f)
    {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    constexpr point3& operator/=(value_type f)
    {
        x /= f;
        y /= f;
        z /= f;
        return *this;
    }

    constexpr value_type product(const point3& other) { return x * other.x + y * other.y + z * other.z; }

    // Distance
    friend float operator^(const point3& first, const point3& second)
    {
        return const_math::hypot(first.x - second.x, first.y - second.y, first.z - second.z);
    }
};


template<typename T>
constexpr bool is_inside(const point2<T> (&rect)[4], const point2<T> &p)
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
constexpr bool collision(const point2<T> (&a)[4], const point2<T> (&b)[4])
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
    constexpr color() = default;
    constexpr color(value_type _r, value_type _g, value_type _b) : r(_r), g(_g), b(_b), a(1) {}
    constexpr color(value_type _r, value_type _g, value_type _b, value_type _a) : r(_r), g(_g), b(_b), a(_a) {}

    constexpr static color greyscale(value_type x, value_type a = 1)
    {
        return { x, x, x, a };
    }

    constexpr color& operator*=(const color &other)
    {
        r *= other.r;
        g *= other.g;
        b *= other.b;
        a *= other.a;
        return *this;
    }

    constexpr friend color operator*(color first, const color & second) { return first *= second; }
};

template<typename T>
struct rect {
    typedef T value_type;

    value_type left, top, right, bottom;

    constexpr rect() = default;
    constexpr rect(value_type _l, value_type _t, value_type _r, value_type _b)
        : left(_l), top(_t), right(_r), bottom(_b) {}
    constexpr rect(point2<value_type> _a, point2<value_type> _b)
        : left(_a.x), top(_a.y), right(_b.x), bottom(_b.y) {}
};


template<typename T, bool Secure = true>
struct matrix4x4
{
    static_assert(std::is_arithmetic_v<T>);

protected:
    using value_type = T;
    using table_type = std::array<value_type, 16>;
    table_type data;

public:
    constexpr matrix4x4() = default;

    constexpr matrix4x4(const table_type& t) : data(t) {}

    constexpr explicit operator const value_type *() const { return data.data(); }

    constexpr decltype(auto) operator [](unsigned i) const { return data[i]; }

    constexpr decltype(auto) operator [](unsigned i) { return data[i]; }

protected:
    constexpr static table_type _multiplication_special_(const table_type& first, const table_type& second)
    {
        table_type dest{first};
        dest.fill(0);
        dest[0] = first[0] * second[0] + first[1] * second[4];
        dest[1] = first[0] * second[1] + first[1] * second[5];
        dest[4] = first[4] * second[0] + first[5] * second[4];
        dest[5] = first[4] * second[1] + first[5] * second[5];
        dest[10] = 1;
        dest[12] = first[12] * second[0] + first[13] * second[4] + second[12];
        dest[13] = first[12] * second[1] + first[13] * second[5] + second[13];
        dest[15] = 1;
        return dest;
    }

private:
    constexpr static table_type _multiplication_(const table_type& first, const table_type& second)
    {
        table_type dest{first};
        for (int i = 0; i < 16; ++i) {
            dest[i] = first[i - (i % 4)] * second[i % 4];
            for (int n = 1; n < 4; ++n)
                dest[i] += first[i - (i % 4) + n] * second[(i % 4) + n * 4];
        }
        return dest;
    }

    constexpr static bool _invert_matrix_(table_type &_mat)
    {
        const table_type inv
        {
            _mat[5]  * _mat[10] * _mat[15] -
            _mat[5]  * _mat[11] * _mat[14] -
            _mat[9]  * _mat[6]  * _mat[15] +
            _mat[9]  * _mat[7]  * _mat[14] +
            _mat[13] * _mat[6]  * _mat[11] -
            _mat[13] * _mat[7]  * _mat[10],

            -_mat[1]  * _mat[10] * _mat[15] +
            _mat[1]  * _mat[11] * _mat[14] +
            _mat[9]  * _mat[2] * _mat[15] -
            _mat[9]  * _mat[3] * _mat[14] -
            _mat[13] * _mat[2] * _mat[11] +
            _mat[13] * _mat[3] * _mat[10],

            _mat[1]  * _mat[6] * _mat[15] -
            _mat[1]  * _mat[7] * _mat[14] -
            _mat[5]  * _mat[2] * _mat[15] +
            _mat[5]  * _mat[3] * _mat[14] +
            _mat[13] * _mat[2] * _mat[7] -
            _mat[13] * _mat[3] * _mat[6],

            -_mat[1] * _mat[6] * _mat[11] +
            _mat[1] * _mat[7] * _mat[10] +
            _mat[5] * _mat[2] * _mat[11] -
            _mat[5] * _mat[3] * _mat[10] -
            _mat[9] * _mat[2] * _mat[7] +
            _mat[9] * _mat[3] * _mat[6],

            -_mat[4]  * _mat[10] * _mat[15] +
            _mat[4]  * _mat[11] * _mat[14] +
            _mat[8]  * _mat[6]  * _mat[15] -
            _mat[8]  * _mat[7]  * _mat[14] -
            _mat[12] * _mat[6]  * _mat[11] +
            _mat[12] * _mat[7]  * _mat[10],

            _mat[0]  * _mat[10] * _mat[15] -
            _mat[0]  * _mat[11] * _mat[14] -
            _mat[8]  * _mat[2] * _mat[15] +
            _mat[8]  * _mat[3] * _mat[14] +
            _mat[12] * _mat[2] * _mat[11] -
            _mat[12] * _mat[3] * _mat[10],

            -_mat[0]  * _mat[6] * _mat[15] +
            _mat[0]  * _mat[7] * _mat[14] +
            _mat[4]  * _mat[2] * _mat[15] -
            _mat[4]  * _mat[3] * _mat[14] -
            _mat[12] * _mat[2] * _mat[7] +
            _mat[12] * _mat[3] * _mat[6],

            _mat[0] * _mat[6] * _mat[11] -
            _mat[0] * _mat[7] * _mat[10] -
            _mat[4] * _mat[2] * _mat[11] +
            _mat[4] * _mat[3] * _mat[10] +
            _mat[8] * _mat[2] * _mat[7] -
            _mat[8] * _mat[3] * _mat[6],

            _mat[4]  * _mat[9] * _mat[15] -
            _mat[4]  * _mat[11] * _mat[13] -
            _mat[8]  * _mat[5] * _mat[15] +
            _mat[8]  * _mat[7] * _mat[13] +
            _mat[12] * _mat[5] * _mat[11] -
            _mat[12] * _mat[7] * _mat[9],

            -_mat[0]  * _mat[9] * _mat[15] +
            _mat[0]  * _mat[11] * _mat[13] +
            _mat[8]  * _mat[1] * _mat[15] -
            _mat[8]  * _mat[3] * _mat[13] -
            _mat[12] * _mat[1] * _mat[11] +
            _mat[12] * _mat[3] * _mat[9],

            _mat[0]  * _mat[5] * _mat[15] -
            _mat[0]  * _mat[7] * _mat[13] -
            _mat[4]  * _mat[1] * _mat[15] +
            _mat[4]  * _mat[3] * _mat[13] +
            _mat[12] * _mat[1] * _mat[7] -
            _mat[12] * _mat[3] * _mat[5],

            -_mat[0] * _mat[5] * _mat[11] +
            _mat[0] * _mat[7] * _mat[9] +
            _mat[4] * _mat[1] * _mat[11] -
            _mat[4] * _mat[3] * _mat[9] -
            _mat[8] * _mat[1] * _mat[7] +
            _mat[8] * _mat[3] * _mat[5],

            -_mat[4]  * _mat[9] * _mat[14] +
            _mat[4]  * _mat[10] * _mat[13] +
            _mat[8]  * _mat[5] * _mat[14] -
            _mat[8]  * _mat[6] * _mat[13] -
            _mat[12] * _mat[5] * _mat[10] +
            _mat[12] * _mat[6] * _mat[9],

            _mat[0]  * _mat[9] * _mat[14] -
            _mat[0]  * _mat[10] * _mat[13] -
            _mat[8]  * _mat[1] * _mat[14] +
            _mat[8]  * _mat[2] * _mat[13] +
            _mat[12] * _mat[1] * _mat[10] -
            _mat[12] * _mat[2] * _mat[9],

            -_mat[0]  * _mat[5] * _mat[14] +
            _mat[0]  * _mat[6] * _mat[13] +
            _mat[4]  * _mat[1] * _mat[14] -
            _mat[4]  * _mat[2] * _mat[13] -
            _mat[12] * _mat[1] * _mat[6] +
            _mat[12] * _mat[2] * _mat[5],

            _mat[0] * _mat[5] * _mat[10] -
            _mat[0] * _mat[6] * _mat[9] -
            _mat[4] * _mat[1] * _mat[10] +
            _mat[4] * _mat[2] * _mat[9] +
            _mat[8] * _mat[1] * _mat[6] -
            _mat[8] * _mat[2] * _mat[5]
        };

        if (const auto det = _mat[0] * inv[0] + _mat[1] * inv[4] + _mat[2] * inv[8] + _mat[3] * inv[12])
        {
            std::transform(std::begin(inv), std::end(inv), std::begin(_mat), [det](auto a) { return a / det; });
            return true;
        }
        return false;
    }

public:
    template <typename U, bool O>
    constexpr matrix4x4& operator*=(const matrix4x4<U, O>& other)
    {
        if constexpr (O || Secure)
            data = _multiplication_(data, other.data);
        else
            data = _multiplication_special_(data, other.data);
        return *this;
    }

    template <typename U, bool O>
    friend constexpr matrix4x4 operator*(matrix4x4 lhs, const matrix4x4<U, O>& rhs)
    {
        return lhs *= rhs;
    }



    constexpr matrix4x4& invert(void)
    {
        _invert_matrix_(data);
        return *this;
    }

    constexpr static matrix4x4 identity()
    {
        return table_type{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    }

    constexpr static matrix4x4 translate(value_type x, value_type y)
    {
        matrix4x4 mat{ identity() };
        mat[12] = x;
        mat[13] = y;
        return mat;
    }

    constexpr static matrix4x4 translate(const point2<value_type> &p)
    {
        matrix4x4 mat{ identity() };
        mat[12] = p.x;
        mat[13] = p.y;
        return mat;
    }

    constexpr static matrix4x4 translate(const point3<value_type> &p)
    {
        matrix4x4 mat{ identity() };
        mat[12] = p.x;
        mat[13] = p.y;
        mat[14] = p.z;
        return mat;
    }

    constexpr static matrix4x4 scale(value_type w, value_type h)
    {
        matrix4x4 mat{ identity() };
        mat[0] = w;
        mat[5] = h;
        return mat;
    }

    constexpr static matrix4x4 scale(value_type w, value_type h, value_type d)
    {
        matrix4x4 mat{ identity() };
        mat[0] = w;
        mat[5] = h;
        mat[10] = d;
        return mat;
    }

    constexpr static matrix4x4 scale(value_type w, value_type h, const point2<value_type> &p)
    {
        matrix4x4 mat{ identity() };
        mat[0] = w;
        mat[5] = h;
        mat[12] = -p.x * w + p.x;
        mat[13] = -p.y * h + p.y;
        return mat;
    }

    constexpr static matrix4x4 scale(const point2<value_type> p)
    {
        matrix4x4 mat{ identity() };
        mat[0] = p.x;
        mat[5] = p.y;
        return mat;
    }

    constexpr static matrix4x4 scale(value_type sc, const point2<value_type> &p)
    {
        matrix4x4 mat{ identity() };
        mat[0] = sc;
        mat[5] = sc;
        mat[12] = -p.x * sc + p.x;
        mat[13] = -p.y * sc + p.y;
        return mat;
    }

    constexpr static matrix4x4 scale(value_type sc)
    {
        matrix4x4 mat{ identity() };
        mat[0] = sc;
        mat[5] = sc;
        return mat;
    }

    constexpr static matrix4x4 rotate_deg(value_type angle, const point2<value_type> &p)
    {
        const auto r = degtorad(angle);
        matrix4x4 mat{ identity() };
        mat[5] = mat[0] = const_math::cos(r);
        mat[4] = -(mat[1] = const_math::sin(r));

        mat[12] = -p.x * mat[0] + -p.y * mat[4] + p.x;
        mat[13] = -p.x * mat[1] + -p.y * mat[5] + p.y;
        return mat;
    }

    constexpr static matrix4x4 rotate_x(value_type rad)
    {
        matrix4x4 mat{ identity() };
        mat[10] = mat[5] = const_math::cos(rad);
        mat[9] = -(mat[6] = const_math::sin(rad));
        return mat;
    }

    constexpr static matrix4x4 rotate_y(value_type rad)
    {
        matrix4x4 mat{ identity() };
        mat[10] = mat[0] = const_math::cos(rad);
        mat[2] = -(mat[8] = const_math::sin(rad));
        return mat;
    }

    constexpr static matrix4x4 rotate(value_type rad)
    {
        matrix4x4 mat{ identity() };
        mat[5] = mat[0] = const_math::cos(rad);
        mat[4] = -(mat[1] = const_math::sin(rad));
        return mat;
    }

    constexpr static matrix4x4 rotate(value_type rad, const point2<value_type> &p)
    {
        matrix4x4 mat{ identity() };
        mat[5] = mat[0] = const_math::cos(rad);
        mat[4] = -(mat[1] = const_math::sin(rad));

        mat[12] = -p.x * mat[0] + -p.y * mat[4] + p.x;
        mat[13] = -p.x * mat[1] + -p.y * mat[5] + p.y;
        return mat;
    }

    constexpr static matrix4x4 orthof(value_type left, value_type right, value_type top, value_type bottom, value_type near, value_type far)
    {
        matrix4x4 mat{ identity() };
        if (right != left && top != bottom && far != near) {
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

    template<int Near, int Far>
    constexpr static matrix4x4 orthof_static(value_type left, value_type right, value_type top, value_type bottom)
    {
        matrix4x4 mat{ identity() };
        static_assert(Far != Near);
        if (right != left && top != bottom) {
            mat[0] = 2 / (right - left);
            mat[5] = 2 / (top - bottom);
            mat[10] = -2 / static_cast<value_type>(Far - Near);
            mat[15] = 1.f;
            mat[14] = -(static_cast<value_type>(Far + Near) / static_cast<value_type>(Far - Near));
            mat[13] = -((top + bottom) / (top - bottom));
            mat[12] = -((right + left) / (right - left));
        }
        return mat;
    }

    constexpr matrix4x4& reverse_multiply(const matrix4x4& other)
    {
        this->data = _multiplication_(other.data, this->data);
        return *this;
    }
};

template<typename T, bool O>
constexpr point2<T> operator*(const matrix4x4<T, O>& mat, const point2<T> p)
{
    return { mat[0] * p.x + mat[4] * p.y + mat[12],
             mat[1] * p.x + mat[5] * p.y + mat[13] };
}

template<typename T, bool O>
constexpr point3<T> operator*(const matrix4x4<T, O>& mat, const point3<T> p)
{
    return { mat[0] * p.x + mat[4] * p.y + mat[8] * p.z + mat[12],
             mat[1] * p.x + mat[5] * p.y + mat[9] * p.z + mat[13],
             mat[2] * p.x + mat[6] * p.y + mat[10] * p.z + mat[14] };
}

template<typename Uint>
struct field
{
    Uint bits;

    constexpr void set(const Uint val)
    {
        bits |= val;
    }

    constexpr void reset(const Uint val)
    {
        bits &= ~val;
    }

    constexpr bool test(const Uint val) const
    {
        return !! (bits & val);
    }

    constexpr void flip()
    {
        bits = ~bits;
    }
};

}  // namespace math

