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

template<typename Painter, typename Source, auto...Indices>
constexpr auto skew_index(const Painter& paint_vertex, const Source& key_frame_vertex_table, const mat4x4_noopt_t& base_matrix, const std::index_sequence<Indices...>) noexcept
{
    return std::array{
        paint_vertex(
            glass::deep_tree(
                key_frame_vertex_table[Indices],
                base_matrix
            )
        ) ...
    };
}

template<auto Size>
struct drawable_strip
{
    using vertex_table_type = std::array<point_t, Size>;
    vertex_table_type mesh;

    template<typename Program>
    void draw_elem(const Program& prog, const vertex_table_type& texture, const drawable_strip& another) const noexcept
    {
        prog.position_vertex(reinterpret_cast<const GLfloat*>(mesh.data()));
        prog.destination_vertex(reinterpret_cast<const GLfloat*>(another.mesh.data()));
        prog.texture_vertex(reinterpret_cast<const GLfloat*>(texture.data()));
        gl::DrawArrays(gl::TRIANGLE_STRIP, 0, Size);
    }
};

template<auto...Sizes>
struct drawable_strip_mesh
{
    using vertex_tuple_type = std::tuple<drawable_strip<Sizes>...>;
    using texture_tuple_type = std::tuple<typename drawable_strip<Sizes>::vertex_table_type...>;
    vertex_tuple_type strip_tuple;

    template<typename Program, unsigned Index = 0>
    void draw(const Program& program, const texture_tuple_type& texture_vertices, const drawable_strip_mesh& another) const noexcept
    {
        std::get<Index>(strip_tuple).draw_elem(program,
                                std::get<Index>(texture_vertices),
                                std::get<Index>(another.strip_tuple));

        if constexpr (Index + 1 < sizeof...(Sizes))
        {
            draw<Program, Index + 1>(program, texture_vertices, another);
        }
    }
};

}  // namespace meta

template<unsigned...Deg, typename Painter, typename Rig, auto Size>
constexpr auto make(const Painter& painter, const std::array<Rig, Size>& animation_frames) noexcept
{
    const auto face = [&] (const float radians)
    {
        return meta::skew_index(
                painter,
                animation_frames,
                math::matrices::rotate(radians) * meta::skew_matrix,
                std::make_index_sequence<Size>{});
    };

    return std::array { face(math::degtorad(static_cast<float>(Deg))) ... };
}

namespace poly
{

template<typename Transform, typename Skin, typename... Links>
struct blob_mesh
{
    using tuple_type = std::tuple<Links...>;

    Transform input_transform;
    Skin texture_geometry;
    tuple_type chain;

    constexpr blob_mesh(const Transform& lambda, const Skin& skin, const tuple_type& var) noexcept
        : input_transform{lambda}, texture_geometry{skin}, chain{var}
    {}

    static constexpr unsigned output_nodes = (0 + ... + Links::output_nodes);
    static_assert(output_nodes > 0);

    using output_array_t = std::array<point_t, output_nodes>;

protected:
    template<typename Val, auto Size, unsigned Index = 0>
    constexpr void vertex(output_array_t& out, const unsigned index, const std::array<Val, Size>& input) const noexcept
    {
        using frag_type = typename std::tuple_element<Index, tuple_type>::type;
        const frag_type& fragment = std::get<Index>(chain);

        if constexpr (frag_type::input_nodes == 1)
        {
            fragment.chew(out, index, input[fragment.input_index]);
        }

        if constexpr (Index + 1 < sizeof...(Links))
        {
            vertex<Val, Size, Index + 1>(out, index + frag_type::output_nodes, input);
        }
    }

    template<unsigned Index = 0>
    constexpr void texture(output_array_t& out, point_t pos, const unsigned index) const noexcept
    {
        using frag_type = typename std::tuple_element<Index, tuple_type>::type;
        const frag_type& fragment = std::get<Index>(chain);

        pos = texture_geometry.chew(out, pos, index, fragment);

        if constexpr (Index + 1 < sizeof...(Links))
        {
            texture<Index + 1>(out, pos, index + frag_type::output_nodes);
        }
    }

public:
    template<typename Tree>
    constexpr auto form_blob(const Tree& input) const noexcept
    {
        meta::drawable_strip<output_nodes> out{};
        vertex(out.mesh, 0, input_transform(input));
        return out;
    }

