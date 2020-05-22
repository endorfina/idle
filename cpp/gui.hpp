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
#include <array>
#include <tuple>
#include <utility>
#include <variant>
#include "gl.hpp"
#include "drawable.hpp"


namespace idle::gui
{

namespace internal
{

TEMPLATE_CHECK_METHOD(draw_bg);

template<std::size_t I = 0, typename... Tp>
inline std::enable_if_t<I < sizeof...(Tp), void>
elem_draw(const graphics::core& gl, const std::tuple<Tp...>& tuple)
{
    if constexpr (has_draw_bg_method<typename std::tuple_element<I, std::tuple<Tp...>>::type>::value)
    {
        std::get<I>(tuple).draw_bg(gl);
    }
    std::get<I>(tuple).draw(gl);
    if constexpr (I + 1 < sizeof...(Tp))
    {
        elem_draw<I + 1, Tp...>(gl, tuple);
    }
}

TEMPLATE_CHECK_METHOD(position_on);

template<std::size_t I = 0, typename... Tp>
inline std::enable_if_t<I < sizeof...(Tp), void>
elem_update_position(point_t screen_size, std::tuple<Tp...>& tuple)
{
    if constexpr (has_position_on_method<typename std::tuple_element<I, std::tuple<Tp...>>::type>::value)
    {
        std::get<I>(tuple).position_on(screen_size);
    }
    if constexpr (I + 1 < sizeof...(Tp))
    {
        elem_update_position<I + 1, Tp...>(screen_size, tuple);
    }
}

TEMPLATE_CHECK_METHOD(trigger);

template<class T, std::size_t I = 0, typename... Tp>
inline std::enable_if_t<I < sizeof...(Tp), void>
elem_trigger(T& meta, point_t pointer, std::tuple<Tp...>& tuple)
{
    if constexpr (has_trigger_method<typename std::tuple_element<sizeof...(Tp) - I - 1, std::tuple<Tp...>>::type>::value)
    {
        if (std::get<sizeof...(Tp) - I - 1>(tuple).is_touching(pointer))
        {
            std::get<sizeof...(Tp) - I - 1>(tuple).trigger(meta);
            return;
        }
    }
    if constexpr (I + 1 < sizeof...(Tp))
    {
        elem_trigger<T, I + 1, Tp...>(meta, pointer, tuple);
    }
}

}  // namespace internal

namespace positions
{

template<int X, int Y>
struct fixed
{
    constexpr static point_t pos { static_cast<float>(X), static_cast<float>(Y) };
};

template<int X, int Y>
struct edge_hugger
{
    point_t pos;

    constexpr void position_on(point_t screen_size)
    {
        if constexpr (X < 0)
            pos.x = screen_size.x + static_cast<float>(X);
        else
            pos.x = static_cast<float>(X);

        if constexpr (Y < 0)
            pos.y = screen_size.y + static_cast<float>(Y);
        else
            pos.y = static_cast<float>(Y);
    }
};

template<int X, int Y>
struct from_center
{
    point_t pos;

    constexpr void position_on(point_t screen_size)
    {
        pos = screen_size / 2.f + point_t{ static_cast<float>(X), static_cast<float>(Y) };
    }
};

}  // namespace positions

namespace shapes
{

template<class Pos, unsigned Width, unsigned Height>
struct rectangle
{
    bool is_touching(point_t pos) const
    {
        pos += point_t{ static_cast<float>(Width), static_cast<float>(Height) } / 2.f;
        return pos.x >= this->pos.x
            && pos.x <= (this->pos.x + Width)
            && pos.y >= this->pos.y
            && pos.y <= (this->pos.y + Height);
    }

    void draw_bg(const graphics::core& gl) const
    {
        gl.prog.fill.use();
        gl.prog.fill.set_transform(mat4x4_t::scale(Width, Height) * mat4x4_t::translate(this->pos));
        fill_rectangle(gl.prog.fill, {-.5f,-.5f,.5f,.5f});
    }
};

// template<int X, int Y, int W, int H>
// struct ellipse
// {
//     point_t pos;
//
//     void set_pos(point_t size)
//     {
//         if constexpr (X >= 0)
//             pos.x = X;
//         else
//             pos.x = size.x + X;
//
//         if constexpr (Y >= 0)
//             pos.y = Y;
//         else
//             pos.y = size.y + Y;
//     }
//
//     bool is_touching(point_t p) const
//     {
//         return (p ^ pos) <= static_cast<float>(H);
//     }
//
//     void draw_bg(const graphics::core& gl) const
//     {
//         gl.prog.fill.use();
//         gl.prog.fill.set_transform(mat4x4_t::scale(W, H) * mat4x4_t::translate(pos));
//         fill_circle(gl.prog.fill, {0, 0}, .5f, 16);
//     }
// };

template<class Pos, unsigned Width, unsigned Height>
struct button : Pos
{
    static_assert(Width > Height);

    bool is_touching(point_t pos) const
    {
        const auto diff = pos - this->pos;

        if (const auto xabs = math::ce::abs(diff.x);
                        xabs <= (Width - Height) / 2)
        {
            return math::ce::abs(diff.y) <= Height / 2;
        }
        else if (xabs < Width / 2)
        {
            const point_t new_pos{
                this->pos.x + (Width - Height) / (diff.x > 0 ? -2 : 2),
                this->pos.y };
            return (pos ^ new_pos) <= Height / 2.f;
        }
        return false;
    }

    void draw_bg(const graphics::core& gl) const
    {
        constexpr auto fan = []
        {
            constexpr unsigned max_size = 32;
            constexpr unsigned fan_size = (max_size - 2) / 2;
            constexpr float radius = Height / 2.f;
            constexpr float circle_center_offset = (Width - Height) / 2.f;

            std::array<point_t, max_size> fan{};

            for (unsigned i = 0; i < fan_size; ++i)
            {
                const float a = static_cast<float>(i) * F_TAU_2 / (fan_size - 1);

                //  the fan is rotationally symmetric
                fan[i + 2 + fan_size] =
                    (fan[i + 2] = {
                        circle_center_offset + math::ce::sin(a) * radius,
                        math::ce::cos(a) * radius })
                    * -1.f;
            }

            fan[1] = fan.back();
            return fan;
        }();

        gl.prog.fill.use();
        gl.prog.fill.set_transform(mat4x4_t::translate(this->pos));
        gl.prog.fill.position_vertex(reinterpret_cast<const GLfloat*>(fan.data()));
        gl::DrawArrays(gl::TRIANGLE_FAN, 0, fan.size());
    }
};

}  // namespace shapes

template<class...Types>
class interface
{
    using tuple_t = std::tuple<Types...>;
    tuple_t tuple_array;

public:
    void resize(point_t size)
    {
        internal::elem_update_position(size, tuple_array);
    }

    void draw(const graphics::core& gl) const
    {
        internal::elem_draw(gl, tuple_array);
    }

    template<class T>
    void click(T& meta, point_t pos)
    {
        internal::elem_trigger<T, 0, Types...>(meta, pos, tuple_array);
    }
};

}  // namespace idle::gui

