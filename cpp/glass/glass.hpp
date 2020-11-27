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

inline constexpr float cell = 1.f / 16;

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
                point_t{ cell, cell * 1.5f },
                point_t{ cell, cell * 1.5f / 2 }
            },

            std::make_tuple(
                skin::sym{20.f, 0, -5.f},
                skin::sym{20.f, 1},
                skin::sym{20.f, 2, 5.f}
            )
        },

        poly::blob_mesh
        {
            selector::split<parts::shoulders, 4>{1},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ cell / 2, cell * 1.5f },
                point_t{ cell / 2, cell / 3 }
            },

            std::make_tuple(
                skin::sym{9.f, 0, -4.f},
                skin::sym{9.f, 1},
                skin::sym{9.f, 2},
                skin::sym{9.f, 3, 2.f}
            )
        },

        poly::blob_mesh
        {
            selector::split<parts::shoulders, 4, true>{1},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, cell * 1.5f },
                point_t{ cell / 2, cell / 3 }
            },

            std::make_tuple(
                skin::sym{9.f, 0, -4.f},
                skin::sym{9.f, 1},
                skin::sym{9.f, 2},
                skin::sym{9.f, 3, 2.f}
            )
        },

        poly::blob_mesh
        {
            selector::split<parts::hips, 2>{3},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ cell / 2, cell * 3.5f },
                point_t{ cell / 2, cell / 2 }
            },

            std::make_tuple(
                skin::sym{10.f, 0, -4.f},
                skin::sym{10.f, 1, 2.f}
            )
        },

        poly::blob_mesh
        {
            selector::split<parts::hips, 2, true>{3},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, cell * 3.5f },
                point_t{ cell / 2, cell / 2 }
            },

            std::make_tuple(
                skin::sym{10.f, 0, -4.f},
                skin::sym{10.f, 1, 2.f}
            )
        },

        poly::blob_mesh
        {
            selector::split<parts::hips, 3>{1},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ cell / 2, cell * 2.5f },
                point_t{ cell / 2, cell / 2 }
            },

            std::make_tuple(
                skin::sym{10.f, 0, -4.f},
                skin::sym{10.f, 1},
                skin::sym{10.f, 2, 2.f}
            )
        },

        poly::blob_mesh
        {
            selector::split<parts::hips, 3, true>{1},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, cell * 2.5f },
                point_t{ cell / 2, cell / 2 }
            },

            std::make_tuple(
                skin::sym{10.f, 0, -4.f},
                skin::sym{10.f, 1},
                skin::sym{10.f, 2, 2.f}
            )
        },

        poly::blob_mesh
        {
            selector::segment<parts::head, 2>{},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, cell },
                point_t{ cell / 2, cell / 2 }
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
                point_t{ cell, cell }
            },

            std::make_tuple(
                skin::sym{-16.f, 1, 6.f},
                skin::sym{-16.f, 0, -6.f}
            )
        }
    ),

    drawing::humanoid
};

}  // namespace idle::glass::paint

