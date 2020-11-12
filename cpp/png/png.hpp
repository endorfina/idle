/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of the Idle.

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
#include <vector>
#include <string_view>

namespace idle
{

struct png_image_data
{
    std::unique_ptr<unsigned char[]> image;
    unsigned width, height, real_width = 1, real_height = 1, size;

private:
    std::vector<unsigned char> decode_png_buffer(const unsigned char * const src, const size_t datalen) noexcept;

    void decode(const std::string_view source) noexcept;

public:
    png_image_data(const char* filename) noexcept;
};

}  // namespace idle
