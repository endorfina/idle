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
#include <memory>
#include <map>
#include <mutex>
#include <math.hpp>

#include "gl.hpp"
#include "objects.hpp"
#include "scoped_thread.hpp"

#define DARKNESS_TEXTURE_SIZE 64

namespace idle
{
class red_room
{
public:
    red_room(::overlay&);

    bool step(::overlay&);

    void post_step(::overlay&);

    void draw(const graphics::core& gl) const;

    void post_draw(const graphics::core& gl) const;

    struct _dialog_system_type_ {
        unsigned current_line = 0, last_line = 0, next_line = 0;
        std::string text;
        long current_text;
        bool text_done; //, has_choice = false;
        const char * current_name_displayed = nullptr;
        struct portrait {
            unsigned id = 0u, id_prev = 0u;
            float transition = 0.f;
            mat4x4_t mat, mat_prev;
        };
        image_t * bg = nullptr, * bg_prev = nullptr;
        float bg_transition = 0.f;
        mat4x4_t mat_bg, mat_bg_prev;
        std::array<portrait, 2> port;
        float indicator_anim = 0.f;
        int chosen = -1;
        const vn_choices *choices = nullptr;
        int delay = 0;

        std::array<std::pair<float,float>, 16> animation;
        float height = 0, height_displayed = 0;

        void build_bg_matrix(float _RWidth, float _RHeight, float imgw, float imgh);

        void clear();

    } dialogSys;

    player_t player;
    uint8_t game_mode = 1;
    idle::mat4x4_t camera = idle::mat4x4_t::identity();

    point_t camera_pos;
    float camera_scale = 1.f;

    std::mutex light_lock;
    int light_y = 0;
    std::unique_ptr<idle::scoped_thread> worker_thread;
    float darkness = .9f;
    int darkness_update_timer = 0;
    math::grid<uint8_t> darkness_data = violet::grid<uint8_t>(DARKNESS_TEXTURE_SIZE, DARKNESS_TEXTURE_SIZE);
    std::map<const idle::object *, idle::light_source> lights;

    float top_speed = 0.f;

    void RecreateGroundTexture();

    void create_worker(const ::overlay&);
};

}