    constexpr auto tex_blob() const noexcept
    {
        output_array_t out{};
        texture(out, texture_geometry.start, 0);
        return out;
    }
};

template<typename... Links>
struct composition_mesh
{
    using tuple_type = std::tuple<Links...>;
    tuple_type chain;

    constexpr composition_mesh(const tuple_type& var) noexcept
        : chain{var}
    {}

    constexpr composition_mesh(tuple_type&& var) noexcept
        : chain{std::move(var)}
    {}

    using output_mesh = meta::drawable_strip_mesh<Links::output_nodes...>;
    using texture_mesh = typename output_mesh::texture_tuple_type;

protected:
    template<typename Tree, unsigned Index>
    constexpr void expand(output_mesh& out, const Tree& input) const noexcept
    {
        std::get<Index>(out.strip_tuple) = std::get<Index>(chain).form_blob(input);

        if constexpr (Index + 1 < sizeof...(Links))
        {
            expand<Tree, Index + 1>(out, input);
        }
    }

    template<unsigned Index>
    constexpr void expand(texture_mesh& out) const noexcept
    {
        std::get<Index>(out) = std::get<Index>(chain).tex_blob();

        if constexpr (Index + 1 < sizeof...(Links))
        {
            expand<Index + 1>(out);
        }
    }

public:
    template<typename Tree>
    constexpr auto compose(const Tree& input) const noexcept
    {
        output_mesh out{};
        expand<Tree, 0>(out, input);
        return out;
    }

    template<typename Tree>
    constexpr auto operator()(const Tree& input) const noexcept
    {
        output_mesh out{};
        expand<Tree, 0>(out, input);
        return out;
    }

    constexpr auto texture() const noexcept
    {
        texture_mesh out{};
        expand<0>(out);
        return out;
    }
};

}  // namespace poly

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
    float shift = 0.f;

    static constexpr unsigned output_nodes = 2;
    static constexpr unsigned input_nodes = 1;

    template<auto Size>
    constexpr void chew(std::array<point_t, Size>& out, const unsigned index, const meta::mesh_node& input) const noexcept
    {
        const auto shift_vec = input.pp * (width / 2);
        const auto shift_pos = input.pt - input.pl * shift;
        out[index] = shift_pos - shift_vec;
        out[index + 1] = shift_pos + shift_vec;
    }
};

struct equiv_rect
{
    point_t start, size;

    template<auto Size>
    constexpr point_t chew(std::array<point_t, Size>& out, point_t pos, const unsigned index, const sym&) const noexcept
    {
        pos.y += size.y;
        out[index] = pos;
        out[index + 1] = { pos.x + size.x, pos.y };
        return pos;
    }
};

}  // namespace skin

namespace paint
{

inline constexpr poly::composition_mesh human_mesh
{
    std::make_tuple(
        poly::blob_mesh
        {
            [] (const auto& tree)
            {
                using joint_type = typename idle_remove_cvr(tree)::joint_type;
                constexpr unsigned head_index = glass::label_index<glass::parts::head, joint_type>;
                const std::array<point_t, 3> neck
                {
                    glass::meta::flatten(tree.table[head_index]),
                    glass::meta::flatten(tree.table[head_index + 1]),
                    glass::meta::flatten(tree.table[head_index + 2])
                };
                return meta::smoothen_perpendicular_vectors(meta::extrapolate_vectors(neck));
            },

            skin::equiv_rect
            {
                point_t{ .25f, .25f },
                point_t{ .5f, .15f }
            },

            std::make_tuple(
                skin::sym{3.f, 0},
                skin::sym{9.f, 1},
                skin::sym{5.f, 2}
            )
        }
    )
};

}  // namespace paint

}  // namespace idle::glass

