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
            selector::join
            {
                std::make_tuple(
                    selector::segment<parts::upperbody, 1>{1},
                    selector::segment<parts::lowerbody, 2>{0}
                )
            },

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, 0.f },
                point_t{ 1.f, 1.f / 3 }
            },

            std::make_tuple(
                skin::sym{18.f, 0, -3.f},
                skin::sym{18.f, 1},
                skin::sym{18.f, 2, 3.f}
            )
        },

        poly::blob_mesh
        {
            selector::split<parts::shoulders, 4>{1},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, 0.f },
                point_t{ 1.f, 1.f / 4 }
            },

            std::make_tuple(
                skin::sym{8.f, 0, -4.f},
                skin::sym{8.f, 1},
                skin::sym{8.f, 2},
                skin::sym{8.f, 3, 2.f}
            )
        },

        poly::blob_mesh
        {
            selector::split<parts::shoulders, 4, true>{1},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, 0.f },
                point_t{ 1.f, 1.f / 4 }
            },

            std::make_tuple(
                skin::sym{8.f, 0, -4.f},
                skin::sym{8.f, 1},
                skin::sym{8.f, 2},
                skin::sym{8.f, 3, 2.f}
            )
        },

        poly::blob_mesh
        {
            selector::split<parts::hips, 3>{1},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, 0.f },
                point_t{ 1.f, 1.f / 4 }
            },

            std::make_tuple(
                skin::sym{10.f, 0, -4.f},
                skin::sym{10.f, 1},
                // skin::sym{8.f, 2},
                skin::sym{10.f, 2, 2.f}
            )
        },

        poly::blob_mesh
        {
            selector::split<parts::hips, 3, true>{1},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, 0.f },
                point_t{ 1.f, 1.f / 4 }
            },

            std::make_tuple(
                skin::sym{10.f, 0, -4.f},
                skin::sym{10.f, 1},
                // skin::sym{8.f, 2},
                skin::sym{10.f, 2, 2.f}
            )
        },

        poly::blob_mesh
        {
            selector::segment<parts::head, 2>{},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, 0.f },
                point_t{ 1.f, .5f }
            },

            std::make_tuple(
                skin::sym{-8.f, 1},
                skin::sym{-8.f, 0}
            )
        },

        poly::blob_mesh
        {
            selector::segment<parts::head, 2>{1},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, 0.f },
                point_t{ 1.f, .5f }
            },

            std::make_tuple(
                skin::sym{-14.f, 1, 4.f},
                skin::sym{-14.f, 0, -4.f}
            )
        }
    ),

    drawing::humanoid
};

}  // namespace idle::glass::paint

