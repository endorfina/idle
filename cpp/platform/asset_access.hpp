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
#include <memory>
#include <string>
#include <log.hpp>

namespace platform
{

struct context;

class asset
{
#ifdef __ANDROID__
    static struct android_app * android_activity;

    friend void ::android_main(android_app *);
    friend struct context;

    AAsset * file = nullptr;
    std::string_view data;

    asset() noexcept = default;
    asset(AAsset*, std::string_view) noexcept;

public:
    asset(asset&&) noexcept;

    ~asset() noexcept;

#else
private:
    std::unique_ptr<unsigned char[]> ptr;
    size_t size = 0;

    asset() noexcept = default;

    template<typename Ptr>
    asset(Ptr&& p, size_t s) noexcept
        : ptr(std::forward<Ptr>(p)), size(s) {}

public:
    asset(asset&&) noexcept = default;

#endif

    operator bool() const noexcept;

    std::string_view view() const noexcept;

    asset(const asset&) = delete;

    static asset hold(const char * path) noexcept;

    static asset hold(std::string path) noexcept;
};

}  // namespace platform

