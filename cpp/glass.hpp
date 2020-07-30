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

    constexpr auto get_transform() const
    {
        return math::matrices::translate<float>({0, 0, length})
            * math::matrices::rotate_x<float>(angle.x)
            * math::matrices::rotate_y<float>(angle.y)
            * math::matrices::rotate<float>(angle.z);
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
    constexpr auto& branch()
    {
        static_assert(Index < sizeof...(Appendages));
        return std::get<Index>(branches);
    }

    template<unsigned Index = 0>
    constexpr auto& branch() const
    {
        static_assert(Index < sizeof...(Appendages));
        return std::get<Index>(branches);
    }

    constexpr joint& operator=(const joint& other)
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

    constexpr const bone& operator[](unsigned pos) const
    {
        return table[pos];
    }

    constexpr bone& operator[](unsigned pos)
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
constexpr void apply_for_all(const Callable& func, blocks::bone& node)
{
    func(node);
}

template<unsigned Size, typename Callable>
constexpr void apply_for_all(const Callable& func, blocks::segment<Size>& node)
{
    for (auto& it : node.table)
    {
        func(it);
    }
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
        apply_for_all(func, node.template branch<Index>());

        if constexpr (Index + 1 < sizeof...(Nodes))
            apply_for_all<Index + 1>(func, node);
    }
}

template<typename Val>
constexpr void sync_right(blocks::symmetry<Val>& sym)
{
    sym.right = sym.left;
    apply_for_all([](auto& b) { b.angle *= point_3d_t{-1.f, 1.f, -1.f}; }, sym.right);
}

using index_pair = std::array<unsigned, 2>;

constexpr point_t flatten(const point_3d_t p)
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
    constexpr meta::index_pair to_lines(meta::index_pair index, const blocks::bone& b, const mat4x4_noopt_t& mat)
    {
        const auto rot = b.get_transform() * mat;
        table[index[0]++] = rot * point_3d_t{};
        return index;
    }

    template<unsigned Size>
    constexpr meta::index_pair to_lines(meta::index_pair index, const blocks::segment<Size>& s, mat4x4_noopt_t mat)
    {
        for (const auto& it : s.table)
        {
            mat.reverse_multiply(it.get_transform());
            table[index[0]++] = mat * point_3d_t{};
        }

        return index;
    }

    template<typename...Vars>
    constexpr meta::index_pair to_lines(meta::index_pair index, const blocks::joint<Vars...>& j, const mat4x4_noopt_t& mat)
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
    constexpr meta::index_pair to_lines(meta::index_pair index, const blocks::symmetry<Val>& s, const mat4x4_noopt_t& mat)
    {
        const auto branchoff_point = index[0];
        index = to_lines(index, s.left, mat);
        lengths[index[1]++] = index[0];
        table[index[0]++] = table[branchoff_point - 1];
        return to_lines(index, s.right, mat);
    }

public:
    constexpr deep_tree(const joint_type& root, const mat4x4_noopt_t& mat)
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

    explicit constexpr flat_tree(const deep_tree<Js...>& source)
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

    constexpr muscle(std::tuple<Links...> var)
        : chain{std::move(var)}
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
    constexpr array_t<Cargo> animate(const Cargo& cargo) const
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
    constexpr decltype(auto) get_upperbody()
    {
        return branch<0>();
    }

    constexpr decltype(auto) get_lowerbody()
    {
        return branch<1>();
    }

    constexpr decltype(auto) get_shoulders()
    {
        return get_upperbody().branch<1>();
    }

    constexpr decltype(auto) get_head()
    {
        return get_upperbody().branch<0>();
    }

    constexpr decltype(auto) get_hips()
    {
        return get_lowerbody().branch<0>();
    }

    constexpr void realign()
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

    constexpr humanoid()
    {
        auto& [neck, face] = get_head().table;
        neck.length = 12;
        face.length = 14;
        face.angle.y = - (neck.angle.y = F_TAU / 30);

        auto& ub = get_upperbody().root;
        auto& lb = get_lowerbody().root;

        ub.length = lb.length = 15;

        lb.angle.y = F_TAU_2;

        auto& sh = get_shoulders();
        auto& hp = get_hips();

        auto& arm = sh.left.table;
        auto& leg = hp.left.table;

        arm = {
            blocks::bone{ 6.f, { -F_TAU_4, 0, 0 } },
            blocks::bone{ 10.f, { -F_TAU_8, 0, 0 } },
            blocks::bone{ 10.f, { 0, 0, 0 } },
            blocks::bone{ 4.f, { 0, 0, 0 } }
        };

        leg = {
            blocks::bone{ 7.f, { -F_TAU_4, 0, 0 } },
            blocks::bone{ 18.f, { F_TAU_4 * .966f, 0, F_TAU_4 * -.1f } },
            blocks::bone{ 18.f, { 0, F_TAU_4 * .2f, 0 } },
            blocks::bone{ 8.f, { 0, F_TAU_4 * (.1f - .2f - 1.f), 0 } }
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

    constexpr blob(const float radius)
        : data{}
    {
        for (auto& it : data)
            it = radius;
    }
};

}  // namespace idle::glass

