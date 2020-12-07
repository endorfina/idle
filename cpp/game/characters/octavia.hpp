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

#include <idle/hotel/stage_include.hpp>
#include "humanoid.hpp"

namespace idle::crimson::characters
{

struct octavia : traits::humanoid
{
    constexpr bool check_drawable() const noexcept
    {
        return true;
    }

    octavia() noexcept;

    void push_move(float direction, float value) noexcept;

    point_t apply_physics(point_t pos) const noexcept;

    hotel::stage::action step() noexcept;

    void draw(const graphics::core& gl) const noexcept;
};


}  // namespace idle:objects
