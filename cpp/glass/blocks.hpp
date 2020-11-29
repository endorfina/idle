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
#include <math_defines.hpp>
#include "utility.hpp"

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

#if !__cpp_lib_constexpr_tuple
    constexpr joint& operator=(const joint& other) noexcept
    {
        root = other.root;
        utility::tuple_copy(branches, other.branches);
        return *this;
    }
#endif

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

template<auto Label, typename Elem>
struct label
{
    Elem elem;

    static constexpr unsigned size = Elem::size;
    static constexpr unsigned oddness = Elem::oddness;
    static constexpr unsigned prime_branch_len = Elem::prime_branch_len;
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

template<auto Label, typename Elem, typename Callable>
constexpr void apply_for_all(const Callable& func, blocks::label<Label, Elem>& label) noexcept
{
    apply_for_all(func, label.elem);
}


template<auto Label, typename T = void>
inline constexpr bool has_label = false;

template<auto Label, typename...Nodes>
inline constexpr bool has_label<Label, blocks::joint<Nodes...>> = (false || ... || has_label<Label, Nodes>);

template<auto Key, auto Label, typename Elem>
inline constexpr bool has_label<Key, blocks::label<Label, Elem>> = (Key == Label) || has_label<Key, Elem>;


template<auto Key, auto Label, typename Elem>
constexpr auto& find_label(blocks::label<Label, Elem>& label) noexcept;

template<auto Label, unsigned Index = 0, typename...Nodes>
constexpr auto& find_label(blocks::joint<Nodes...>& node) noexcept
{
    static_assert(sizeof...(Nodes) > 0);

    if constexpr (has_label<Label, typename std::tuple_element<Index, typename blocks::joint<Nodes...>::tuple_type>::type>)
    {
        return find_label<Label>(node.template branch<Index>());
    }
    else
    {
        static_assert(Index + 1 < sizeof...(Nodes));
        return find_label<Label, Index + 1>(node);
    }
}

template<auto Key, auto Label, typename Elem>
constexpr auto& find_label(blocks::label<Label, Elem>& label) noexcept
{
    if constexpr (Key == Label)
    {
        return label.elem;
    }
    else
    {
        return find_label<Key>(label.elem);
    }
}

template<typename Val>
constexpr void sync_right(blocks::symmetry<Val>& sym) noexcept
{
    sym.right = sym.left;
    apply_for_all([](auto& b) { b.angle *= point_3d_t{-1.f, 1.f, -1.f}; }, sym.right);
}


struct symmetry_index
{
    int left, right;
};

constexpr symmetry_index operator+(const unsigned offset, symmetry_index index) noexcept
{
    index.left += offset;
    index.right += offset;
    return index;
}

template<auto Key, typename Elem>
struct label_index_helper
{
    // static constexpr unsigned inc = Elem::size;
};

template<auto Key>
struct label_index_helper<Key, blocks::bone>
{
    static constexpr unsigned inc = 1;
};

template<auto Key, auto Size>
struct label_index_helper<Key, blocks::segment<Size>>
{
    static constexpr unsigned inc = Size;
};

template<auto Key, typename Elem>
struct label_index_helper<Key, blocks::symmetry<Elem>>
{
    static constexpr unsigned inc = label_index_helper<Key, Elem>::inc * 2 + 1;
};

template<auto Key, typename...Nodes>
struct label_index_helper<Key, blocks::joint<Nodes...>>
{
    static_assert(sizeof...(Nodes) > 0);

private:
    template<unsigned Index = 0>
    static constexpr auto get_index(const unsigned index) noexcept
    {
        using elem_type = typename std::tuple_element<Index, typename blocks::joint<Nodes...>::tuple_type>::type;

        if constexpr (Index + 1 < sizeof...(Nodes) && !has_label<Key, elem_type>)
        {
            const auto new_index = index + 1 + label_index_helper<Key, elem_type>::inc;
            return get_index<Index + 1>(new_index);
        }
        else
        {
            return index + 1 + label_index_helper<Key, elem_type>::inc;
        }
    }

public:
    static constexpr auto inc = get_index<0>(0);
};

template<auto Key, auto Label, typename Elem>
struct label_index_helper<Key, blocks::label<Label, blocks::symmetry<Elem>>>
{
    static constexpr auto inc = []
    {
        if constexpr (Key == Label)
        {
            return symmetry_index{ -1, label_index_helper<Key, Elem>::inc };
        }
        else
        {
            return label_index_helper<Key, blocks::symmetry<Elem>>::inc;
        }
    }();
};

template<auto Key, auto Label, typename Elem>
struct label_index_helper<Key, blocks::label<Label, Elem>>
{
    static constexpr auto inc = []
    {
        if constexpr (Key == Label)
        {
            return int(-1);
        }
        else
        {
            return label_index_helper<Key, Elem>::inc;
        }
    }();
};


}  // namespace meta

template<auto Label, class Struct, typename = std::enable_if_t<meta::has_label<Label, Struct>>>
inline constexpr auto label_index = meta::label_index_helper<Label, Struct>::inc;

}  // namespace idle::glass

