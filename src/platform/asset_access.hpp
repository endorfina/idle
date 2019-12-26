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

class asset
{
#ifdef __ANDROID__
protected:
    static const struct android_app * android_activity;
    friend struct window;

    AAsset * file = nullptr;
    std::string_view data;

    asset() = default;
    asset(AAsset*, std::string_view);
#else
    std::unique_ptr<char[]> ptr;
    size_t size = 0;
#endif

public:
    operator bool() const;

    std::string_view view() const;

    ~asset();

    static asset hold(const char * path);

    static asset hold(std::string path);
};

}  // namespace platform

