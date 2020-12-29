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

inline constexpr float cell = .25f;

inline constexpr poly::composition_mesh human_mesh
{
    std::make_tuple(
        poly::torso_mesh
        {
            selector::join
            {
                std::make_tuple(
                    selector::segment<parts::upperbody, 1>{1},
                    selector::segment<parts::lowerbody, 2>{0}
                )
            },

            extra::smooth,

            selector::join
            {
                std::make_tuple(
                    selector::segment<parts::upperbody, 1>{},
                    selector::split<parts::shoulders, 2>{}
                )
            },

            selector::join
            {
                std::make_tuple(
                    selector::segment<parts::lowerbody, 1>{},
                    selector::split<parts::hips, 2>{}
                )
            },

            skin::equiv_rect
            {
                point_t{ 0.f, cell * 5 },
                point_t{ cell * 2, cell * 3 }
            },

            extra::uniform_sym_strip<0, 3>(16.f, -5.f, 5.f)
        },

        poly::blob_mesh
        {
            selector::split<parts::shoulders, 4>{1},

            extra::curved,

            skin::equiv_rect
            {
                point_t{ cell, cell * 2 },
                point_t{ cell, cell * 3 }
            },

            extra::uniform_sym_strip<0, 7>(8.f, -2.f, 2.f)
        },

        poly::blob_mesh
        {
            selector::split<parts::shoulders, 4, true>{1},

            extra::curved,

            skin::equiv_rect
            {
                point_t{ 0.f, cell * 2 },
                point_t{ cell, cell * 3 }
            },

            extra::uniform_sym_strip<0, 7>(8.f, -2.f, 2.f)
        },

        poly::blob_mesh
        {
            selector::split<parts::hips, 2>{3},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ cell, cell * 11 },
                point_t{ cell, cell }
            },

            extra::uniform_sym_strip<>(8.f, -2.f, 2.f)
        },

        poly::blob_mesh
        {
            selector::split<parts::hips, 2, true>{3},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ 0.f, cell * 11 },
                point_t{ cell, cell }
            },

            extra::uniform_sym_strip<>(8.f, -2.f, 2.f)
        },

        poly::blob_mesh
        {
            selector::split<parts::hips, 3>{1},

            extra::curved,

            skin::equiv_rect
            {
                point_t{ cell, cell * 8 },
                point_t{ cell, cell * 3 }
            },

            extra::uniform_sym_strip<0, 5>(8.f, -3.f, 2.f)
        },

        poly::blob_mesh
        {
            selector::split<parts::hips, 3, true>{1},

            extra::curved,

            skin::equiv_rect
            {
                point_t{ 0.f, cell * 8 },
                point_t{ cell, cell * 3 }
            },

            extra::uniform_sym_strip<0, 5>(8.f, -3.f, 2.f)
        },

        poly::blob_mesh
        {
            selector::segment<parts::head, 2>{},

            extra::smooth,

            skin::equiv_rect
            {
                point_t{ cell * 3, cell * 7 },
                point_t{ cell, cell }
            },

            std::make_tuple(
                skin::sym{-8.f, 1, 2.f},
                skin::sym{-8.f, 0}
            )
        },

        poly::face_mesh
        {
            selector::segment<parts::head, 2>{1},

            extra::flat,

            {
                skin::sym{-21.5f, 1, 3.f},
                skin::sym{-21.5f, 0, -3.f}
            },

            {
                skin::equiv_rect
                {
                    point_t{ 0.f, 0.f },
                    point_t{ cell * 2, cell * 2 }
                },

                skin::equiv_rect
                {
                    point_t{ cell * 2, 0.f },
                    point_t{ cell * 2, cell }
                },

                skin::equiv_rect
                {
                    point_t{ cell * 2, cell * 5 },
                    point_t{ cell, cell }
                },

                skin::equiv_rect
                {
                    point_t{ cell * 2, cell * 8 },
                    point_t{ cell * 2, cell * 2 }
                },

                skin::equiv_rect
                {
                    point_t{ cell * 2, cell * 10 },
                    point_t{ cell * 2, cell * 2 }
                }
            }
        }
    ),

    drawing::humanoid
};

}  // namespace idle::glass::paint

