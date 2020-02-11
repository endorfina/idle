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

#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <math.hpp>
#include "gl_programs.hpp"

namespace idle::models {

struct bone
{
    float length;
    point_3d_t angle, lower_bound, upper_bound;
    float stiffness = 0;
    unsigned parent_id = static_cast<unsigned>(-1);
};

struct skeleton
{
private:
    std::vector<bone> _bag_;

public:
    point_3d_t center{};

    bone& add_bone_to_the_bag();
    void remove_bone(const unsigned pos);

    bone& operator[](unsigned pos);
    const bone& operator[](unsigned pos) const;

    size_t size() const;

    const std::vector<bone>& bag() const;
};

std::unordered_map<std::string, skeleton> load(std::string_view);

}
