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

#include "top.hpp"

void overlay::request_shutdown(void)
{
    shutdown_was_requested_ = true;
}

bool overlay::shutdown_was_requested() const
{
    return shutdown_was_requested_;
}

platform::pointer overlay::get_pointer() const
{
    return pointer.load(std::memory_order_relaxed);
}

void overlay::set_pointer(platform::pointer p)
{
    p.pos *= gl.translate_vector;
    pointer.store(p, std::memory_order_relaxed);
}


void overlay::need_a_light(const idle::object *attach, float x, float y, float r, unsigned char pow) {
    // if (room_current_id == idle::room_id_enum::room_red) {
    //     reinterpret_cast<idle::red_room*>(room_ctrl_ptr.get())->lights.emplace(attach, idle::light_source{ attach, x, y, r, pow });
    // }
}

