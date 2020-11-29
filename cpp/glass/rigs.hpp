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

namespace idle::glass
{

template<typename... Links>
struct muscle
{
    std::tuple<Links...> chain;

    constexpr muscle(const std::tuple<Links...>& var) noexcept
        : chain{var}
    {}

protected:
    template<unsigned Index = 0, typename Load>
    constexpr void expand(Load& load) const noexcept
    {
        constexpr auto source_index = Index > 0 ? Index - 1 : 0;
        load[Index] = std::get<Index>(chain)(load[source_index]);

        if constexpr (Index + 1 < sizeof...(Links))
        {
            expand<Index + 1, Load>(load);
        }
    }

public:
    template<typename Val>
    using array_t = std::array<Val, sizeof...(Links)>;

    template<typename Cargo>
    constexpr array_t<Cargo> animate(const Cargo& cargo) const noexcept
    {
        array_t<Cargo> out{};
        out[0] = cargo;
        expand<0>(out);
        return out;
    }
};


enum struct parts
{
    shoulders,
    hips,
    upperbody,
    lowerbody,
    head
};

namespace closet
{

struct humanoid : blocks::joint
                <
                    blocks::label<parts::upperbody, blocks::joint
                    <
                        blocks::label<parts::head, blocks::segment<2>>,
                        blocks::label<parts::shoulders, blocks::symmetry<blocks::segment<4>>>
                    >>,

                    blocks::label<parts::lowerbody, blocks::joint
                    <
                        blocks::label<parts::hips, blocks::symmetry<blocks::segment<4>>>
                    >>
                >
{
    template<auto Key>
    constexpr const auto& get_ref() const noexcept
    {
        return meta::find_label<Key>(*this);
    }

    template<auto Key>
    constexpr auto& get_ref() noexcept
    {
        return meta::find_label<Key>(*this);
    }

    constexpr void realign() noexcept
    {
        root.length = 1;

        const deep_tree tree(*this, {});
        const auto low = std::min_element(
                tree.table.cbegin(),
                tree.table.cend(),
                [] (const auto& lhs, const auto& rhs) { return lhs.z < rhs.z; }
            )->z;

        if (low < 0)
            root.length -= low;
    }

    constexpr humanoid() noexcept
    {
        auto& [neck, face] = get_ref<parts::head>().table;
        neck.length = 12;
        face.length = 14;
        face.angle.y = - (neck.angle.y = math::tau / 30);

        auto& ub = get_ref<parts::upperbody>().root;
        auto& lb = get_ref<parts::lowerbody>().root;

        ub.length = lb.length = 15;

        lb.angle.y = math::tau_2;

        auto& sh = get_ref<parts::shoulders>();
        auto& hp = get_ref<parts::hips>();

        auto& arm = sh.left.table;
        auto& leg = hp.left.table;

        arm = {
            blocks::bone{ 6.f, { -math::tau_4, 0, 0 } },
            blocks::bone{ 11.f, { -math::tau_8, 0, 0 } },
            blocks::bone{ 11.f, { 0, 0, 0 } },
            blocks::bone{ 5.f, { -math::tau_8 / 8, math::tau_8 / 8, 0 } }
        };

        constexpr float
                knee_bend = math::tau_4 * .12f,
                thigh_raise = math::tau_4 * .08f;

        leg = {
            blocks::bone{ 5.f, { -math::tau_4, 0, 0 } },
            blocks::bone{ 18.f, { math::tau_4 * .966f, 0, -thigh_raise } },
            blocks::bone{ 18.f, { 0, knee_bend, 0 } },
            blocks::bone{ 7.f, { 0, - knee_bend + thigh_raise - math::tau_4, 0 } }
        };

        meta::sync_right(sh);
        meta::sync_right(hp);

        realign();
    }
};

}  // namespace closet

}  // namespace idle::glass

