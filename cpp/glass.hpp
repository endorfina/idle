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

template <typename... Appendages>
struct bone
{
    float length;
    point_3d_t angle, lower_bound, upper_bound;
    float stiffness;
    std::tuple<Appendages...> appendages;

    constexpr bone() :
        length(10),
        angle{0, 0, 0},
        lower_bound{0, 0, 0},
        upper_bound{0, 0, 0},
        stiffness{0},
        appendages{}
    {}

    template<unsigned Index>
    constexpr auto& get()
    {
        static_assert(Index < sizeof...(Appendages));
        return std::get<Index>(appendages);
    }

    static constexpr unsigned size = (1 + ... + Appendages::size);
};

template <unsigned FrameSize, unsigned AnimLength>
using atable = std::array<std::array<float, FrameSize * 2>, AnimLength>;

template<typename Value>
struct symmetry
{
    Value left, right;

    static constexpr unsigned size = Value::size * 2;
};

template<typename... Links>
struct muscle
{
    std::tuple<Links...> chain;

    template<typename... Li>
    constexpr muscle(Li&&... var)
        : chain{std::forward<Li>(var)...}
    {}

protected:
    template<unsigned Index = 0, typename Load>
    constexpr void expand(Load& load) const
    {
        constexpr auto source_index = Index > 0 ? Index - 1 : 0;
        load[Index] = std::get<Index>(chain)(load[source_index]);

        if constexpr (Index + 1 < sizeof...(Links))
        {
            expand<Index + 1, Load>(load);
        }
    }

public:
    template<typename Val>
    using array_t = std::array<Val, sizeof...(Links)>;

    template<typename Cargo>
    constexpr array_t<Cargo> link(const Cargo& cargo) const
    {
        array_t<Cargo> out{};
        out[0] = cargo;
        expand<0>(out);
        return out;
    }
};

template<unsigned Index = 0, typename Callable, typename...Nodes>
constexpr void apply_for_all(const Callable& func, bone<Nodes...>& node)
{
    if constexpr (Index == 0)
    {
        func(node);
    }

    if constexpr (sizeof...(Nodes) > 0)
    {
        apply_for_all<0>(func, node.template get<Index>());

        if constexpr (Index + 1 < sizeof...(Nodes))
            apply_for_all<Index + 1>(func, node);
    }
}

namespace spine
{

struct humanoid
{
    using limb_bone = bone<bone<bone<>>>;
    symmetry<limb_bone> arms, legs;
    bone<> head;
    point_3d_t center, rotation;

    static constexpr humanoid get_default()
    {
        humanoid hu{};
        constexpr auto mat = mat4x4_t::rotate(0);

        apply_for_all([](auto& b){ b.length = 15; }, hu.arms.left);
        apply_for_all([](auto& b){ b.length = 15; }, hu.arms.right);

        apply_for_all([](auto& b){ b.length = 20; }, hu.legs.left);
        apply_for_all([](auto& b){ b.length = 20; }, hu.legs.right);
        return hu;
    }
};

}  // namespace spine

template <unsigned Steps>
struct blob
{
    std::array<float, Steps> data;

    constexpt blob(const float radius)
        : data{}
    {
        for (auto& it : data)
            it = radius;
    }
};

template <typename T>
struct flesh
{

};

template<unsigned FrameSize, unsigned AnimLength>
void draw(const graphics::double_vertex_program_t& program, const atable<FrameSize, AnimLength>& table)
{
    constexpr blob<5> b(5);


    program.position_vertex(table[0].data());
    program.destination_vertex(table[1].data());
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, FrameSize);
}

}  // namespace idle::glass

