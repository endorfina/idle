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

#include <cmath>
#include "octavia.hpp"

namespace idle::crimson::characters
{

namespace
{

template<auto Enum>
inline constexpr auto octa_ani = 0;

// template<>
// constexpr auto octa_ani<traits::humanoid::animation::stand> = glass::make<0, 45, 90, 135, 180, 225, 270, 315>(glass::paint::human_mesh, traits::humanoid::walking_muscle_digest);

template<>
inline constexpr auto octa_ani<traits::humanoid::animation::walk> = glass::make<0, 45, 90, 135, 180, 225, 270, 315>(glass::paint::human_mesh, traits::humanoid::walking_muscle.animate(traits::humanoid::default_model));

inline constexpr auto human_skin = glass::paint::human_mesh.texture();

void draw_octavia(const graphics::double_vertex_program_t& prog, const traits::humanoid::frame& fr) noexcept
{
    prog.set_interpolation(fr.timer);
    prog.set_texture_shift({ static_cast<uint8_t>(fr.dir) / 8.f, 0 });
    const bool data = true;

    const auto animate = [&](const auto& paint)
    {
        paint[fr.sub[0]].draw(
                prog,
                paint[fr.sub[1]],
                human_skin, data);
    };

    switch (fr.anim)
    {
#define idle_animation(x) \
        case traits::humanoid::animation::x: \
            animate(octa_ani<traits::humanoid::animation::x>[static_cast<uint8_t>(fr.dir)]); \
            break

        idle_animation(walk);

        default:
            break;
    }
}
#undef idle_animation

unsigned anim_length(const traits::humanoid::animation a) noexcept
{
    switch (a)
    {
#define idle_animation(x) \
        case traits::humanoid::animation::x: \
            return octa_ani<traits::humanoid::animation::x>.front().size()

        idle_animation(walk);

        default:
            return 1;
    }
}
#undef idle_animation

}  // namespace

octavia::octavia() noexcept
    : traits::humanoid
    {
        frame
        {
            .anim = traits::humanoid::animation::walk,
            .sub = { 0, 1 }
        }
    }
{
}

void octavia::draw(const graphics::core& gl) const noexcept
{
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, tex.id);
    gl.prog.double_normal.set_texture_mult(tex.area / 8.f);

    draw_octavia(gl.prog.double_normal, fr.load(std::memory_order_relaxed));
}

point_t octavia::apply_physics(const point_t pos) const noexcept
{
    return pos + speed * uni_time_factor;
}

void octavia::push_move(float direction, float value) noexcept
{
    const point_t shift { std::cos(direction) * value, std::sin(direction) * value };
    speed += (shift - speed) * .5f;
    friction_recalc = 0;
}

hotel::stage::action octavia::step() noexcept
{
    if (friction_recalc-- == 0)
    {
        friction_recalc = application_frames_per_second / 30;
        friction = speed / 50.f * uni_time_factor;
    }

    auto frame = fr.load(std::memory_order_relaxed);
    if (std::abs(speed.x) + std::abs(speed.y) > .1f)
    {
        speed -= friction;

        const point_t nv = speed * -1.f / std::hypot(speed.x, speed.y);
        constexpr point_t right{ 1, 0 };
        const float det = nv.determinant(right);
        const float prod = nv.product(right);
        const float direction = std::atan2(det, prod) + math::tau_2;
        frame.dir = static_cast<traits::eightway>(static_cast<uint8_t>(direction / math::tau_8) % 8);
        frame.timer += .03f * uni_time_factor;
        if (frame.timer >= 1.f)
        {
            frame.timer -= 1.f;
            frame.sub[0] = frame.sub[1];

            if (++frame.sub[1] >= anim_length(frame.anim))
            {
                frame.sub[1] = 0;
            }
        }
    }
    else
    {
        speed = {0, 0};
    }
    fr.store(frame, std::memory_order_relaxed);
    return hotel::stage::action::none;
}

}  // namespace idle:objects
