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

struct index_pair
{
    unsigned table, lengths;
};

constexpr point_t flatten(const point_3d_t p) noexcept
{
    return { p.y, - p.z };
}

template<auto Key>
constexpr unsigned get_label_index(const unsigned index, const blocks::bone& b) noexcept
{
    return index + 1;
}

template<auto Key, unsigned Size>
constexpr unsigned get_label_index(const unsigned index, const blocks::segment<Size>& s) noexcept
{
    return index + Size;
}

template<auto Key, typename Val>
constexpr unsigned get_label_index(unsigned index, const blocks::symmetry<Val>& s) noexcept;

template<auto Key, auto Label, typename Elem>
constexpr unsigned get_label_index(const unsigned index, const blocks::label<Label, Elem>& l) noexcept;

template<auto Label, unsigned Index = 0, typename...Nodes>
constexpr unsigned get_label_index(const unsigned index, const blocks::joint<Nodes...>& node) noexcept
{
    static_assert(sizeof...(Nodes) > 0);

    if constexpr (Index + 1 < sizeof...(Nodes)
            && !has_label<Label, typename std::tuple_element<Index, typename blocks::joint<Nodes...>::tuple_type>::type>)
    {
        const auto new_index = get_label_index<Label>(index + 1, node.template branch<Index>());
        return get_label_index<Label, Index + 1>(new_index, node);
    }
    else
    {
        return get_label_index<Label>(index + 1, node.template branch<Index>());
    }
}

template<auto Key, typename Val>
constexpr unsigned get_label_index(unsigned index, const blocks::symmetry<Val>& s) noexcept
{
    index = get_label_index<Key>(index, s.left);
    return get_label_index<Key>(index + 1, s.right);
}

template<auto Key, auto Label, typename Elem>
constexpr unsigned get_label_index(const unsigned index, const blocks::label<Label, Elem>& l) noexcept
{
    if constexpr (Key == Label)
    {
        return index > 0 ? index - 1 : 0;
    }
    else
    {
        return get_label_index<Key>(index, l.elem);
    }
}

}  // namespace meta

template<auto Label, class Struct, typename = std::enable_if_t<meta::has_label<Label, Struct>>>
inline constexpr unsigned label_index = meta::get_label_index<Label>(0u, Struct{});

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
            tuple_visit<1>([&](const auto& it)
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


enum struct parts
{
    shoulders,
    hips,
    upperbody,
    lowerbody,
    head
};

namespace closet
{

struct humanoid : blocks::joint
                <
                    blocks::label<parts::upperbody, blocks::joint
                    <
                        blocks::label<parts::head, blocks::segment<2>>,
                        blocks::label<parts::shoulders, blocks::symmetry<blocks::segment<4>>>
                    >>,

                    blocks::label<parts::lowerbody, blocks::joint
                    <
                        blocks::label<parts::hips, blocks::symmetry<blocks::segment<4>>>
                    >>
                >
{
    template<auto Key>
    constexpr const auto& get_ref() const noexcept
    {
        return meta::find_label<Key>(*this);
    }

    template<auto Key>
    constexpr auto& get_ref() noexcept
    {
        return meta::find_label<Key>(*this);
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
        auto& [neck, face] = get_ref<parts::head>().table;
        neck.length = 12;
        face.length = 14;
        face.angle.y = - (neck.angle.y = math::tau / 30);

        auto& ub = get_ref<parts::upperbody>().root;
        auto& lb = get_ref<parts::lowerbody>().root;

        ub.length = lb.length = 15;

        lb.angle.y = math::tau_2;

        auto& sh = get_ref<parts::shoulders>();
        auto& hp = get_ref<parts::hips>();

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

