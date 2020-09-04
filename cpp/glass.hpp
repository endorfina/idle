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
constexpr void tuple_visit(const Callable& call, const std::tuple<Vars...>& tuple) noexcept
{
    call(std::get<I>(tuple));

    if constexpr (I + 1 < sizeof...(Vars))
    {
        tuple_visit<I + 1, Callable, Vars...>(call, tuple);
    }
}

template<std::size_t I = 0, typename...Vars>
constexpr void tuple_copy(std::tuple<Vars...>& dest, const std::tuple<Vars...>& src) noexcept
{
    std::get<I>(dest) = std::get<I>(src);

    if constexpr (I + 1 < sizeof...(Vars))
    {
        tuple_copy<I + 1, Vars...>(dest, src);
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

    constexpr auto get_transform() const noexcept
    {
        math::matrix4x4<float, 1> out{};
        // math::transform::translate(out, {0, 0, length});
        out[14] = length;

        math::transform::rotate_x(out, angle.x);
        math::transform::rotate_y(out, angle.y);
        math::transform::rotate_z(out, angle.z);
        return out;
    }
};

template<typename...Appendages>
struct joint
{
    static_assert(sizeof...(Appendages) > 0);
    bone root;

    using tuple_type = std::tuple<Appendages...>;
    tuple_type branches;

    template<unsigned Index = 0>
    constexpr auto& branch() noexcept
    {
        static_assert(Index < sizeof...(Appendages));
        return std::get<Index>(branches);
    }

    template<unsigned Index = 0>
    constexpr auto& branch() const noexcept
    {
        static_assert(Index < sizeof...(Appendages));
        return std::get<Index>(branches);
    }

    constexpr joint& operator=(const joint& other) noexcept
    {
        root = other.root;
        tuple_copy(branches, other.branches);
        return *this;
    }

    static constexpr unsigned size = (1 + ... + Appendages::size);
    static constexpr unsigned oddness = ((sizeof...(Appendages) - 1) + ... + Appendages::oddness);
    static constexpr unsigned prime_branch_len = 1 + std::tuple_element<0, tuple_type>::type::prime_branch_len;
};

template<unsigned Size>
struct segment
{
    static_assert(Size > 0);

    std::array<bone, Size> table;

    constexpr const bone& operator[](unsigned pos) const noexcept
    {
        return table[pos];
    }

    constexpr bone& operator[](unsigned pos) noexcept
    {
        return table[pos];
    }

    static constexpr unsigned size = Size;
    static constexpr unsigned oddness = 0;
    static constexpr unsigned prime_branch_len = Size;
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
constexpr void apply_for_all(const Callable& func, blocks::bone& node) noexcept
{
    func(node);
}

template<unsigned Size, typename Callable>
constexpr void apply_for_all(const Callable& func, blocks::segment<Size>& node) noexcept
{
    for (auto& it : node.table)
    {
        func(it);
    }
}

template<unsigned Index = 0, typename Callable, typename...Nodes>
constexpr void apply_for_all(const Callable& func, blocks::joint<Nodes...>& node) noexcept
{
    if constexpr (Index == 0)
    {
        func(node.root);
    }

    if constexpr (sizeof...(Nodes) > 0)
    {
        apply_for_all(func, node.template branch<Index>());

        if constexpr (Index + 1 < sizeof...(Nodes))
            apply_for_all<Index + 1>(func, node);
    }
}

template<typename Val>
constexpr void sync_right(blocks::symmetry<Val>& sym) noexcept
{
    sym.right = sym.left;
    apply_for_all([](auto& b) { b.angle *= point_3d_t{-1.f, 1.f, -1.f}; }, sym.right);
}

using index_pair = std::array<unsigned, 2>;

constexpr point_t flatten(const point_3d_t p) noexcept
{
    return { p.y, - p.z };
}

}  // namespace meta


template<typename...Js>
struct deep_tree
{
    using joint_type = blocks::joint<Js...>;

    std::array<point_3d_t, joint_type::size + joint_type::oddness> table{};
    std::array<unsigned, 1 + joint_type::oddness> lengths{};

private:
    constexpr meta::index_pair to_lines(meta::index_pair index, const blocks::bone& b, const mat4x4_noopt_t& mat) noexcept
    {
        const auto rot = b.get_transform() * mat;
        table[index[0]++] = rot * point_3d_t{};
        return index;
    }

    template<unsigned Size>
    constexpr meta::index_pair to_lines(meta::index_pair index, const blocks::segment<Size>& s, mat4x4_noopt_t mat) noexcept
    {
        for (const auto& it : s.table)
        {
            mat.reverse_multiply(it.get_transform());
            table[index[0]++] = mat * point_3d_t{};
        }

        return index;
    }

    template<typename...Vars>
    constexpr meta::index_pair to_lines(meta::index_pair index, const blocks::joint<Vars...>& j, const mat4x4_noopt_t& mat) noexcept
    {
        const auto rot = j.root.get_transform() * mat;
        const auto branchoff_point = index[0];

        table[index[0]++] = rot * point_3d_t{};
        index = to_lines(index, j.template branch<0>(), rot);

        if constexpr (sizeof...(Vars) > 1)
        {
            tuple_visit<1>([&](const auto& it)
                {
                    lengths[index[1]++] = index[0];
                    table[index[0]++] = table[branchoff_point];
                    index = to_lines(index, it, rot);
                },
                j.branches);
        }
        return index;
    }

    template<typename Val>
    constexpr meta::index_pair to_lines(meta::index_pair index, const blocks::symmetry<Val>& s, const mat4x4_noopt_t& mat) noexcept
    {
        const auto branchoff_point = index[0];
        index = to_lines(index, s.left, mat);
        lengths[index[1]++] = index[0];
        table[index[0]++] = table[branchoff_point - 1];
        return to_lines(index, s.right, mat);
    }

public:
    constexpr deep_tree(const joint_type& root, const mat4x4_noopt_t& mat) noexcept
    {
        const auto [total_length, last_iter] = to_lines({0, 0}, root, mat);
        lengths[last_iter] = total_length;

        for (auto i = lengths.size() - 1; i > 0; --i)
        {
            lengths[i] -= lengths[i - 1];
        }
    }
};

template<typename...Js>
struct flat_tree
{
    using joint_type = blocks::joint<Js...>;

    std::array<point_t, joint_type::size + joint_type::oddness> table{};
    std::array<unsigned, 1 + joint_type::oddness> lengths;

    explicit constexpr flat_tree(const deep_tree<Js...>& source) noexcept
        : lengths{source.lengths}
    {
        // std::transform is unavailable in C++17

        for (unsigned i = 0; i < table.size(); ++i)
        {
            table[i] = meta::flatten(source.table[i]);
        }
    }
};

template<typename... Links>
struct muscle
{
    std::tuple<Links...> chain;

    constexpr muscle(std::tuple<Links...> var) noexcept
        : chain{std::move(var)}
    {}

protected:
    template<unsigned Index = 0, typename Load>
    constexpr void expand(Load& load) const noexcept
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
    constexpr array_t<Cargo> animate(const Cargo& cargo) const noexcept
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
                        blocks::segment<2>,
                        blocks::symmetry<blocks::segment<4>>
                    >,

                    blocks::joint
                    <
                        blocks::symmetry<blocks::segment<4>>
                    >
                >
{
    constexpr decltype(auto) get_upperbody() noexcept
    {
        return branch<0>();
    }

    constexpr decltype(auto) get_lowerbody() noexcept
    {
        return branch<1>();
    }

    constexpr decltype(auto) get_shoulders() noexcept
    {
        return get_upperbody().branch<1>();
    }

    constexpr decltype(auto) get_head() noexcept
    {
        return get_upperbody().branch<0>();
    }

    constexpr decltype(auto) get_hips() noexcept
    {
        return get_lowerbody().branch<0>();
    }

    constexpr void realign() noexcept
    {
        root.length = 1;

        const deep_tree tree(*this, {});
        const auto low = std::min_element(
                tree.table.cbegin(),
                tree.table.cend(),
                [] (const auto& lhs, const auto& rhs) { return lhs.z < rhs.z; }
            )->z;

        if (low < 0)
            root.length -= low;
    }

    constexpr humanoid() noexcept
    {
        auto& [neck, face] = get_head().table;
        neck.length = 12;
        face.length = 14;
        face.angle.y = - (neck.angle.y = math::tau / 30);

        auto& ub = get_upperbody().root;
        auto& lb = get_lowerbody().root;

        ub.length = lb.length = 15;

        lb.angle.y = math::tau_2;

        auto& sh = get_shoulders();
        auto& hp = get_hips();

        auto& arm = sh.left.table;
        auto& leg = hp.left.table;

        arm = {
            blocks::bone{ 7.f, { -math::tau_4, 0, 0 } },
            blocks::bone{ 11.f, { -math::tau_8, 0, 0 } },
            blocks::bone{ 11.f, { 0, 0, 0 } },
            blocks::bone{ 5.f, { -math::tau_8 / 8, math::tau_8 / 8, 0 } }
        };

        constexpr float
                knee_bend = math::tau_4 * .12f,
                thigh_raise = math::tau_4 * .08f;

        leg = {
            blocks::bone{ 6.f, { -math::tau_4, 0, 0 } },
            blocks::bone{ 18.f, { math::tau_4 * .966f, 0, -thigh_raise } },
            blocks::bone{ 18.f, { 0, knee_bend, 0 } },
            blocks::bone{ 7.f, { 0, - knee_bend + thigh_raise - math::tau_4, 0 } }
        };

        meta::sync_right(sh);
        meta::sync_right(hp);

        realign();
    }
};

}  // namespace closet

template <unsigned Steps>
struct blob
{
    std::array<float, Steps> data;

    constexpr blob(const float radius) noexcept
        : data{}
    {
        for (auto& it : data)
            it = radius;
    }
};

}  // namespace idle::glass

