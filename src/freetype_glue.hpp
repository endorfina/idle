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

#include <string_view>
#include <optional>
#include "glyph.hpp"

namespace fonts
{
enum class texture_quality : unsigned
{
    poor = 2 << 9,
    ok = 2 << 10,
    sharp = 2 << 11,
    ludicrous = 2 << 12
};

struct freetype_glue
{
    std::optional<ft_data_t> operator()(bool (* filter_function)(unsigned long), const std::string_view &memory, texture_quality resolution) const;

    freetype_glue();
    ~freetype_glue();

    freetype_glue(const freetype_glue &) = delete;
    freetype_glue(freetype_glue &&) = delete;
};

}
