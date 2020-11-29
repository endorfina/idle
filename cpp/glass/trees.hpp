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

#include "blocks.hpp"

namespace idle::glass
{

namespace meta
{

struct index_pair
{
    unsigned table, lengths;
};

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
        table[index.table++] = rot * point_3d_t{};
        return index;
    }

    template<unsigned Size>
    constexpr meta::index_pair to_lines(meta::index_pair index, const blocks::segment<Size>& s, mat4x4_noopt_t mat) noexcept
    {
        for (const auto& it : s.table)
        {
            mat.reverse_multiply(it.get_transform());
            table[index.table++] = mat * point_3d_t{};
        }

        return index;
    }

    template<typename...Vars>
    constexpr meta::index_pair to_lines(meta::index_pair index, const blocks::joint<Vars...>& j, const mat4x4_noopt_t& mat) noexcept
    {
        const auto rot = j.root.get_transform() * mat;
        const auto branchoff_point = index.table;

        table[index.table++] = rot * point_3d_t{};
        index = to_lines(index, j.template branch<0>(), rot);

        if constexpr (sizeof...(Vars) > 1)
        {
            utility::tuple_visit<1>([&](const auto& it)
                {
                    lengths[index.lengths++] = index.table;
                    table[index.table++] = table[branchoff_point];
                    index = to_lines(index, it, rot);
                },
                j.branches);
        }
        return index;
    }

    template<typename Val>
    constexpr meta::index_pair to_lines(meta::index_pair index, const blocks::symmetry<Val>& s, const mat4x4_noopt_t& mat) noexcept
    {
        const auto branchoff_point = index.table;
        index = to_lines(index, s.left, mat);
        lengths[index.lengths++] = index.table;
        table[index.table++] = table[branchoff_point - 1];
        return to_lines(index, s.right, mat);
    }

    template<auto Label, typename Elem>
    constexpr meta::index_pair to_lines(const meta::index_pair index, const blocks::label<Label, Elem>& l, const mat4x4_noopt_t& mat) noexcept
    {
        return to_lines(index, l.elem, mat);
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
#ifdef __cpp_lib_constexpr_algorithms
        std::transform(source.table.begin(), source.table.end(), table.begin(), meta::flatten);
#else
        for (unsigned i = 0; i < table.size(); ++i)
        {
            table[i] = meta::flatten(source.table[i]);
        }
#endif
    }
};

}  // namespace idle::glass

