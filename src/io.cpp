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

#include <unistd.h>
#include "io.hpp"

namespace idle::io
{
void file::destroy() {
    if (fd != -1)
        ::close(fd);
    fd = -1;
}

file::file(int _fd)
    : fd(_fd) {}

file::~file(void) {
    destroy();
}

file::operator int() const {
    return fd;
}

file::operator bool() const {
    return fd != -1;
}

}  // namespace idle::io
