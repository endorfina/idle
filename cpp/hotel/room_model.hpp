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

#include <idle/gl.hpp>

#ifdef IDLE_COMPILE_GALLERY
#include <idle/pointer_wrapper.hpp>
#include <idle/draw_text.hpp>
#include "gui.hpp"
#include "keys.hpp"

namespace idle::hotel::model
{

struct animation
{
    unsigned source, dest;
    float interpolation;
};

struct room
{
    float timer = 0.f;
    unsigned char facing = 0;

    std::atomic<animation> model_anim = animation{0, 1, 0.f};

    std::optional<keyring::variant> step(const pointer_wrapper& cursor) noexcept;

    void draw(const graphics::core& gl) const noexcept;
};

}  // namespace idle::hotel::model

#endif  // IDLE_COMPILE_GALLERY

