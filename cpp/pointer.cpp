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

void pointer::clear(platform::pointer& cursor)
{
    cursor.single_press = false;
}

void pointer::update(platform::pointer& cursor)
{
    if (cursor.pressed && cursor.touch < 0.99f)
        cursor.touch += .05f;
    else if (!cursor.pressed && cursor.touch > 0.01f) {
        cursor.touch -= .1f;
        if (cursor.touch < .049f)
            cursor.touch = 0.f;
    }

    cursor.double_tap = false;

    if ((single_press_previous_state < 1) && cursor.pressed)
    {
        cursor.single_press = true;
        single_press_previous_state = 2;
        if (double_tap_timer < 1) {
            double_tap_timer = 15;
        }
        else cursor.double_tap = true;
    }
    else cursor.single_press = false;

    if (single_press_previous_state > 0 && !cursor.pressed)
        --single_press_previous_state;

    if (double_tap_timer > 0)
        --double_tap_timer;
}

}  // namespace idle
