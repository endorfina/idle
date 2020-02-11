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
#include <string_view>
#include <variant>
#include <unordered_map>
#include <optional>
#include <memory>

namespace blue
{
    enum Codepoints : uint_least32_t
    {
        SuitHeart = 0x2665,
        RedHeart = 0x2764,
        Tangerine = 0x1F34A,
        Ghost = 0x1F47B,
        Grape = 0x1F347,
        Watermelon = 0x1F349,
        Strawberry = 0x1F353,
        Lemon = 0x1F34B,
        CrossMark = 0x274C,
        SoftIceCream = 0x1F366,
        IceCream = 0x1F368,
        Custard = 0x1F36E,
        WrappedGift = 0x1F381,
        Scroll = 0x1F4DC,
        GrinningFace = 0x1F600,
        ChequeredFlag = 0x1F3C1,
        PushPin = 0x1F4CC,
        Ribbon = 0x1F380,
        Lollipop = 0x1F36D,
        Ticket = 0x1F3AB,
        Key = 0x1F511,
        SpaceInvader = 0x1F47E,
        ABC = 0x1F524,
        Honey = 0x1F36F,
        Eggplant = 0x1F346,
        Cloud = 0x2601,
        Sparkles = 0x2728,
        Rocket = 0x1F680,
        Locomotive = 0x1F682,
        Ok = 0x1F44C,
        Candy = 0x1F36C,
        Thinking = 0x1F914,
        Bell = 0x1F514,
        Angry = 0x1F620,
        ThumbsUp = 0x1F44D,
        ThumbsDown = 0x1F44E,
        Goat = 0x1F410,
        Drama = 0x1F3AD
    };
    // enum class Operator : uint_fast8_t { none, equality, lessthan, greaterthan };
    enum class Types : uint_fast8_t { Bool, Integer, Float, String, Function, Dictionary, List, Reference, Error };

    //struct function;
    using bool_t = bool;
    using int_t = long;
    using float_t = float;
    using string_t = std::string_view;
    class dictionary;
    class list;
    struct reference;

    using type_underlying_representation_t = std::underlying_type_t<Types>;
    using variable_t = std::variant<int_t, float_t, string_t, bool_t, Types, dictionary, list, reference>;
    using namespace_t = std::unordered_map<std::string_view, variable_t>;
    using return_t = std::optional<variable_t>;
    using args_t = std::unique_ptr<variable_t[]>;
}
