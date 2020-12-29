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
#include <thread>
#include <condition_variable>
#include <forward_list>
#include <deque>
#include <utility>
#include <variant>
#include <math_defines.hpp>

namespace idle::hotel::stage
{

class crawler_pool
{
    std::mutex cond_mutex;
    std::condition_variable cond_variable;
    bool worker_alive_flag = false;
    std::forward_list<std::thread> workers;

    template<typename Check>
    bool wait(const Check& check) noexcept
    {
        std::unique_lock<std::mutex> lock(cond_mutex);
        cond_variable.wait(lock, [this, &check]() { return check() && !worker_alive_flag; });

        return worker_alive_flag;
    }

public:
    ~crawler_pool() noexcept;

    template<typename Check, typename Func>
    void start_worker(const unsigned amount, const Check& check, const Func& func) noexcept
    {
        const auto lambda = [this, &check, &func]()
        {
            while (wait(check))
            {
                func();
            }
        };

        for (unsigned i = 0; i < amount; ++i)
        {
            workers.emplace_front(lambda);
        }
    }

    void kill_worker() noexcept;

    void notify() noexcept;
};

template<typename Variant>
struct crawler_queue
{
    using variant_type = Variant;

private:
    std::deque<variant_type> queue;
    std::atomic_uint active = 0, index = 0, size = 0;
    crawler_pool pool;

public:
    void wait() noexcept
    {
        while (!!active.load(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }
        size.store(0, std::memory_order_relaxed);
        index.store(0, std::memory_order_relaxed);
        queue.clear();
    }

    void notify() noexcept
    {
        size.store(queue.size(), std::memory_order_relaxed);
        pool.notify();
    }

    template<typename It, typename Gen>
    void generate(const unsigned copies, It it, const Gen& gen) noexcept
    {
        for (unsigned i = 0; i < copies; ++i)
        {
            queue.emplace_back(gen(it++));
        }
    }

    template<typename It>
    void copy_iter(const unsigned copies, It it) noexcept
    {
        for (unsigned i = 0; i < copies; ++i)
        {
            queue.emplace_back(variant_type{it++});
        }
    }

private:
    template<typename Vis>
    bool run(const Vis& visitor) noexcept
    {
        constexpr unsigned step = 4;
        auto i = index.fetch_add(step, std::memory_order_relaxed);
        const auto max_size = std::min<unsigned>(i + step, size.load(std::memory_order_relaxed));

        if (i >= max_size)
        {
            return false;
        }
        else
        {
            active.fetch_add(1, std::memory_order_release);
            do
            {
                std::visit(visitor, queue[i]);
            }
            while (++i < max_size);

            active.fetch_sub(1, std::memory_order_release);
            return true;
        }
    }

public:
    template<typename Vis>
    crawler_queue(const Vis& visitor) noexcept
    {
        pool.start_worker(5,
            [this]
            {
                return !!size.load(std::memory_order_relaxed);
            },

            [this, &visitor]
            {
                this->template run<Vis>(visitor);
            });
    }
};

}  // namespace idle::hotel::stage
