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

#include <guard.hpp>

#include "blocks.hpp"
#include "trees.hpp"
#include "rigs.hpp"

namespace idle::glass
{

namespace meta
{

inline constexpr mat4x4_noopt_t skew_matrix = math::matrices::rotate<GLfloat>(math::tau_8) * math::matrices::rotate_y<GLfloat>(math::tau_4 / 3);

template<typename P, typename S, auto...Is>
constexpr auto skew_index(const P& paint, const S& source, const mat4x4_noopt_t& mat, std::index_sequence<Is...>) noexcept
{
    return std::array{
        paint(
            glass::deep_tree(
                source[Is],
                mat
            )) ...
    };
}

template<auto Size>
struct strip_mesh
{
    using array_t = std::array<point_t, Size>;
    array_t mesh;

    template<typename Core>
    void draw(const Core& gl, const strip_mesh& another) const noexcept
    {
        gl.prog.double_fill.position_vertex(reinterpret_cast<const GLfloat*>(mesh.data()));
        gl.prog.double_fill.destination_vertex(reinterpret_cast<const GLfloat*>(another.mesh.data()));
        gl::DrawArrays(gl::TRIANGLE_STRIP, 0, Size);
    }
};

}  // namespace meta

template<unsigned...Deg, typename Painter, typename Rig, auto Size>
constexpr auto make(const Painter& paint, const std::array<Rig, Size>& anim) noexcept
{
    const auto face = [&paint, &anim] (const float deg)
    {
        return meta::skew_index(
                paint,
                anim,
                math::matrices::rotate<float>(math::degtorad<float>(deg)) * meta::skew_matrix,
                std::make_index_sequence<Size>{});
    };

    return std::array { face(static_cast<float>(Deg)) ... };
}

template<typename... Links>
struct blob_mesh
{
    using tuple_type = std::tuple<Links...>;
    tuple_type chain;

    constexpr blob_mesh(tuple_type var) noexcept
        : chain{std::move(var)}
    {}

    static constexpr unsigned output_nodes = (0 + ... + Links::output_nodes);
    static_assert(output_nodes > 0);

    using output_array_t = std::array<point_t, output_nodes>;

protected:
    template<typename Val, auto Size, unsigned Index = 0>
    constexpr void expand(output_array_t& out, const unsigned index, const std::array<Val, Size>& input) const noexcept
    {
        using frag_type = typename std::tuple_element<Index, tuple_type>::type;
        const frag_type& fragment = std::get<Index>(chain);

        if constexpr (frag_type::input_nodes == 1)
        {
            fragment.chew(out, index, input[fragment.input_index]);
        }

        if constexpr (Index + 1 < sizeof...(Links))
        {
            expand<Val, Size, Index + 1>(out, index + frag_type::output_nodes, input);
        }
    }

public:
    template<typename Val, auto Size>
    constexpr auto compose(const std::array<Val, Size>& input) const noexcept
    {
        meta::strip_mesh<output_nodes> out{};
        expand<Val, Size>(out.mesh, 0, input);
        return out;
    }
};

namespace meta
{

constexpr point_t parallel_vec(const point_t a, const point_t b) noexcept
{
    const point_t inv_vec{ a.x - b.x, a.y - b.y };
    return inv_vec / math::const_math::sqrt(inv_vec.x * inv_vec.x + inv_vec.y * inv_vec.y);
}

constexpr point_t perpendicular_vec(const point_t a, const point_t b) noexcept
{
    const point_t inv_vec{ b.y - a.y, a.x - b.x };
    return inv_vec / math::const_math::sqrt(inv_vec.x * inv_vec.x + inv_vec.y * inv_vec.y);
}

struct mesh_node
{
    point_t pt, pp, pl;
};

template<auto Size, typename = std::enable_if_t<(Size > 1)>>
constexpr std::array<mesh_node, Size> extrapolate_vectors(const std::array<point_t, Size>& input) noexcept
{
    std::array<mesh_node, Size> out;
    for (unsigned i = 0; i < Size - 1; ++i)
    {
        out[i] = {
            input[i],
            perpendicular_vec(input[i], input[i + 1]),
            parallel_vec(input[i], input[i + 1])
        };
    }
    out[Size - 1] = out[Size - 2];
    out[Size - 1].pt = input[Size - 1];
    return out;
}

template<auto Size, typename = std::enable_if_t<(Size > 2)>>
constexpr std::array<mesh_node, Size> smoothen_perpendicular_vectors(const std::array<mesh_node, Size>& input) noexcept
{
    std::array<mesh_node, Size> out = input;
    for (unsigned i = 1; i < Size - 1; ++i)
    {
        out[i].pp = (input[i - 1].pp + input[i].pp + input[i + 1].pp) / 3.f;
    }
    return out;
}

}  // namespace meta

namespace skin
{

struct sym
{
    float width;
    unsigned input_index;

    static constexpr unsigned output_nodes = 2;
    static constexpr unsigned input_nodes = 1;

    template<auto Size>
    constexpr void chew(std::array<point_t, Size>& out, unsigned index, const meta::mesh_node& input) const noexcept
    {
        const auto shift_vec = input.pp * (width / 2);
        out[index] = input.pt - shift_vec;
        out[index + 1] = input.pt + shift_vec;
    }
};

}  // namespace skin

namespace paint
{

inline constexpr auto human = [] (const auto& tree)
{
    using joint_type = typename idle_remove_cvr(tree)::joint_type;
    constexpr unsigned head_index = glass::label_index<glass::parts::head, joint_type>;
    const std::array<point_t, 3> neck
    {
        glass::meta::flatten(tree.table[head_index]),
        glass::meta::flatten(tree.table[head_index + 1]),
        glass::meta::flatten(tree.table[head_index + 2])
    };

    constexpr blob_mesh mesh(
            std::make_tuple(
                skin::sym{3.f, 0},
                skin::sym{9.f, 1},
                skin::sym{5.f, 2}
            ));

    const auto input_vecs = meta::smoothen_perpendicular_vectors(meta::extrapolate_vectors(neck));

    return mesh.compose(input_vecs);
};

}  // namespace paint

}  // namespace idle::glass

