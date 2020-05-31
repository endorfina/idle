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

#include "pointer.hpp"

namespace idle
{

void pointer_keeper::advance(const platform::pointer& cur)
{
    pointer.cursor = cur;

    if (pointer.cursor.pressed)
    {
        pointer.touch = std::min(pointer.touch + .05f, 1.f);
    }
    else
    {
        pointer.touch = std::max(pointer.touch - .1f, 0.f);
    }

    pointer.single_press = false;
    pointer.double_tap = false;

    if (single_press_previous_state < 1 && pointer.cursor.pressed)
    {
        pointer.single_press = true;
        single_press_previous_state = 2;

        if (double_tap_timer < 1)
            double_tap_timer = 15;
        else
            pointer.double_tap = true;
    }

    if (single_press_previous_state > 0 && !pointer.cursor.pressed)
        --single_press_previous_state;

    if (double_tap_timer > 0)
        --double_tap_timer;
}

const pointer_wrapper& pointer_keeper::get() const
{
    return pointer;
}

}  // namespace idle
