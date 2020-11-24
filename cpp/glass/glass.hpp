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

#include "blocks.hpp"
#include "trees.hpp"
#include "rigs.hpp"
#include "skin.hpp"

namespace idle::glass::paint
{

inline constexpr poly::composition_mesh human_mesh
{
    std::make_tuple(
        poly::blob_mesh
        {
            selector::segment<parts::head, 3>{},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, 0.f },
                point_t{ 1.f, .33f }
            },

            std::make_tuple(
                skin::sym{3.f, 0},
                skin::sym{9.f, 1},
                skin::sym{5.f, 2}
            )
        }
    ),

    [] (const auto& drawable, const auto& average_zs)
    {
        return extra::default_drawable{ drawable };
    }
};

}  // namespace idle::glass::paint

