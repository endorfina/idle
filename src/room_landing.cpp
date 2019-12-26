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

#include "room_landing.hpp"
#include "room_model.hpp"
#include "top.hpp"
#include "draw_text.hpp"

namespace idle
{
// #ifdef COMPILE_M_ROOM
// static void _draw_maker_room_button_(const graphics::core& gl, point_t pos, const gui_elem& elem)
// {
//     const float v[] = {
//             pos.x, pos.y,
//             pos.x + elem.bound.x, pos.y,
//             pos.x, pos.y + elem.bound.y
//     };
//     gl.pfill.use();
//     gl.pfill.set_color(elem.bg, elem.hover ? elem.bg.a : elem.bg.a / 2);
//     gl.pfill.position_vertex(v);
//     gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 3);
//     gl.ptext.use();
//     gl.ptext.set_color(elem.fg);
//     gl.draw_text("Make", pos + elem.bound / 3.6f, 24, TextAlign::Center, TextAlign::Center);
// }
// #endif


landing_room::landing_room(::overlay& parent)
{
// #ifdef COMPILE_M_ROOM
//     create_gui_elem(parent.collection, {0, 0}, {100, 50}, gui_elem::trigger_t::MakerRoom, _draw_maker_room_button_, false, {1.f, 0.f, .1f});
// #endif
}

bool landing_room::step(::overlay& parent)
{
    // if (auto clicked_obj = check_hover_and_get_clicked(parent))
    //     switch (std::get<gui_elem>(clicked_obj->data).trigger)
    //     {
    //         case gui_elem::trigger_t::MakerRoom:
    //             parent.room_next_id = room_id_enum::room_models;
    //             break;
    //         default:
    //             break;
    //     }
    return true;
}

}  // namespace idle

