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

#include "image_loader.hpp"

namespace idle::hotel
{

image_pool::image_pool() noexcept
{
    start_worker();
}

void image_pool::start_worker() noexcept
{
    worker.start([this]()
    {
        while (worker.is_active())
        {
            std::unique_lock<std::mutex> lock(mutex);
            cond_variable.wait(lock, [this]() { return queue.size() || !worker.is_active(); });

            for (const auto [fn, out, q] : queue)
            {
                *out = db.load_from_assets(fn, q);
            }
            queue.clear();
        }
    });
}

void image_pool::load_image(const char * filename, images::texture& out, GLint quality) noexcept
{
    {
        std::lock_guard lock{mutex};
        queue.push_back(item{filename, &out, quality});
    }
    cond_variable.notify_one();
}

image_pool::~image_pool() noexcept
{
    kill_worker();
}

void image_pool::kill_worker() noexcept
{
    {
        std::lock_guard lock{mutex};
        worker.set_active(false);
    }
    cond_variable.notify_one();
    worker.stop();
}

void image_loader::load_queued_images() noexcept
{
    pool.db.load_topmost_queued_picture();
}

void image_loader::start_workers() noexcept
{
    pool.start_worker();
}

void image_loader::kill_workers() noexcept
{
    pool.kill_worker();
}

}  // namespace idle::hotel

