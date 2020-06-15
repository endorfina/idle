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
#include "idle_defines.hpp"

namespace idle::glass
{

namespace blocks
{

struct bone
{
    float length = 10;
    point_3d_t angle; //, lower_bound, upper_bound;
    //float stiffness = 0;

    static constexpr unsigned size = 1;
    static constexpr unsigned oddness = 0;
    static constexpr unsigned prime_branch_len = 1;
};

template<typename...Appendages>
struct joint
{
    static_assert(sizeof...(Appendages) > 0);
    bone root;

    using tuple_type = std::tuple<Appendages...>;
    tuple_type links;

    template<unsigned Index = 0>
    constexpr auto& link()
    {
        static_assert(Index < sizeof...(Appendages));
        return std::get<Index>(links);
    }

    template<unsigned Index = 0>
    constexpr auto& link() const
    {
        static_assert(Index < sizeof...(Appendages));
        return std::get<Index>(links);
    }

    static constexpr unsigned size = (1 + ... + Appendages::size);
    static constexpr unsigned oddness = ((sizeof...(Appendages) - 1) + ... + Appendages::oddness);
    static constexpr unsigned prime_branch_len = 1 + std::tuple_element<0, tuple_type>::type::prime_branch_len;
};

struct arm : joint<joint<bone>>
{
    constexpr void set_bone_length(const float len)
    {
        root.length = len;
        link<0>().root.length = len;
        link<0>().link<0>().length = len / 2.f;
    }
};

template<typename Value>
struct symmetry
{
    Value left, right;

    static constexpr unsigned size = Value::size * 2;
    static constexpr unsigned oddness = Value::oddness * 2 + 1;
    static constexpr unsigned prime_branch_len = Value::prime_branch_len;
};

}  // namespace blocks

namespace meta
{

template<typename Callable>
constexpr void apply_for_all(const Callable& func, blocks::bone& node)
{
    func(node);
}

template<unsigned Index = 0, typename Callable, typename...Nodes>
constexpr void apply_for_all(const Callable& func, blocks::joint<Nodes...>& node)
{
    if constexpr (Index == 0)
    {
        func(node.root);
    }

    if constexpr (sizeof...(Nodes) > 0)
    {
        apply_for_all(func, node.template link<Index>());

        if constexpr (Index + 1 < sizeof...(Nodes))
            apply_for_all<Index + 1>(func, node);
    }
}

}  // namespace meta

template <unsigned FrameSize, unsigned AnimLength>
using atable = std::array<std::array<float, FrameSize * 2>, AnimLength>;

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

namespace closet
{

struct humanoid : blocks::joint
                <
                    blocks::joint
                    <
                        blocks::symmetry<blocks::joint<blocks::arm>>
                    >,

                    blocks::joint
                    <
                        blocks::symmetry<blocks::joint<blocks::arm>>
                    >
                >
{
    constexpr decltype(auto) get_upperbody()
    {
        return link<0>();
    }

    constexpr decltype(auto) get_lowerbody()
    {
        return link<1>();
    }

    constexpr decltype(auto) get_shoulders()
    {
        return get_upperbody().link<0>();
    }

    constexpr decltype(auto) get_hips()
    {
        return get_lowerbody().link<0>();
    }

    static constexpr humanoid get_default()
    {
        humanoid hu;
        // hu.head.length = 10;

        hu.root.length = 35;

        auto& ub = hu.get_upperbody().root;
        auto& lb = hu.get_lowerbody().root;

        ub.length = lb.length = 5.f;

        lb.angle.y = F_TAU_2;

        auto& sh = hu.get_shoulders();
        auto& hp = hu.get_hips();

        blocks::arm& left_arm = sh.left.link<0>();
        blocks::arm& right_arm = sh.right.link<0>();
        blocks::arm& left_leg = hp.left.link<0>();
        blocks::arm& right_leg = hp.right.link<0>();

        left_arm.set_bone_length(10);
        right_arm.set_bone_length(10);

        left_arm.root.angle = {F_TAU / 8, 0, 0};
        left_arm.link<0>().root.angle = {F_TAU / 8, 0, 0};
        right_arm.root.angle.x = - left_arm.root.angle.x;
        right_arm.link<0>().root.angle.x = - left_arm.link<0>().root.angle.x;

        left_leg.set_bone_length(15);
        right_leg.set_bone_length(15);

        left_leg.root.angle = {F_TAU / 16, 0, 0};
        right_leg.root.angle.x = - left_leg.root.angle.x;

        return hu;
    }
};

}  // namespace closet

template <unsigned Steps>
struct blob
{
    std::array<float, Steps> data;

    constexpr blob(const float radius)
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

