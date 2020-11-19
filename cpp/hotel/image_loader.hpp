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
#include <mutex>
#include <condition_variable>
#include <vector>
#include <utility>
#include <math_defines.hpp>
#include "service.hpp"
#include "../png/image_queue.hpp"

namespace idle::hotel
{

class image_pool
{
    std::mutex mutex;
    std::condition_variable cond_variable;
    room_service worker;
    std::vector<std::pair<const char*, GLuint*>> queue;

public:
    images::database db;

    image_pool() noexcept;

    ~image_pool() noexcept;

    void load_image(const char * filename, GLuint& out) noexcept;

    void kill_worker() noexcept;
};

struct image_loader
{
    image_pool pool;

    void load_queued_images() noexcept;

    void kill_workers() noexcept;
};

}  // namespace idle::hotel
