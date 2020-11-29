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
constexpr float average_depth(const std::array<point_3d_t, Size>& input) noexcept
{
    float depth = 0;
    for (const auto& it : input)
    {
        depth += it.x;
    }
    return depth / static_cast<float>(Size);
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

    static constexpr bool extra_arguments = false;

    template<typename Program>
    void draw_elem(const Program& prog, const drawable_strip& another, const vertex_table_type& texture) const noexcept
    {
        prog.position_vertex(reinterpret_cast<const GLfloat*>(mesh.data()));
        prog.destination_vertex(reinterpret_cast<const GLfloat*>(another.mesh.data()));
        prog.texture_vertex(reinterpret_cast<const GLfloat*>(texture.data()));
        gl::DrawArrays(gl::TRIANGLE_STRIP, 0, Size);
    }
};

template<typename...Links>
struct drawable_strip_mesh
{
    using vertex_tuple_type = std::tuple<Links...>;
    using texture_tuple_type = std::tuple<typename Links::vertex_table_type...>;
    vertex_tuple_type strip_tuple;

#if !__cpp_lib_constexpr_tuple
    constexpr drawable_strip_mesh& operator=(const drawable_strip_mesh& other) noexcept
    {
        utility::tuple_copy(strip_tuple, other.strip_tuple);
        return *this;
    }
#endif

    template<unsigned Index, typename Program, typename...Extra>
    void draw(const Program& program,
            const drawable_strip_mesh& another,
            const texture_tuple_type& texture_vertices,
            const Extra&...extra) const noexcept
    {
        static_assert(Index < sizeof...(Links));
        using frag_type = typename std::tuple_element<Index, vertex_tuple_type>::type;
        if constexpr(frag_type::extra_arguments)
        {
            std::get<Index>(strip_tuple).draw_elem(program,
                                std::get<Index>(another.strip_tuple),
                                std::get<Index>(texture_vertices),
                                extra...);
        }
        else
        {
            std::get<Index>(strip_tuple).draw_elem(program,
                                std::get<Index>(another.strip_tuple),
                                std::get<Index>(texture_vertices));
        }
    }

