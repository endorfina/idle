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
#include <optional>

#include <log.hpp>
#include "pointer.hpp"
#include "cmd_queue.hpp"
#include "opengl_core_adaptive.hpp"

namespace platform
{

constexpr math::color<float> background{.35f, .3f, .35f};

/* To be implemented: */

void recreate_matrices();

struct resize_request_t
{
    int w, h, r, q;
};


struct window
{
    using data_t = std::byte[sizeof(void*) * 4];

    data_t data;
    pointer cursor;
    command_queue_t commands;
    std::optional<resize_request_t> resize_request;

#ifdef __ANDROID__
    window(struct android_app *);
#else
    window();
#endif

    ~window();

    void terminate_display();

    void buffer_swap();

    void event_loop_back(bool block_if_possible);

    bool has_gl_context() const;
};

}  // namespace platform

