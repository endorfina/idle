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
#include <string_view>
#include <math.hpp>
#include "gl.hpp"

namespace idle::glass
{

template <typename Head, typename... Tail>
constexpr inline unsigned elem_size()
{
    if constexpr (!!sizeof...(Tail))
    {
        return Head::size +
            elem_size<Tail...>();
    }
    else
    {
        return Head::size;
    }
}

template <typename... Appendages>
struct bone
{
    float length;
    point_3d_t angle, lower_bound, upper_bound;
    float stiffness;
    std::tuple<Appendages...> appendages;

    static constexpr unsigned size = elem_size<Appendages...>() + 1;
};

template<>
struct bone<>
{
    float length;
    point_3d_t angle, lower_bound, upper_bound;
    float stiffness;

    static constexpr unsigned size = 1;
};

template <unsigned FrameSize, unsigned AnimLength>
using atable = std::array<std::array<float, FrameSize * 2>, AnimLength>;

template<typename Value>
struct symmetry
{
    Value left, right;
};

namespace spine
{

struct humanoid
{
    using limb_bone = bone<bone<bone<>>>;
    symmetry<limb_bone> arms, legs;
    bone<> head;
    point_3d_t center, rotation;
};

}  // namespace spine

template <typename T>
struct flesh
{

};

template<unsigned FrameSize, unsigned AnimLength>
inline void draw(const graphics::double_vertex_program_t& program, const atable<FrameSize, AnimLength>& table)
{
    program.position_vertex(table[0].data());
    program.destination_vertex(table[1].data());
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, FrameSize);
}

}  // namespace idle::glass

