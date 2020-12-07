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

#include <idle/glass/glass.hpp>
#include "stage_objects.hpp"

namespace idle::hotel::stage
{

namespace
{

idle_check_method_boilerplate(check_drawable);
idle_check_method_boilerplate(step);
idle_check_method_boilerplate(draw);
idle_check_method_boilerplate(push_move);
idle_check_method_boilerplate(apply_physics);

}  // namespace


action object::step() noexcept
{
    const action ret = std::visit([&pos = this->pos](auto& obj) -> action
        {
            if constexpr(idle_has_method(idle_remove_cvr(obj), apply_physics))
            {
                pos = obj.apply_physics(pos);
            }

            if constexpr(idle_has_method(idle_remove_cvr(obj), step))
            {
                return obj.step();
            }
            else
            {
                return action::none;
            }
        },
        variant);

    if (ret == action::none && !shown)
    {
        if (std::visit([](const auto& obj) -> bool
            {
                if constexpr(idle_has_method(idle_remove_cvr(obj), check_drawable))
                {
                    return obj.check_drawable();
                }
                else
                {
                    return false;
                }
            },
            variant))
        {
            shown = true;
            return action::register_drawable;
        }
    }
    return ret;
}

object::object(const point_t position) noexcept
    : pos{ position }
{}

void object::move(float direction, float value) noexcept
{
    std::visit([direction, value](auto& obj)
    {
        if constexpr(idle_has_method(idle_remove_cvr(obj), push_move))
        {
            return obj.push_move(direction, value);
        }
    },
    variant);
}

void object::draw(const graphics::core& gl) const noexcept
{
    std::visit([&gl](const auto& obj)
    {
        if constexpr(idle_has_method(idle_remove_cvr(obj), draw))
        {
            return obj.draw(gl);
        }
    },
    variant);
}

}  // namespace idle::hotel::stage

