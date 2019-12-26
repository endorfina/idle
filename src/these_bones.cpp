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

#include <iostream>
#include <algorithm>
#include "these_bones.hpp"
#include "blue/script.hpp"
#include <log.hpp>

namespace idle::models
{

bone& skeleton::add_bone_to_the_bag()
{
    return _bag_.emplace_back();
}

void skeleton::remove_bone(const unsigned pos)
{
    if (pos >= _bag_.size())
        return;

    const auto parent = _bag_[pos].parent_id;
    _bag_.erase(_bag_.begin() + pos);

    std::for_each(_bag_.begin(), _bag_.end(), [parent, pos](bone& b){
            if (b.parent_id == pos)
                b.parent_id = parent;
            else if (b.parent_id > pos)
                --b.parent_id;
        });
}

size_t skeleton::size() const
{
    return _bag_.size();
}

std::unordered_map<std::string, skeleton> load(std::string_view view)
{
    constexpr std::string_view _map_name = u8"💀";
    std::unordered_map<std::string, skeleton> out;
    blue::parser p;
    p << view;

    if (!p.has_defined(_map_name) || !std::get_if<blue::dictionary>(&p[_map_name])) {
        return {};
    }


    const auto& map = std::get<blue::dictionary>(p[_map_name]);
    for (const auto name : map.keys())
    {
        skeleton sk;
        if (const auto skely = map.yield<blue::dictionary>(name))
        {
            if (const auto cn = skely->yield<blue::list>("center"); cn && cn->size() == 3)
            {
                auto val = cn->yield<blue::float_t>(0);
                if (val)
                    sk.center.x = *val;
                val = cn->yield<blue::float_t>(1);
                if (val)
                    sk.center.y = *val;
                val = cn->yield<blue::float_t>(2);
                if (val)
                    sk.center.z = *val;
            }

            if (const auto pile = skely->yield<blue::list>("pile"))
            {
                for (const auto bp : pile->filter<blue::dictionary>())
                {
                    auto& bo = sk.add_bone_to_the_bag();

                    if (const auto an = bp->yield<blue::list>("angle"); an && an->size() == 3)
                    {
                        auto val = an->yield<blue::float_t>(0);
                        if (val)
                            bo.angle.x = *val;
                        val = an->yield<blue::float_t>(1);
                        if (val)
                            bo.angle.y = *val;
                        val = an->yield<blue::float_t>(2);
                        if (val)
                            bo.angle.z = *val;
                    }

                    if (const auto len = bp->yield<blue::float_t>("len"))
                    {
                        bo.length = *len;
                    }
                    else {
                        const auto names = bp->keys();
                        for (auto n : names)
                        {
                            std::cout << n << " size: " << n.size() << '\n';
                        }
                    }

                    if (const auto jo = bp->yield<blue::int_t>("parent"))
                    {
                        if (*jo >= 0)
                            bo.parent_id = static_cast<unsigned>(*jo);
                    }
                }
            }
        }

        out.try_emplace(std::string(name), std::move(sk));
    }
    return out;
}

bone& skeleton::operator[](unsigned pos)
{
    return _bag_[pos];
}

const bone& skeleton::operator[](unsigned pos) const
{
    return _bag_[pos];
}

const std::vector<bone>& skeleton::bag() const
{
    return _bag_;
}

}
