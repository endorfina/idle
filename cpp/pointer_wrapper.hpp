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
#include <math_defines.hpp>
#include "platform/pointer.hpp"

namespace idle
{

struct pointer_wrapper
{
    platform::pointer cursor;
    bool single_press = false, double_tap = false;
    float touch = 0.f;
};

class pointer_keeper
{
    pointer_wrapper pointer;
    int single_press_previous_state = 0, double_tap_timer = 0;

public:
    void advance(const platform::pointer& cur) noexcept;

    auto get() const noexcept -> const pointer_wrapper&;
};

}  // namepsace idle
