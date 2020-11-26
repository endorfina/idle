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
constexpr float average_z(const std::array<point_3d_t, Size>& input) noexcept
{
    float z = 0;
    for (const auto& it : input)
    {
        z += it.z;
    }
    return z / static_cast<float>(Size);
}

template<auto Size>
constexpr auto flatten(const std::array<point_3d_t, Size>& input) noexcept
{
    std::array<point_t, Size> out{};
    for (unsigned i = 0; i < Size; ++i)
    {
        out[i] = meta::flatten(input[i]);
    }
    return out;
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

    template<unsigned Index, typename Program>
    void draw(const Program& program,
            const texture_tuple_type& texture_vertices,
            const drawable_strip_mesh& another) const noexcept
    {
        std::get<Index>(strip_tuple).draw_elem(program,
                            std::get<Index>(texture_vertices),
                            std::get<Index>(another.strip_tuple));
    }

    template<unsigned Index = 0, typename Program>
    void draw_all(const Program& program,
            const texture_tuple_type& texture_vertices,
            const drawable_strip_mesh& another) const noexcept
    {
        draw<Index>(program, texture_vertices, another);

        if constexpr (Index + 1 < sizeof...(Sizes))
        {
            draw_all<Index + 1, Program>(program, texture_vertices, another);
        }
    }
};

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

template<auto Size>
constexpr auto smooth_vectors(const std::array<point_3d_t, Size>& input) noexcept
{
    if constexpr(Size > 2)
    {
        return smoothen_perpendicular_vectors(
                extrapolate_vectors(
                    flatten(input)));
    }
    else
    {
        return extrapolate_vectors(flatten(input));
    }
}

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

template<typename Selector, typename Transform, typename Skin, typename... Links>
struct blob_mesh
{
    using tuple_type = std::tuple<Links...>;

    Selector input_select;
    Transform input_transform;
    Skin texture_geometry;
    tuple_type chain;

    constexpr blob_mesh(
            const Selector& select,
            const Transform& transform,
            const Skin& skin,
            const tuple_type& var) noexcept
    :
        input_select{select},
        input_transform{transform},
        texture_geometry{skin},
        chain{var}
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
    using returned_pair = std::pair<meta::drawable_strip<output_nodes>, float>;

    template<typename Tree>
    constexpr returned_pair form_blob(const Tree& input) const noexcept
    {
        const auto select = input_select(input);
        const auto z = meta::average_z(select);

        meta::drawable_strip<output_nodes> out{};
        vertex(out.mesh, 0, input_transform(select));
        return { out, z };
    }

    constexpr auto tex_blob() const noexcept
    {
        output_array_t out{};
        texture(out, texture_geometry.start, 0);
        return out;
    }
};

template<typename PainterFactory, typename... Links>
struct composition_mesh
{
    using tuple_type = std::tuple<Links...>;
    tuple_type chain;
    PainterFactory factory;

    constexpr composition_mesh(const tuple_type& blobs, const PainterFactory& pain) noexcept
        : chain{blobs}, factory{pain}
    {}

    using z_array_t = std::array<float, sizeof...(Links)>;
    using output_mesh = meta::drawable_strip_mesh<Links::output_nodes...>;
    using texture_mesh = typename output_mesh::texture_tuple_type;

protected:
    template<typename Tree, unsigned Index>
    constexpr void expand(output_mesh& mesh_out, z_array_t& z_out, const Tree& input) const noexcept
    {
        std::tie(std::get<Index>(mesh_out.strip_tuple), z_out[Index])
            = std::get<Index>(chain).form_blob(input);

        if constexpr (Index + 1 < sizeof...(Links))
        {
            expand<Tree, Index + 1>(mesh_out, z_out, input);
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
    constexpr auto operator()(const Tree& input) const noexcept
    {
        output_mesh out{};
        z_array_t zs{};
        expand<Tree, 0>(out, zs, input);
        return factory(out, zs);
    }

    constexpr auto texture() const noexcept
    {
        texture_mesh out{};
        expand<0>(out);
        return out;
    }
};

}  // namespace poly

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

namespace selector
{

template<auto Key, unsigned Size>
struct segment
{
    unsigned offset = 0;

    static constexpr unsigned size = Size;

    using output_type = std::array<point_3d_t, Size>;

    constexpr output_type operator()(const auto& tree) const noexcept
    {
        using joint_type = typename idle_remove_cvr(tree)::joint_type;
        constexpr unsigned key_offset = label_index<Key, joint_type>;
        unsigned i = key_offset + offset;
        output_type out{};

        for (auto& it : out)
        {
            it = tree.table[i++];
        }
        return out;
    }
};

template<typename...Links>
struct join
{
    static constexpr unsigned size = (0 + ... + Links::size);

    using tuple_type = std::tuple<Links...>;
    using output_type = std::array<point_3d_t, size>;

    tuple_type chain;

    constexpr join(const tuple_type& tuple) noexcept
        : chain{tuple}
    {}

private:
    template<unsigned Index = 0, typename Tree>
    constexpr void use_link(output_type& out, unsigned index, const Tree& tree) const noexcept
    {
        using frag_type = typename std::tuple_element<Index, tuple_type>::type;
        const frag_type& fragment = std::get<Index>(chain);

        const auto array = fragment(tree);

        for (const auto& it : array)
        {
            out[index++] = it;
        }

        if constexpr(Index + 1 < sizeof...(Links))
        {
            use_link<Index + 1, Tree>(out, index, tree);
        }
    }

public:
    template<typename Tree>
    constexpr output_type operator()(const Tree& tree) const noexcept
    {
        output_type out{};
        use_link<0, Tree>(out, 0, tree);
        return out;
    }
};

}  // namespace selector

namespace extra
{

inline constexpr auto smooth = [] (const auto& input)
{
    return meta::smooth_vectors(input);
};

template<auto...Sizes>
struct default_drawable
{
    using drawable_t = meta::drawable_strip_mesh<Sizes...>;
    drawable_t drawable;

    constexpr default_drawable(const drawable_t& m) noexcept
        : drawable{m}
    {}

    template<typename Program>
    void draw(const Program& program,
            const typename drawable_t::texture_tuple_type& texture_vertices,
            const default_drawable& another) const noexcept
    {
        drawable.draw_all(program, texture_vertices, another.drawable);
    }
};

}  // namespace extra

}  // namespace idle::glass

