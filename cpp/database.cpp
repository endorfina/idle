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

#include "platform/asset_access.hpp"
#include "gl.hpp"
#include "database.hpp"
#include "config/head.hpp"
#include "freetype_glue.hpp"

// #include <soloud.h>
// #include <soloud_modplug.h>

// void testsoloud() {
//     SoLoud::Soloud soloud;
//     SoLoud::Modplug mp;
//     soloud.init();
//     auto h = idle::parent.hold_asset("battlefield.xm");
//     const std::string_view data = *h;
//     if (auto err = mp.loadMem(data))
//         LOGE("Mod load: %s", soloud.getErrorString(err));}
//     soloud.play(mp);
//     std::this_thread::sleep_for(std::chrono::seconds(30))}
//     soloud.deinit();
// }


// #include "generated_textures.hpp"

namespace idle
{

// namespace
// {
// std::unordered_set<minimal_string, std::hash<minimal_string>, minimal_string::compare> unique_name_set;

// std::string_view unique(const std::string_view c)
// {
//     return unique_name_set.emplace(c).first->view();
// }
//
// std::string ultostr(const unsigned long u)
// {
//     char s[20];
//     snprintf(s, sizeof(s), "%lu", u);
//     return {s};
// }

// std::vector<line> load_speechlist(
//     const std::string_view _file,
//     Database::speech_key_map_t& speech_keys,
//     const std::unordered_map<std::string_view, std::string_view>& character_name_map)
// {
//     blue::parser p;
//     p << _file;
//     const auto& speechlist = std::get<blue::list>(p[u8"🎭"]);
//     std::vector<line> lines;
//     std::forward_list<line::options_pair_t> options;
//     LOGD("🎭 has %zu items", speechlist.size());
//     for (size_t i = 0; i < speechlist.size(); ++i)
//     {
//         if (const auto var = speechlist.yield<blue::dictionary>(i))
//         {
//             using op_t = line::options_enum;
//             if (const auto s = var->yield<blue::string_t>("name")) {
//                 auto fdcn = character_name_map.find(*s);
//                 options.emplace_front(op_t::Name, fdcn != character_name_map.end() ?
//                         fdcn->second : unique(*s));
//             }
//             if (const auto s = var->yield<blue::string_t>("bg")) {
//                 options.emplace_front(op_t::Bg, unique(*s));
//             }
//             if (const auto s = var->yield<blue::string_t>("left")) {
//                 options.emplace_front(op_t::PortraitLeft, unique(*s));
//             }
//             if (const auto s = var->yield<blue::string_t>("right")) {
//                 options.emplace_front(op_t::PortraitRight, unique(*s));
//             }
//         }
//         else if (const auto str = speechlist.yield<blue::string_t>(i))
//         {
//             if (str->size() < 2 || utf8x::sequence_length(str->data()) != str->size())
//             {
//                 lines.emplace_back(*str);
//                 if (!options.empty())
//                     lines.back().options = std::move(options);
//             }
//             else switch (utf8x::get_switch(str->data(), str->size())) {
//                 case blue::Codepoints::PushPin:
//                     if (const auto ptr = speechlist.yield<blue::string_t>(++i))
//                         speech_keys[*ptr] = lines.size();
//                     else throw std::runtime_error{"No text found at speech id " + ultostr(i)};
//                     break;
//                 case blue::Codepoints::Goat:
//                     if (const auto ptr = speechlist.yield<blue::string_t>(++i))
//                         lines.back().options.emplace_front(line::options_enum::Jump,
//                             std::string_view(speech_keys.try_emplace(*ptr).first->first));
//                     else throw std::runtime_error{"No text found at speech id " + ultostr(i)};
//                     break;
//                 case blue::Codepoints::Drama:
//                     if (speechlist.size() >= i + 2) {
//                         if (const auto key = speechlist.yield<blue::string_t>(i + 1),
//                                 label = speechlist.yield<blue::string_t>(i + 2);
//                                 key && label)
//                             lines.back().choices.emplace_front(
//                                 std::string_view(speech_keys.try_emplace(*key).first->first), *label);
//                     }
//                     i += 2;
//                     break;
//             }
//         }
//     }
//     lines.shrink_to_fit();
//     return lines;
// }

// std::unordered_map<std::string_view, std::string_view> create_character_name_map(blue::parser& parser, std::string_view var_name)
// {
//     std::unordered_map<std::string_view, std::string_view> out;
//     if (parser.has_defined(var_name))
//     {
//         auto& var = parser[var_name];
//         if (blue::yield_type(var) != blue::Types::Dictionary)
//             throw std::runtime_error{std::string(var_name) + " is not a dictionary"};
//
//         for (const auto& [name, type] : blue::cast<blue::dictionary&>(var)) {
//             const auto ptr = blue::get<blue::string_t>(&type);
//             if (!ptr)
//                 throw std::runtime_error{std::string(var_name) + "[ " + std::string(name) + " ] is not a string"};
//             out.try_emplace(name, unique_name_set.emplace(*ptr).first->view());
//         }
//     }
//     return out;
// }

// }

bool load_everything(const ::platform::window& sys, graphics::core& gl)
{
    if (const auto fontfile = platform::asset::hold(config::font_asset))
    {
#ifndef __ANDROID__
        constexpr int resolution = 72;
#else
        constexpr int resolution = 48;
#endif
        if (auto opt = fonts::freetype_glue{}.load(fontfile.view(), resolution))
        {
            LOGD("Font acquired!");
            gl.font.swap(opt);
        }
    }
    else
    {
        LOGE("Asset %s not found!", config::font_asset);
        return false;
    }

    LOGD("Asset refresh was successful");
    return true;
}

}  // namespace idle
