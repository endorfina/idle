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

#include <idle/drawable.hpp>
#include "room_model.hpp"

#ifdef IDLE_COMPILE_GALLERY

#include <idle/glass.hpp>

namespace idle::hotel::model
{

namespace
{

constexpr point_t flatten(const point_3d_t p)
{
    return { p.y, - p.z };
}

constexpr auto make_matrix(const point_3d_t angle)
{
    return math::matrices::rotate_x<float>(angle.x)
        * math::matrices::rotate_y<float>(angle.y);
}

template<std::size_t I = 0, typename Callable, typename... Tp>
constexpr void elem_visit(const Callable& call, const std::tuple<Tp...>& tuple)
{
    call(std::get<I>(tuple));

    if constexpr (I + 1 < sizeof...(Tp))
    {
        elem_visit<I + 1, Callable, Tp...>(call, tuple);
    }
}

using index_pair = std::array<unsigned, 2>;

template<typename J>
struct tree_skeleton
{
    std::array<point_t, J::size + J::oddness> table{};
    std::array<unsigned, 1 + J::oddness> lengths{};

private:
    constexpr index_pair to_lines(index_pair index, const glass::blocks::bone& b, const mat4x4_noopt_t& mat)
    {
        const auto rot = math::matrices::translate<float>({0, 0, b.length}) * make_matrix(b.angle) * mat;
        table[index[0]++] = flatten(rot * point_3d_t{});
        return index;
    }

    template<typename...Vars>
    constexpr index_pair to_lines(const index_pair index, const glass::blocks::joint<Vars...>& j, const mat4x4_noopt_t& mat)
    {
        const auto rot = math::matrices::translate<float>({0, 0, j.root.length}) * make_matrix(j.root.angle) * mat;
        auto new_index = index;
        table[new_index[0]++] = flatten(rot * point_3d_t{});
        new_index = to_lines(new_index, j.template link<0>(), rot);

        if constexpr (sizeof...(Vars) > 1)
        {
            elem_visit<1>([&](const auto& it)
                {
                    lengths[new_index[1]++] = new_index[0];
                    table[new_index[0]++] = table[index[0]];
                    new_index = to_lines(new_index, it, rot);
                },
                j.links);
        }
        return new_index;
    }

    template<typename Val>
    constexpr index_pair to_lines(const index_pair index, const glass::blocks::symmetry<Val>& s, const mat4x4_noopt_t& mat)
    {
        auto new_index = to_lines(index, s.left, mat);
        lengths[new_index[1]++] = new_index[0];
        table[new_index[0]++] = table[index[0] - 1];
        return to_lines(new_index, s.right, mat);
    }

public:
    void draw(const graphics::core& gl) const
    {
        auto ptr = table.data();

        for (const auto it : lengths)
        {
            gl.prog.fill.position_vertex(reinterpret_cast<const GLfloat*>(ptr));
            gl::DrawArrays(gl::LINE_STRIP, 0, it);
            ptr += it;
        }
    }

    constexpr tree_skeleton(const J& root)
    {
        const auto [total_length, last_iter] = to_lines({0, 0}, root, math::matrices::rotate<GLfloat>(F_TAU_8) * math::matrices::rotate_y<GLfloat>(F_TAU_4 / 3));
        lengths[last_iter] = total_length;

        for (auto i = lengths.size() - 1; i > 0; --i)
        {
            lengths[i] -= lengths[i - 1];
        }
    }
};

void draw_bones(const graphics::core& gl)
{
    constexpr auto hu = glass::closet::humanoid::get_default();
    constexpr tree_skeleton model(hu);

    gl.prog.fill.use();
    gl.prog.fill.set_view_transform(math::matrices::translate(gl.draw_size / 2.f));

    model.draw(gl);
}

}  // namespace

void room::draw(const graphics::core& gl) const
{
    gl.prog.fill.use();
    gl.prog.fill.set_identity();
    gl.prog.fill.set_view_identity();
    gl.prog.fill.set_color({0,0,0});
    fill_screen(gl, gl.prog.fill);

    constexpr auto greyscale = color_t::greyscale(.29f);

    gl.prog.text.use();
    gl.prog.text.set_color(greyscale);
    draw_text<text_align::center, text_align::center>(*gl.fonts.title, gl.prog.text, "Model", gl.draw_size / 2.f, 48);

    gl.prog.fill.use();
    gl.prog.fill.set_color({1,0,0});
    draw_bones(gl);

}

}  // namespace idle::hotel::model

#endif  // IDLE_COMPILE_GALLERY

