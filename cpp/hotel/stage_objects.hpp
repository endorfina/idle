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

#include "stage_include.hpp"
#include "../game/objects.hpp"

namespace idle::hotel::stage
{

struct object
{
    point_t pos;
    bool shown = false;
    objects::variant variant;

    object(point_t p) noexcept;

    action step() noexcept;

    void draw(const graphics::core& gl) const noexcept;

    void move(float direction, float value) noexcept;
};


struct player_object
{
    object* captive_mind = nullptr;

    struct camera_type
    {
        point_t translate {0, 0};
        float scale = 1.f;
    };
    camera_type camera;
    point_t cursor_pos;

    point_t hud_size;
};

}  // namespace idle::hotel::stage

