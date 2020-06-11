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
#include <atomic>
#include "drawable.hpp"
#include "gl.hpp"

namespace idle
{
class lodge
{
    idle::image_t picture, background;
    float alpha = 0;

  public:
    std::atomic_bool load_status { false };

    template<typename Im1, typename Im2>
    lodge(Im1&& i1, Im2&& i2)
        : picture(std::forward<Im1>(i1)), background(std::forward<Im2>(i2))
    {
    }

    void tick();

    void draw(const graphics::core&) const;

    bool is_done() const;
};

}  //namespace idle
