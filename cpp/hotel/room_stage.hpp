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
#include <idle/pointer_wrapper.hpp>
#include <idle/draw_text.hpp>
#include "gui.hpp"
#include "keys.hpp"
#include <colony.hpp>
#include "image_loader.hpp"
#include <mutex>
#include "stage_objects.hpp"

namespace idle::hotel::stage
{

struct room : image_loader
{
private:
    std::mutex cell_mod_mutex;
    std::atomic_uint8_t draw_fork = 0;
    std::array<std::vector<const object*>, 6> render_order;
    cells::colony<unsigned, object> objs;
    player_object player;
    std::array<uint8_t, 32 * 32> floor_tiles;

public:
    using iterator_type = typename cells::colony<unsigned, object>::iterator;

    room() noexcept;

    void on_resize(point_t) noexcept;

    std::optional<keyring::variant> step(const pointer_wrapper& cursor) noexcept;

    void draw(const graphics::core& gl) noexcept;
};

}  // namespace idle::hotel::stage

