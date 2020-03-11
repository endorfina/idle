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
#include "drawable.hpp"
#include "gl.hpp"

namespace idle
{
class lodge
{
#ifdef DEBUG
    unsigned frame_count = 0;
#endif
    idle::image_t picture, background;
    float alpha = 0;
    // bool loaded = false;

  public:
    template<typename Im1, typename Im2>
    lodge(Im1&& i1, Im2&& i2)
        : picture(std::forward<Im1>(i1)), background(std::forward<Im2>(i2))
    {
    }

#ifdef DEBUG
    ~lodge();
#endif

    void draw(const graphics::core&);
};

}  //namespace idle
