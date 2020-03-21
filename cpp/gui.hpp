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
elem_draw(const graphics::core& gl, const std::tuple<Tp...>& t)
{
    if constexpr (has_draw_bg_method<typename std::tuple_element<I, std::tuple<Tp...>>::type>::value)
    {
        std::get<I>(t).draw_bg(gl);
    }
    std::get<I>(t).draw(gl);
    if constexpr (I + 1 < sizeof...(Tp))
    {
        elem_draw<I + 1, Tp...>(gl, t);
    }
}

TEMPLATE_CHECK_METHOD(set_pos);

template<std::size_t I = 0, typename... Tp>
inline std::enable_if_t<I < sizeof...(Tp), void>
elem_pos(point_t s, std::tuple<Tp...>& t)
{
    if constexpr (has_set_pos_method<typename std::tuple_element<I, std::tuple<Tp...>>::type>::value)
    {
        std::get<I>(t).set_pos(s);
    }
    if constexpr (I + 1 < sizeof...(Tp))
    {
        elem_pos<I + 1, Tp...>(s, t);
    }
}

TEMPLATE_CHECK_METHOD(trigger);

template<class T, std::size_t I = 0, typename... Tp>
inline std::enable_if_t<I < sizeof...(Tp), void>
elem_trigger(T& r, point_t p, std::tuple<Tp...>& t)
{
    if constexpr (has_trigger_method<typename std::tuple_element<sizeof...(Tp) - I - 1, std::tuple<Tp...>>::type>::value)
    {
        if (std::get<sizeof...(Tp) - I - 1>(t).check(p))
        {
            std::get<sizeof...(Tp) - I - 1>(t).trigger(r);
            return;
        }
    }
    if constexpr (I + 1 < sizeof...(Tp))
    {
        elem_trigger<T, I + 1, Tp...>(r, p, t);
    }
}

}  // namespace internal


namespace shapes
{

template<int X, int Y, int W, int H>
struct rectangle
{
    point_t pos;

    void set_pos(point_t size)
    {
        if constexpr (X >= 0)
            pos.x = X;
        else
            pos.x = size.x + X - W;

        if constexpr (Y >= 0)
            pos.y = Y;
        else
            pos.y = size.y + Y - H;
    }

    bool check(point_t p) const
    {
        return p.x >= pos.x && p.x <= (pos.x + W) && p.y >= pos.y && p.y <= (pos.y + H);
    }

    void draw_bg(const graphics::core& gl) const
    {
        gl.prog.fill.use();
        gl.prog.fill.set_transform(mat4x4_t::scale(W, H) * mat4x4_t::translate(pos + point_t{W, H} / 2.f));
        fill_rectangle(gl.prog.fill, {-.5f,-.5f,.5f,.5f});
    }
};

template<int X, int Y, int W, int H>
struct ellipse
{
    point_t pos;

    void set_pos(point_t size)
    {
        if constexpr (X >= 0)
            pos.x = X;
        else
            pos.x = size.x + X;

        if constexpr (Y >= 0)
            pos.y = Y;
        else
            pos.y = size.y + Y;
    }

    bool check(point_t p) const
    {
        return (p ^ pos) <= static_cast<float>(H);
    }

    void draw_bg(const graphics::core& gl) const
    {
        gl.prog.fill.use();
        gl.prog.fill.set_transform(mat4x4_t::scale(W, H) * mat4x4_t::translate(pos));
        fill_circle(gl.prog.fill, {0, 0}, .5f, 16);
    }
};

}  // namespace shapes

template<class...Types>
class interface
{
    using tuple_t = std::tuple<Types...>;
    tuple_t data;

public:
    void resize(point_t p)
    {
        internal::elem_pos(p, data);
    }

    void draw(const graphics::core& gl) const
    {
        internal::elem_draw(gl, data);
    }

    template<class T>
    void click(T& r, point_t p)
    {
        internal::elem_trigger<T, 0, Types...>(r, p, data);
    }
};

}  // namespace idle::gui

