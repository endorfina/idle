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

template<std::size_t I = 0, typename Callable, typename...Vars>
constexpr void tuple_visit(const Callable& call, const std::tuple<Vars...>& tuple)
{
    call(std::get<I>(tuple));

    if constexpr (I + 1 < sizeof...(Vars))
    {
        tuple_visit<I + 1, Callable, Vars...>(call, tuple);
    }
}

template<std::size_t I = 0, typename...Vars>
constexpr void tuple_copy(std::tuple<Vars...>& dest, const std::tuple<Vars...>& src)
{
    std::get<I>(dest) = std::get<I>(src);

    if constexpr (I + 1 < sizeof...(Vars))
    {
        tuple_visit<I + 1, Vars...>(dest, src);
    }
}

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

    constexpr joint& operator=(const joint& other)
    {
        root = other.root;
        tuple_copy(links, other.links);
        return *this;
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

template<typename Val>
constexpr void sync_right(blocks::symmetry<Val>& sym)
{
    sym.right = sym.left;
    apply_for_all([](auto& b) { b.angle *= math::point3<float>{-1.f, 1.f, -1.f}; }, sym.right);
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
                        blocks::joint<blocks::bone>,
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
        return get_upperbody().link<1>();
    }

    constexpr decltype(auto) get_head()
    {
        return get_upperbody().link<0>();
    }

    constexpr decltype(auto) get_hips()
    {
        return get_lowerbody().link<0>();
    }

    constexpr void align_feet()
    {
    }

    constexpr humanoid()
    {
        get_head().root.length = 10;
        get_head().link<0>().length = 15;
        get_head().link<0>().angle.y = - (get_head().root.angle.y = F_TAU / 30);

        root.length = 40;

        auto& ub = get_upperbody().root;
        auto& lb = get_lowerbody().root;

        ub.length = lb.length = 15;

        lb.angle.y = F_TAU_2;

        auto& sh = get_shoulders();
        auto& hp = get_hips();

        blocks::arm& arm = sh.left.link<0>();
        blocks::arm& leg = hp.left.link<0>();

        arm.set_bone_length(10);

        sh.left.root.length = 7;

        sh.left.root.angle = {F_TAU / -4, 0, 0};
        arm.root.angle = {F_TAU / -5, F_TAU / -12, 0};
        arm.link<0>().root.angle = {F_TAU / 20, F_TAU / 12, 0};
        arm.link<0>().link<0>().angle.y = F_TAU / 12;

        leg.set_bone_length(18);

        hp.left.root.length = 7;

        hp.left.root.angle = { F_TAU / -5, 0, 0};
        leg.root.angle = { F_TAU / 5.5f, F_TAU / -20, 0};
        leg.link<0>().root.angle = { 0.f, F_TAU / 16, F_TAU / -20 };
        leg.link<0>().link<0>().angle.y = F_TAU / -4;

        meta::sync_right(sh);
        meta::sync_right(hp);

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

