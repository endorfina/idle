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
#include "gl.hpp"
#include "objects.hpp"

namespace idle
{
struct landing_room
{
    float alpha = 0, rotation = 0;
    unsigned counter = 1;

    std::array<float, 128> ray_array, ray_array_mirror;

    landing_room(graphics::core&);

    bool step(graphics::core&);

    void draw(const graphics::core&) const;
};

}
