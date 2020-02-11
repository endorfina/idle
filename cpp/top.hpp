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

#include <atomic>
#include <math.hpp>
#include "platform/pointer.hpp"
#include "gl.hpp"

namespace idle
{
class image_t;

enum class room_id_enum : uint_fast8_t {
#ifndef NDEBUG
    room_models,
#endif
    room_uninitialized,
    room_landing,
    room_red
};

}

#include "objects.hpp"
#include "database.hpp"

class overlay
{
public:
    graphics::core gl;
    idle::Database db;

    void need_a_light(const idle::object *attach, float x, float y, float r, unsigned char pow);

private:
    bool shutdown_was_requested_ = false;
    std::atomic<platform::pointer> pointer;

public:
    platform::pointer get_pointer() const;

    void set_pointer(platform::pointer p);

    void request_shutdown(void);

    bool shutdown_was_requested() const;

    void change_room(idle::room_id_enum id);
};