    template<unsigned Index = 0, typename Program, typename...Extra>
    void draw_all(const Program& program,
            const drawable_strip_mesh& another,
            const texture_tuple_type& texture_vertices,
            const Extra&...extra) const noexcept
    {
        draw<Index>(program, another, texture_vertices, extra...);

        if constexpr (Index + 1 < sizeof...(Links))
        {
            draw_all<Index + 1, Program>(program, another, texture_vertices, extra...);
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
        out[i].pp = (input[i - 1].pp + input[i].pp /*+ input[i + 1].pp*/) / 2.f;
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

template<auto Size>
constexpr auto curved_vectors(const std::array<point_3d_t, Size>& input) noexcept
{
    static_assert(Size > 2);
    const auto vec1 = extrapolate_vectors(flatten(input));
    std::array<point_t, Size * 2 - 1> vec2;
    for (unsigned i = 0; i < Size; ++i)
    {
        vec2[i * 2] = vec1[i].pt;
    }
    for (unsigned i = 0; i < Size - 1; ++i)
    {
        vec2[i * 2 + 1] = (vec1[i].pt + vec1[i + 1].pt) / 2.f;
    }
    for (unsigned i = 1; i < Size - 1; ++i)
    {
        const auto a = vec1[i - 1];
        const auto b = vec1[i];
        const auto prod = a.pl.product(b.pl);
        const auto det = a.pl.determinant(b.pl);
        const auto angle = math::const_math::atan2(det, prod);
        const auto val = angle / math::tau / 2;
        vec2[i * 2 - 1] += a.pp * (a.pt.distance(b.pt)) * val;
        vec2[i * 2 + 1] += b.pp * (b.pt.distance(vec1[i + 1].pt)) * val;
    }
    return smoothen_perpendicular_vectors(extrapolate_vectors(vec2));
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

#if !__cpp_lib_constexpr_tuple
    constexpr blob_mesh& operator=(const blob_mesh& other) noexcept
    {
        input_select = other.input_select;
        input_transform = other.input_transform;
        texture_geometry = other.texture_geometry;
        utility::tuple_copy(chain, other.chain);
        return *this;
    }
#endif


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

        pos = texture_geometry.template chew<sizeof...(Links)>(out, pos, index, fragment);

        if constexpr (Index + 1 < sizeof...(Links))
        {
            texture<Index + 1>(out, pos, index + frag_type::output_nodes);
        }
    }

public:
    using output_vertex_type = meta::drawable_strip<output_nodes>;
    using returned_pair = std::pair<output_vertex_type, float>;

    template<typename Tree>
    constexpr returned_pair form_blob(const Tree& input) const noexcept
    {
        const auto select = input_select(input);
        const auto z = meta::average_depth(select);

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

#if !__cpp_lib_constexpr_tuple
    constexpr composition_mesh& operator=(const composition_mesh& other) noexcept
    {
        factory = other.factory;
        utility::tuple_copy(chain, other.chain);
        return *this;
    }
#endif

    using z_array_t = std::array<float, sizeof...(Links)>;
    using output_mesh = meta::drawable_strip_mesh<typename Links::output_vertex_type...>;
    using texture_mesh = typename output_mesh::texture_tuple_type;

protected:
    template<typename Tree, unsigned Index>
    constexpr void expand(output_mesh& mesh_out, z_array_t& z_out, const Tree& input) const noexcept
    {
#if __cpp_lib_constexpr_utility
        std::tie(std::get<Index>(mesh_out.strip_tuple), z_out[Index])
            = std::get<Index>(chain).form_blob(input);
#else
        const auto [tuple, z] = std::get<Index>(chain).form_blob(input);
        std::get<Index>(mesh_out.strip_tuple) = tuple;
        z_out[Index] = z;
#endif

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

    template<unsigned Parts, auto Size>
    constexpr point_t chew(std::array<point_t, Size>& out, point_t pos, const unsigned index, const sym&) const noexcept
    {
        out[index] = pos;
        out[index + 1] = { pos.x + size.x, pos.y };
        pos.y += size.y / (Parts - 1);
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

    template<typename Tree>
    constexpr output_type operator()(const Tree& tree) const noexcept
    {
        using joint_type = typename Tree::joint_type;
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

template<auto Key, unsigned Size, bool Right = false>
struct split
{
    unsigned offset = 0;

    static constexpr unsigned size = Size;

    using output_type = std::array<point_3d_t, Size>;

    template<typename Tree>
    constexpr output_type operator()(const Tree& tree) const noexcept
    {
        using joint_type = typename Tree::joint_type;
        constexpr auto split_index = label_index<Key, joint_type>;
        constexpr unsigned key_offset = Right ? split_index.right : split_index.left;
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

#if !__cpp_lib_constexpr_tuple
    constexpr join& operator=(const join& other) noexcept
    {
        utility::tuple_copy(chain, other.chain);
        return *this;
    }
#endif

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

namespace meta
{

template<unsigned Start, unsigned...Seq>
constexpr auto uniform_sym_strip(const float width, const float begin, const float end, const std::integer_sequence<unsigned, Seq...>) noexcept
{
    return std::make_tuple(
            skin::sym{ width, Start, begin },
            skin::sym{ width, Start + 1 + Seq }...,
            skin::sym{ width, Start + 1 + sizeof...(Seq), end }
        );
}

}  // namespace meta

namespace extra
{

inline constexpr auto flat = [] (const auto& input)
{
    return meta::extrapolate_vectors(meta::flatten(input));
};

inline constexpr auto smooth = [] (const auto& input)
{
    return meta::smooth_vectors(input);
};

inline constexpr auto curved = [] (const auto& input)
{
    return meta::curved_vectors(input);
};

template<unsigned Start = 0, unsigned Len = 2>
constexpr auto uniform_sym_strip(const float width, const float begin, const float end) noexcept
{
    if constexpr(Len > 2)
    {
        return meta::uniform_sym_strip<Start>(
                width, begin, end, std::make_integer_sequence<unsigned, Len - 2>{});
    }
    else
    {
        static_assert(Len == 2);
        return std::make_tuple(
                skin::sym{ width, Start, begin },
                skin::sym{ width, Start + 1, end }
            );
    }
}

}  // namespace extra

namespace meta
{

template<typename...Links>
struct default_drawable
{
    using drawable_t = meta::drawable_strip_mesh<Links...>;
    drawable_t drawable;

    constexpr default_drawable(const drawable_t& m) noexcept
        : drawable{m}
    {}

    template<typename Program, typename...Extra>
    void draw(const Program& program,
            const default_drawable& another,
            const typename drawable_t::texture_tuple_type& texture_vertices,
            const Extra&...extra) const noexcept
    {
        drawable.draw_all(program, another.drawable, texture_vertices, extra...);
    }
};

template<std::size_t Size>
struct drawing_precedence
{
    static_assert(Size > 1 && Size <= 16);

private:
    std::array<unsigned char, Size> table{};

public:
    constexpr drawing_precedence(const std::array<float, Size>& input) noexcept
    {
        std::array<std::pair<unsigned char, float>, Size> work;
        for (unsigned char i = 0; i < Size; ++i)
        {
#if __cpp_lib_constexpr_utility
            work[i] = { i, input[i] };
#else
            work[i].first = i;
            work[i].second = input[i];
#endif
        }

#if !__cpp_lib_constexpr_algorithms
        for (unsigned n = 0; n < Size - 1; ++n)
        {
            for (unsigned char i = 0; i < Size - 1; ++i)
            {
                const auto temp = work[i];
                if (temp.second < work[i + 1].second)
                {
#if __cpp_lib_constexpr_utility
                    work[i] = work[i + 1];
                    work[i + 1] = temp;
#else
                    utility::pair_copy(work[i], work[i + 1]);
                    utility::pair_copy(work[i + 1], temp);
#endif
                }
            }
        }
#else
        std::sort(work.begin(), work.end(), [](const auto lhs, const auto rhs)
            {
                return lhs.second < rhs.second;
            });
#endif

        for (unsigned char i = 0; i < Size; ++i)
        {
            table[i] = work[i].first;
        }
    }

    template<typename Program, typename Drawable, typename...Extra>
    void draw(const Program& program,
            const Drawable& drawable,
            const Drawable& another,
            const typename Drawable::texture_tuple_type& texture_vertices,
            const Extra&...extra) const noexcept
    {
        for (const auto index : table)
            switch(index)
            {
#define IDLE_DRAWABLE(num) \
            case num: \
                if constexpr ((num) < Size) { \
                    drawable.template draw<num>(program, another, texture_vertices, extra...); \
                } \
                break;

            IDLE_DRAWABLE(0)
            IDLE_DRAWABLE(1)
            IDLE_DRAWABLE(2)
            IDLE_DRAWABLE(3)
            IDLE_DRAWABLE(4)
            IDLE_DRAWABLE(5)
            IDLE_DRAWABLE(6)
            IDLE_DRAWABLE(7)
            IDLE_DRAWABLE(8)
            IDLE_DRAWABLE(9)
            IDLE_DRAWABLE(10)
            IDLE_DRAWABLE(11)
            IDLE_DRAWABLE(12)
            IDLE_DRAWABLE(13)
            IDLE_DRAWABLE(14)
            IDLE_DRAWABLE(15)

#undef IDLE_DRAWABLE
            }
    }
};

template<typename...Links>
struct default_depth_check
{
    using drawable_t = meta::drawable_strip_mesh<Links...>;
    drawable_t drawable;
    drawing_precedence<sizeof...(Links)> precedence;

    constexpr default_depth_check(const drawable_t& m, const std::array<float, sizeof...(Links)>& zs) noexcept
        : drawable{m}, precedence{zs}
    {}

    template<typename Program, typename...Extra>
    void draw(const Program& program,
            const default_depth_check& another,
            const typename drawable_t::texture_tuple_type& texture_vertices,
            const Extra&...extra) const noexcept
    {
        precedence.template draw<Program, drawable_t, Extra...>(
                program, drawable, another.drawable, texture_vertices, extra...);
    }
};

}  // namespace meta

namespace drawing
{

inline constexpr auto default_procedure = [] (const auto& drawable, const auto& average_depth_array)
{
    return meta::default_drawable{ drawable };
};

inline constexpr auto humanoid = [] (const auto& drawable, const auto& average_depth_array)
{
    return meta::default_depth_check{ drawable, average_depth_array };
};

}  // namespace drawing

}  // namespace idle::glass

