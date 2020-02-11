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

#include <string.h>
#include "exception.hpp"

namespace idle
{

haiku::haiku(const char * middle, const char * last)
{
    const size_t len[]{
        ::strlen(middle),
        ::strlen(last)
    };

    cached_str = std::make_unique<char[]>(len[0] + len[1] + 2);

    cached_str[len[0]] = '\n';
    cached_str[len[0] + len[1] + 1] = 0x0;

    ::memcpy(cached_str.get(), middle, len[0]);
    ::memcpy(cached_str.get() + len[0] + 1, last, len[1] + 1);
}

const char * haiku::tell() const
{
    return cached_str.get();
}

/*
const char * poet::has_sung() const
{
    return has_sung_.load() ?
            lyric->tell() : nullptr;
}

void poet::sing(const char * middle_verse, const char * last_verse) const
{
    std::lock_guard claim{stage};

    if (has_sung_.load())
    {
        return;
    }

    lyric->emplace(middle_verse, last_verse);
    has_sung_.store(true);
}
*/


}  // namespace idle
