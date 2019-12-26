/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of Violet.

    Violet is free software: you can study it, redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Violet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Violet.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <type_traits>
#include <array>
#include <cmath>
#include <algorithm>

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

namespace violet
{

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
constexpr inline T sqr(T a)
{
    return a * a;
}

template<typename T>
constexpr inline auto degtorad(T a)
{
    return a * T(M_PI) / T(180);
}

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
        return std::hypotf(first.x - second.x, first.y - second.y);
    }
};



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
        return std::hypot(first.x - second.x, first.y - second.y, first.z - second.z);
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

template<typename T>
struct grid {
    typedef T value_type;

    value_type ** m = nullptr;

    const size_t width;
    const size_t height;

private:
    void create_arrays() {
        m = new value_type*[width];
        *m = new value_type[width * height]; // data
        for (size_t i = 1; i < width; ++i)
            m[i] = *m + (i * height);
    }

public:

    grid(size_t w, size_t h) : width(w), height(h) {
        create_arrays();
    }

    grid(const grid &_cp) : width(_cp.width), height(_cp.height) {
        create_arrays();
        memcpy(*m, *_cp.m, width * height * sizeof(value_type));
    }

    grid(grid &&_mv) noexcept : width(_mv.width), height(_mv.height) {
        m = _mv.m;
        _mv.m = nullptr;
    }

    ~grid(void) {
        if (m) {
            delete[](*m); // data
            delete[]m;
        }
    }

    value_type * operator [](size_t i) const { return m[i]; }
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
        mat[5] = mat[0] = std::cos(r);
        mat[4] = -(mat[1] = std::sin(r));

        mat[12] = -p.x * mat[0] + -p.y * mat[4] + p.x;
        mat[13] = -p.x * mat[1] + -p.y * mat[5] + p.y;
        return mat;
    }

    constexpr static matrix4x4 rotate_x(value_type rad)
    {
        matrix4x4 mat{ identity() };
        mat[10] = mat[5] = std::cos(rad);
        mat[9] = -(mat[6] = std::sin(rad));
        return mat;
    }

    constexpr static matrix4x4 rotate_y(value_type rad)
    {
        matrix4x4 mat{ identity() };
        mat[10] = mat[0] = std::cos(rad);
        mat[2] = -(mat[8] = std::sin(rad));
        return mat;
    }

    constexpr static matrix4x4 rotate(value_type rad)
    {
        matrix4x4 mat{ identity() };
        mat[5] = mat[0] = std::cos(rad);
        mat[4] = -(mat[1] = std::sin(rad));
        return mat;
    }

    constexpr static matrix4x4 rotate(value_type rad, const point2<value_type> &p)
    {
        matrix4x4 mat{ identity() };
        mat[5] = mat[0] = std::cos(rad);
        mat[4] = -(mat[1] = std::sin(rad));

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

}

