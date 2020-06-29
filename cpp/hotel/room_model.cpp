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
        * math::matrices::rotate_y<float>(angle.y)
        * math::matrices::rotate<float>(angle.z);
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
            glass::tuple_visit<1>([&](const auto& it)
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

    void draw_interpolated(const graphics::core& gl, const tree_skeleton& another) const
    {
        size_t sum = 0;

        for (const auto it : lengths)
        {
            gl.prog.double_fill.position_vertex(reinterpret_cast<const GLfloat*>(&table[sum]));
            gl.prog.double_fill.destination_vertex(reinterpret_cast<const GLfloat*>(&another.table[sum]));
            gl::DrawArrays(gl::LINE_STRIP, 0, it);
            sum += it;
        }
    }

    constexpr tree_skeleton(const J& root, const mat4x4_noopt_t& mat)
    {
        const auto [total_length, last_iter] = to_lines({0, 0}, root, mat);
        lengths[last_iter] = total_length;

        for (auto i = lengths.size() - 1; i > 0; --i)
        {
            lengths[i] -= lengths[i - 1];
        }
    }
};

constexpr mat4x4_noopt_t skew_mat = math::matrices::rotate<GLfloat>(F_TAU_8) * math::matrices::rotate_y<GLfloat>(F_TAU_4 / 3);

template<unsigned...Degs, typename J>
constexpr std::array<tree_skeleton<J>, sizeof...(Degs)> rotate(const J& root)
{
    return { tree_skeleton<J>{ root, math::matrices::rotate<float>(math::degtorad<float>(Degs)) * skew_mat } ... };
}

constexpr auto def_models_rotated = rotate<0, 45, 90, 135, 180, 225, 270, 315>(glass::closet::humanoid{});

template<typename Models>
void draw_bones(const Models& models, const graphics::core& gl, const animation anim)
{
    gl.prog.fill.use();
    gl.prog.fill.set_color({1,0,0});

    const auto model_offset = gl.draw_size.x / (models.size() + 1);
    gl.prog.fill.set_view_transform(math::matrices::translate<float>({model_offset, gl.draw_size.y * .8f}));

    unsigned i = 0;
    for (const auto& it : models)
    {
        gl.prog.fill.set_transform(math::matrices::translate<float>({model_offset * i++, 0}));
        it.draw(gl);
    }

    gl.prog.double_fill.use();
    gl.prog.double_fill.set_color({1,1,1,.9f});
    const auto view = math::matrices::uniform_scale(2.f) * math::matrices::translate(gl.draw_size / 2.f);
    gl.prog.double_fill.set_view_transform(view);
    gl.prog.double_fill.set_interpolation(anim.interpolation);

    models[anim.source % models.size()].draw_interpolated(gl, models[anim.dest % models.size()]);
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

    draw_bones(def_models_rotated, gl, model_anim.load(std::memory_order_relaxed));

}

std::optional<keyring::variant> room::step(const pointer_wrapper& [[maybe_unused]] cursor)
{
    auto work_copy = model_anim.load(std::memory_order_relaxed);
    timer += .04f;

    if (timer >= F_TAU)
    {
        timer = F_TAU_2;

        work_copy.source = work_copy.dest;

        if (++work_copy.dest >= def_models_rotated.size())
        {
            work_copy.dest = 0;
        }
    }

    work_copy.interpolation = (std::cos(timer) + 1.f) / 2.f;
    model_anim.store(work_copy, std::memory_order_relaxed);
    return {};
}

}  // namespace idle::hotel::model

#endif  // IDLE_COMPILE_GALLERY

