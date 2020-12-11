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
#include <atomic>
#include <idle/gl.hpp>
#include <idle/hotel/stage_include.hpp>
#include <idle/glass/glass.hpp>
#include <idle/png/image_queue.hpp>

namespace idle::crimson::characters::traits
{

enum eightway : uint8_t
{
    se, e, ne, n, nw, w, sw, s
};

struct humanoid
{
    enum class animation : uint8_t
    {
        stand,
        walk
    };

    static constexpr glass::closet::humanoid default_model{};

    struct frame
    {
        animation anim = animation::stand;
        std::array<uint8_t, 2> sub { 0, 1 };
        eightway dir = eightway::se;
        float timer = 0.f;
    };

    std::atomic<frame> fr;
    point_t speed, friction;
    uint8_t friction_recalc = 0;
    images::texture tex;

    humanoid() noexcept = default;

    humanoid(const frame& f) noexcept
        : fr{f}
    {}

    static constexpr auto walking_muscle = []()
    {
        constexpr float leg_raise = math::tau_8 / 4.f;
        constexpr float knee_bend = math::tau_8 * .9f;
        constexpr float foot_raise = math::tau_8 * -.9f;
        constexpr float hip_swing = math::tau_8 / 12.f;

        constexpr float shoulder_swing = math::tau_8 / 16.f;
        constexpr float arm_raise = math::tau_8 / 8.f;
        constexpr float elbow_bend = math::tau_8 / 4.f;
        constexpr float belly_bend = math::tau_8 / 16.f;
        constexpr float torso_bend = belly_bend / 3;
        constexpr float head_nod = math::tau_8 / 32.f;

        return glass::muscle
        {
            std::make_tuple(
                [](glass::closet::humanoid h)
                {
                    auto& head = h.get_ref<glass::parts::head>();
                    head[0].angle.z += head_nod;
                    head[1].angle.z -= head_nod;

                    auto& breast = h.get_ref<glass::parts::upperbody>().root;
                    breast.angle.z -= shoulder_swing;


                    auto& belly = h.get_ref<glass::parts::lowerbody>().root;
                    belly.angle.z += hip_swing;
                    belly.angle.y += belly_bend;

                    auto& shoulders = h.get_ref<glass::parts::shoulders>();
                    shoulders.right[1].angle.x += math::tau_8 * .61f;
                    shoulders.left[1].angle.x -= math::tau_8 * .61f;
                    shoulders.right[0].angle.y += arm_raise / 2;
                    shoulders.right[2].angle.y += elbow_bend;
                    shoulders.left[0].angle.y -= arm_raise / 2;
                    shoulders.left[2].angle.y += elbow_bend;

                    auto& hips = h.get_ref<glass::parts::hips>();
                    hips.right[0].angle.y -= leg_raise;
                    hips.right[2].angle.y += knee_bend;
                    hips.right[3].angle.y += leg_raise + foot_raise;
                    hips.left[0].angle.y += leg_raise;
                    hips.left[3].angle.y -= leg_raise;

                    h.realign();
                    return h;
                },

                [](glass::closet::humanoid h)
                {
                    auto& head = h.get_ref<glass::parts::head>();
                    head[0].angle.z -= head_nod;
                    head[1].angle.z += head_nod;

                    auto& breast = h.get_ref<glass::parts::upperbody>().root;
                    breast.angle.z -= shoulder_swing;
                    breast.angle.y += torso_bend;

                    auto& belly = h.get_ref<glass::parts::lowerbody>().root;
                    belly.angle.z += hip_swing;
                    belly.angle.y -= belly_bend;

                    auto& shoulders = h.get_ref<glass::parts::shoulders>();
                    shoulders.right[0].angle.y += arm_raise;
                    shoulders.left[0].angle.y -= arm_raise;
                    shoulders.left[2].angle.y += elbow_bend;

                    auto& hips = h.get_ref<glass::parts::hips>();

                    hips.right[0].angle.y -= leg_raise;
                    hips.right[2].angle.y -= knee_bend;
                    hips.right[3].angle.y += leg_raise - foot_raise;
                    hips.left[0].angle.y += leg_raise;
                    hips.left[3].angle.y -= leg_raise;

                    h.realign();
                    return h;
                },

                [](glass::closet::humanoid h)
                {
                    auto& head = h.get_ref<glass::parts::head>();
                    head[0].angle.z += head_nod;
                    head[1].angle.z -= head_nod;

                    auto& breast = h.get_ref<glass::parts::upperbody>().root;
                    breast.angle.z += shoulder_swing * 3;
                    breast.angle.y -= torso_bend;

                    auto& belly = h.get_ref<glass::parts::lowerbody>().root;
                    belly.angle.z -= hip_swing * 3;
                    belly.angle.y += belly_bend;


                    auto& shoulders = h.get_ref<glass::parts::shoulders>();
                    shoulders.right[0].angle.y -= arm_raise * 3;
                    shoulders.left[0].angle.y += arm_raise * 3;
                    shoulders.left[2].angle.y -= elbow_bend;

                    auto& hips = h.get_ref<glass::parts::hips>();

                    hips.right[0].angle.y += leg_raise * 3;
                    hips.right[3].angle.y -= leg_raise * 3;
                    hips.left[0].angle.y -= leg_raise * 3;
                    hips.left[2].angle.y += knee_bend;
                    hips.left[3].angle.y += leg_raise * 3 + foot_raise;

                    h.realign();
                    return h;
                },

                [](glass::closet::humanoid h)
                {
                    auto& head = h.get_ref<glass::parts::head>();
                    head[0].angle.z -= head_nod;
                    head[1].angle.z += head_nod;

                    auto& breast = h.get_ref<glass::parts::upperbody>().root;
                    breast.angle.z += shoulder_swing;
                    breast.angle.y += torso_bend;

                    auto& belly = h.get_ref<glass::parts::lowerbody>().root;
                    belly.angle.z -= hip_swing;
                    belly.angle.y -= belly_bend;

                    auto& shoulders = h.get_ref<glass::parts::shoulders>();
                    shoulders.right[0].angle.y -= arm_raise;
                    shoulders.left[0].angle.y += arm_raise;
                    shoulders.right[2].angle.y += elbow_bend;

                    auto& hips = h.get_ref<glass::parts::hips>();

                    hips.left[0].angle.y -= leg_raise;
                    hips.left[2].angle.y -= knee_bend;
                    hips.left[3].angle.y += leg_raise - foot_raise;
                    hips.right[0].angle.y += leg_raise;
                    hips.right[3].angle.y -= leg_raise;

                    h.realign();
                    return h;
                }
            )
        };
    }();

};


}  // namespace idle::crimson::characters::traits
