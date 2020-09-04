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
#include "interface.hpp"
#include <cmath>
#include <math.hpp>

// TEST(GlassCompile) {
    // constexpr auto sp = idle::glass::spine::humanoid::get_default();
    // EXPECT_TRUE(true);
// }

TEST(math_absolute_value)
{
    const float bob = -math::tau;
    const auto wow = math::ce::abs(bob);
    EXPECT_TRUE(wow == -bob);
    EXPECT_FALSE(bob == wow);
}

TEST(math_trig)
{
    const float bob = sin(math::tau_4);
    const float wow = cos(math::tau_4);
    EXPECT_TRUE(math::ce::detail::epsilon_equal(bob, 1.0f));
    EXPECT_TRUE(math::ce::detail::epsilon_equal(wow, 0.f));
}


