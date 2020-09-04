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
#include <optional>
#include <idle/gl.hpp>
#include <idle/drawable.hpp>


namespace idle::gui
{

namespace internal
{

template<std::size_t I = 0, typename Tuple, typename...Vars>
void elem_draw(const Tuple& tuple, const Vars&...vars) noexcept
{
    constexpr auto tuple_size = std::tuple_size<Tuple>::value;
    using tuple_elem_type = typename std::tuple_element<I, Tuple>::type;

    if constexpr (requires { &tuple_elem_type::draw_background; })
    {
        std::get<I>(tuple).draw_background(vars...);
    }

    if constexpr (requires { &tuple_elem_type::draw_foreground; })
    {
        std::get<I>(tuple).draw_foreground(vars...);
    }

    if constexpr (I + 1 < tuple_size)
    {
        elem_draw<I + 1, Tuple, Vars...>(tuple, vars...);
    }
}

template<std::size_t I = 0, typename... Tp>
void elem_update_position(point_t screen_size, std::tuple<Tp...>& tuple) noexcept
{
    using tuple_elem_type = typename std::tuple_element<I, std::tuple<Tp...>>::type;
    if constexpr (requires { &tuple_elem_type::position_on; })
    {
        std::get<I>(tuple).position_on(screen_size);
    }
    if constexpr (I + 1 < sizeof...(Tp))
    {
        elem_update_position<I + 1, Tp...>(screen_size, tuple);
    }
}

template<std::size_t I = 0, typename Ret, typename Tuple, typename Func, typename...Vars>
std::optional<Ret> elem_trigger(const point_t pointer, Tuple& tuple, Func&& func, Vars&&...vars) noexcept
{
    constexpr auto tuple_size = std::tuple_size<Tuple>::value;
    constexpr auto elem_id = tuple_size - I - 1;
    using tuple_elem_type = typename std::tuple_element<elem_id, Tuple>::type;

    if constexpr (requires { &tuple_elem_type::trigger; })
    {
        auto& elem = std::get<elem_id>(tuple);

        if (elem.is_touching(pointer))
        {
            func(elem);

            return { elem.trigger(std::forward<Vars>(vars)...) };
        }
    }

    if constexpr (I + 1 < tuple_size)
    {
        return elem_trigger<I + 1, Ret, Tuple>(pointer, tuple, std::forward<Func>(func), std::forward<Vars>(vars)...);
    }
    else
    {
        return {};
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

    constexpr void position_on(point_t screen_size) noexcept
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

    constexpr void position_on(point_t screen_size) noexcept
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
    bool is_touching(point_t pos) const noexcept
    {
        constexpr auto shift = point_t{ static_cast<float>(Width), static_cast<float>(Height) } / 2.f;

        pos += shift;

        return pos.x >= this->pos.x
            && pos.x <= (this->pos.x + Width)
            && pos.y >= this->pos.y
            && pos.y <= (this->pos.y + Height);
    }

    void draw_background(const graphics::core& gl) const noexcept
    {
        auto mat = math::matrices::scale(point_t{Width, Height});
        math::transform::translate(mat, this->pos);

        gl.prog.fill.use();
        gl.prog.fill.set_transform(mat);
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
//         gl.prog.fill.set_transform(math::matrices::scale(W, H) * math::matrices::translate(pos));
//         fill_circle(gl.prog.fill, {0, 0}, .5f, 16);
//     }
// };

template<class Pos, unsigned Width, unsigned Height>
struct buttonless : Pos
{
    static_assert(Width > Height);

    bool is_touching(point_t pt) const noexcept
    {
        const auto diff = pt - this->pos;

        if (const auto x_abs = math::ce::abs(diff.x);
                        x_abs <= (Width - Height) / 2)
        {
            return math::ce::abs(diff.y) <= Height / 2;
        }
        else if (x_abs <= Width / 2)
        {
            const auto x_shift = (Width - Height) / (diff.x > 0 ? -2.f : 2.f);
            const auto radius = std::hypotf(diff.x + x_shift, diff.y);
            return radius <= (Height / 2.f);
        }
        return false;
    }
};

template<class Pos, unsigned Width, unsigned Height>
struct button : buttonless<Pos, Width, Height>
{
    void draw_background(const graphics::core& gl) const noexcept
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
                const float a = static_cast<float>(i) * math::tau_2 / (fan_size - 1);

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
        gl.prog.fill.set_transform(math::matrices::translate(this->pos));
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
    void resize(point_t size) noexcept
    {
        internal::elem_update_position(size, tuple_array);
    }

    template<typename...Vars>
    void draw(const Vars&...vars) const noexcept
    {
        internal::elem_draw<0, tuple_t, Vars...>(tuple_array, vars...);
    }

    template<typename Ret, typename Func>
    std::optional<Ret> click(point_t pos, Func&& func) noexcept
    {
        return internal::elem_trigger<0, Ret, tuple_t>(pos, tuple_array, std::forward<Func>(func));
    }
};

}  // namespace idle::gui

