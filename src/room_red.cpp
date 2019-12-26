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

#include "room_red.hpp"
#include "top.hpp"
#include "draw_text.hpp"
#include "objects.hpp"
#include "form_gui.hpp"

#define FONTSIZE 28
#define DIALOG_CHOICE_PADDING 50

// constexpr idle::color_t gold_brush{ 1, .95f, .1f };


//  static void VoronoiFill(uint8_t ** m, const int w, const int h, const unsigned int amt, const unsigned int maxi)
//  {
//      unsigned int i, ix, iy, a;
//      float d, e;
//      std::vector<std::pair<point_t, uint8_t>> p;
//      p.reserve(amt * 3 + 1);
//
//      for(i = 0; i < amt; ++i) {
//          p.emplace_back(point_t((rand() % 10000) / 10000.f, (rand() % 10000) / 10000.f), static_cast<uint8_t>(rand() % maxi));
//  //        p[i].first.x = (rand() % 10000) / 10000.f;
//  //        p[i].first.y = (rand() % 10000) / 10000.f;
//  //        p[i].second = static_cast<BYTE>(rand() % maxi);
//      }
//  //    d = .65f / static_cast<float>(w);
//  //    e = 1.f - d;
//  //    for(i = 0; i < amt; ++i) {
//  //        if(p[i].first.x < d)
//  //        {
//  //            p[entireset].first.x = p[i].first.x + 1.f;
//  //            p[entireset].first.y = p[i].first.y;
//  //            p[entireset].second = p[i].second;
//  //            if(++entireset >= amt * 3) break;
//  //        }
//  //        else if(p[i].first.x > e)
//  //        {
//  //            p[entireset].first.x = p[i].first.x - 1.f;
//  //            p[entireset].first.y = p[i].first.y;
//  //            p[entireset].second = p[i].second;
//  //            if(++entireset >= amt * 3) break;
//  //        }
//  //    }
//  //    d = .65f / static_cast<float>(h);
//  //    e = 1.f - d;
//  //    for(i = 0; i < amt; ++i) {
//  //        if(p[i].first.y < d)
//  //        {
//  //            p[entireset].first.x = p[i].first.x;
//  //            p[entireset].first.y = p[i].first.y + 1.f;
//  //            p[entireset].second = p[i].second;
//  //            if(++entireset >= amt * 3) break;
//  //        }
//  //        else if(p[i].first.y > e)
//  //        {
//  //            p[entireset].first.x = p[i].first.x;
//  //            p[entireset].first.y = p[i].first.y - 1.f;
//  //            p[entireset].second = p[i].second;
//  //            if(++entireset >= amt * 3) break;
//  //        }
//  //    }
//
//      for(ix = 0; ix < w; ++ix)
//          for(iy = 0; iy < h; ++iy)
//          {
//              a = 0;
//              d = 1.f;
//              for(auto it = p.begin(); it != p.end(); ++it)
//              {
//                  e = sqrtf(powf(it->first.x - static_cast<float>(ix) / w, 2) + powf(it->first.y - static_cast<float>(iy) / h, 2));
//                  if(e < d)
//                  {
//                      d = e;
//                      a = it - p.begin();
//                  }
//              }
//              m[ix][iy] = p[a].second;
//          }
//  };

static_assert(violet::is_power_of_2(DARKNESS_TEXTURE_SIZE));

namespace idle
{

void red_room::create_worker(const ::overlay& parent)
{
    worker_thread = std::make_unique<scoped_thread>([this, &parent]{
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(30ms); // 500 seems to be the best
        light_lock.lock();
        std::for_each(darkness_data[light_y], darkness_data[light_y] + DARKNESS_TEXTURE_SIZE,
                      [](uint8_t &a) { if (a < 254) a += static_cast<uint8_t>(ceilf((255 - a) * .1333f)); });
        const mat4x4_t mat(camera * parent.gl.projectionMatrix * mat4x4_t::translate(1, -1) * mat4x4_t::scale(DARKNESS_TEXTURE_SIZE * .5f, DARKNESS_TEXTURE_SIZE * -.5f));
        const float div = parent.gl.draw_size.x / parent.gl.draw_size.y;
        auto row = darkness_data[light_y];
        for (auto &ls : lights) {
            if (ls.first->is_active) {
                ls.second.pos += point_t(ls.first->pos.x - ls.second.sx,
                                                 ls.first->pos.y - ls.second.sy);
                ls.second.sx = ls.first->pos.x;
                ls.second.sy = ls.first->pos.y;
            }
            const auto center = mat * ls.second.pos;
            const float r = ls.second.radius * camera_scale;
            if (static_cast<int>(center.y + r) < light_y || static_cast<int>(center.y - r) > light_y)
                continue;
            const int sx = std::max(static_cast<int>(center.x - r / .8f), 0),
                    //sy = std::max(static_cast<int>(center.y - r), 0),
                    ex = std::min(static_cast<int>(center.x + r / .8f), DARKNESS_TEXTURE_SIZE);
                    //ey = std::min(static_cast<int>(center.y + r), DARKNESS_TEXTURE_SIZE);
            for (int ix = sx; ix < ex; ++ix) {
                float d = sqrtf(powf((ix - center.x) * .8f * div, 2) + powf(light_y - center.y, 2));
                if (d < r / 5)
                    row[ix] = ls.second.power < row[ix] ? row[ix] - ls.second.power : 0;
                else if (d < r)
                    row[ix] -= std::min(static_cast<uint8_t>(
                                               (1 - sinf(F_TAU_4 * (d - r * .2f) / (r * .8f))) * ls.second.power), row[ix]); // * 50;
            }
        }
        light_lock.unlock();
        if (++light_y >= DARKNESS_TEXTURE_SIZE)
            light_y = 0;
        return true;
    });
}

// void idle::red_room::RecreateGroundTexture() {
//     const EGLint attrib_list[] = { EGL_HEIGHT, static_cast<EGLint>(TILE_DRAW_SIZE * world.grid.height), EGL_WIDTH, static_cast<EGLint>(TILE_DRAW_SIZE * world.grid.width), EGL_NONE };
//     auto get_display = eglGetCurrentDisplay();
//     auto get_surface = eglGetCurrentSurface(EGL_DRAW);
//     auto get_context = eglGetCurrentContext();

//     EGLSurface new_surface = eglCreatePbufferSurface(get_display, parent.m_EGLconfig, attrib_list);
//     EGLint w, h;
//     eglQuerySurface(get_display, new_surface, EGL_WIDTH, &w);
//     eglQuerySurface(get_display, new_surface, EGL_HEIGHT, &h);

//     if (eglMakeCurrent(get_display, new_surface, new_surface, get_context) == EGL_FALSE) {
//         LOGW("Unable to eglMakeCurrent");
//         throw;
//     }
//     glClearColor(0, .4f, 0, 1);
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//     glUseProgram(DATAB->pid);
//     glViewport(0, 0, w, h);
//     glUniformMatrix4fv(DATAB->mMVPMatrixHandle, 1, GL_FALSE,
//                        mat4x4_t::scale(TILE_DRAW_SIZE, TILE_DRAW_SIZE) * Orthof(0, w, h, 0, -1, 1)
//     );
//     glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, 1);

//     if (glGetError() == 0) {
//         std::array<float, 8> v;
//         const float t[8] = {
//                 0, 0, 1, 0, 0, 1, 1, 1
//         };
//         for (size_t x = 0, y; x < world.grid.width; ++x)
//             for (y = 0; y < world.grid.height; ++y) {
//                 v = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(x) + 1.f, static_cast<float>(y), static_cast<float>(x), static_cast<float>(y) + 1.f, static_cast<float>(x) + 1.f, static_cast<float>(y) + 1.f };
//                 glActiveTexture(GL_TEXTURE0);
//                 glBindTexture(GL_TEXTURE_2D, DATAB->tiles[world.tiles[x][y]].i);
//                 glUniform1i(Database::instance->mTextureHandle, 0);
//                 glVertexAttribPointer(Database::instance->position_handle, 2, GL_FLOAT, GL_FALSE, 0, v.data());
//                 glVertexAttribPointer(Database::instance->texture_position_handle, 2, GL_FLOAT, GL_FALSE, 0, t);
//                 glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//             }
//         size_t u2 = 1, v2 = 1;
//         while (u2 < w) u2 *= 2;
//         while (v2 < h) v2 *= 2;
//         std::vector<uint8_t> image(static_cast<size_t>(w * h) * 4u), image2(u2 * v2 * 3);
//         glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image.data());

//         eglDestroySurface(get_display, new_surface);
//         if (eglMakeCurrent(get_display, get_surface, get_surface, get_context) == EGL_FALSE) {
//             LOGW("Unable to eglMakeCurrent");
//             throw;
//         }

//         for (size_t y = 0; y < h; ++y)
//             for (size_t x = 0; x < w; ++x)
//                 memcpy(image2.data() + (3 * u2 * y + 3 * x), image.data() + (4 * w * y + 4 * x), 3);

//         image.clear();

//         GLuint texID;
//         glGenTextures(1, &texID);
//         glBindTexture(GL_TEXTURE_2D, texID);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_NEAREST = no smoothing
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //GL_LINEAR = smoothing
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, u2, v2, 0, GL_RGB, GL_UNSIGNED_BYTE, image2.data());

//         glBindTexture(GL_TEXTURE_2D, 0);

//         if (DATAB->current_tileset != nullptr)
//             delete DATAB->current_tileset;
//         DATAB->current_tileset = new idle::image_t(texID, w, h, u2, v2);
//     } else {
//         eglDestroySurface(get_display, new_surface);
//         if (eglMakeCurrent(get_display, get_surface, get_surface, get_context) == EGL_FALSE) {
//             LOGW("Unable to eglMakeCurrent");
//             throw;
//         }
//     }
//     eglQuerySurface(get_display, get_surface, EGL_WIDTH, &w);
//     eglQuerySurface(get_display, get_surface, EGL_HEIGHT, &h);
//     glViewport(0, 0, w, h);
//     DATAB->SetTransform(mat4x4_t::identity());
// }

red_room::red_room(::overlay& parent)
{
    memset(*darkness_data.m, 0xff, DARKNESS_TEXTURE_SIZE * DARKNESS_TEXTURE_SIZE);

    camera_pos.x = (-static_cast<float>(player.possesed->pos.x) + parent.gl.draw_size.x * .5f);
    camera_pos.y = 0;
    camera_scale = 0;
    camera = mat4x4_t::translate(camera_pos.x, camera_pos.y) *
            mat4x4_t::scale(camera_scale, camera_scale, point_t(parent.gl.draw_size.x * .5f, parent.gl.draw_size.y * .5f));

    //::VoronoiFill(world.tiles.m, world.grid.width, world.grid.height, 24, DATAB->tiles.size());

    parent.need_a_light(player.possesed, 0, -100, 72, 16);
    //RecreateGroundTexture();
}

void red_room::_dialog_system_type_::clear()
{
    current_line = last_line = 0u;
    text.clear();
    current_text = 0;
    text_done = false;
    current_name_displayed = nullptr;
    bg = bg_prev = nullptr;
    bg_transition = 0.f;
    height = height_displayed = 0.f;
    for (auto &it : port) {
        it.id = it.id_prev = 0u;
        it.transition = 0.f;
    }
    indicator_anim = 0.f;
    chosen = -1;
    choices = nullptr;
    delay = 0;
}

// void red_room::_dialog_system_type_::ReadLine(Database &db, Database::line &ne)
// {
//     text_done = false;
//     bg_prev = bg;
//     if (ne.addr > 0)
//         text = db.text.c_str() + ne.addr;
//     current_text = 0;
//     animation.fill({0, 0});
//     if (text.length() > 0)
//         db.font->prepare_string(text, FONTSIZE, RoomWidth * .85f);
//     next_line = ne.next;
//     if (ne.data)
//     {
//         choices = reinterpret_cast<Database::vn_choices*>(ne.data);
//         chosen = -1;
//         indicator_anim = 0.f;
//         delay = 10;
//         height = 0.f;
//     }
//     else choices = nullptr;
//     if (ne.bg >= 0)
//         if ((bg = static_cast<size_t>(ne.bg) < db.backgrounds.size() ? &db.backgrounds[ne.bg] : nullptr) != bg_prev)
//         {
//             bg_transition = 0.f;
//             mat_bg_prev = mat_bg;
//             if (bg)
//                 BuildBgMatrix(
//                         static_cast<float>(RoomWidth),
//                         static_cast<float>(RoomHeight),
//                         static_cast<float>(bg->width),
//                         static_cast<float>(bg->height)
//                 );
//         }
//     for (auto&&it : port) {
//         it.id_prev = it.id;
//     }
//     for (int i = 0; i < 2; ++i)
//         if (ne.portrait[i] >= 0)
//         if ((port[i].id = static_cast<size_t>(ne.portrait[i]) <= db.portraits.size() ? static_cast<unsigned>(ne.portrait[i]) : 0)
//             != port[i].id_prev)
//         {
//             port[i].transition = 0.f;
//             port[i].mat_prev = port[i].mat;
//             if (port[i].id > 0) {
//                 auto &it = db.portraits[port[i].id - 1];
//                 float sc = RoomHeight * .98f / it.size.height;
//                 port[i].mat = mat4x4_t::translate((i == 0 ? RoomWidth * .3f : RoomWidth * .7f) - it.size.width / 2, RoomHeight - it.size.height) * mat4x4_t::scale(sc, sc, i == 0 ? RoomWidth * .3f : RoomWidth * .7f, RoomHeight - 1.f);
//             }
//         }
//     if (ne.name >= 0)
//         current_name_displayed = db.text.c_str() + db.char_names[ne.name];
//     else if (ne.name == -2)
//         current_name_displayed = nullptr;
// }

// void red_room::_dialog_system_type_::FetchNextLine(Database &db)
// {
//     last_line = current_line;
//     if ((current_line = choices != nullptr ? (*choices)[chosen].second : next_line) >= db.VN.size())
//     {
//         mat_bg_prev = mat_bg;
//         bg_prev = bg;
//         bg = nullptr;
//         bg_transition = 0.f;
//         height = height_displayed = 0.f;
//         parent.game_mode = 1;
//         return;
//     }
//     ReadLine(db, db.VN[current_line]);
// }

bool red_room::step(::overlay& parent)
{
    // if (dialogSys.bg_transition < 1.f)
    //     dialogSys.bg_transition += .0091f;
    // if (parent.game_mode == 0) {
    //  for (auto &&it : dialogSys.port)        /// Portraits
    //      if (it.transition < 1.f)
    //          it.transition += .041f;
    //  if (dialogSys.text_done || dialogSys.choices != nullptr) /// Indicator
    //  {
    //      dialogSys.indicator_anim += .098f;
    //      if (dialogSys.indicator_anim > 1.f)
    //          dialogSys.indicator_anim -= 2.f;
    //  }
    //  else if (!dialogSys.text_done && dialogSys.bg_transition >= 1.f) /// Roll text
    //  {
    //         //auto &db = *DATAB;
    //      if (dialogSys.current_text >= dialogSys.text.length())
    //      {
    //          dialogSys.text_done = true;
    //          dialogSys.indicator_anim = 0.f;
    //      }
    //      else if (--dialogSys.delay <= 0)
    //      {
    //             point_t text_size;
    //             for (int n = FONT_ANIMATION1_ARRAY - 1; n > 0; --n)
    //                 dialogSys.animation[n] = dialogSys.animation[n - 1];
    //             dialogSys.animation[0] = { 0.f, (violet::degtorad(rand() % 100 + 15) - F_PI_4) * 1.274f };
    //          dialogSys.delay = rand() % 4;
    //             parent.font->getExtent(dialogSys.text.c_str(), FONTSIZE, text_size, ++dialogSys.current_text);
    //             dialogSys.height = text_size.height;
    //      }
    //  }
    //     for (auto &a : dialogSys.animation) {       /// Animation
    //         a.first += (F_TAU_4 - a.first) / 6;
    //         a.second -= a.second / 6;
    //     }
    //     dialogSys.height_displayed += (dialogSys.height - dialogSys.height_displayed) / 12;

    //     if (dialogSys.choices)              /// Branching pathways
    //         if (dialogSys.delay > 0)
    //             --dialogSys.delay;
    //     if (parent.get_pointer().single_press && dialogSys.bg_transition >= 1.f) /// Valid press
    //         if (dialogSys.choices) {
    //             if (dialogSys.delay <= 0)
    //             {
    //                 float boxes_height = dialogSys.choices->size() * DIALOG_CHOICE_PADDING;
    //                 float hh = (parent.height - boxes_height) / 2.f;
    //                 parent.get_pointer().touch = 0.f;
    //                 float yy = parent.get_pointer().pos.y;
    //                 if (yy <= hh + DIALOG_CHOICE_PADDING - 5)
    //                     dialogSys.chosen = 0;
    //                 else {
    //                     dialogSys.chosen = static_cast<unsigned int>(yy - (hh - 5.f)) / DIALOG_CHOICE_PADDING;
    //                     if (dialogSys.chosen >= dialogSys.choices->size())
    //                         dialogSys.chosen = dialogSys.choices->size() - 1;
    //                 }
    //             }
    //         }
    //         else
    //         if (dialogSys.text_done)
    //         {
    //             //dialogSys.FetchNextLine(db);
    //             parent.get_pointer().touch = 0.f;
    //             parent.get_pointer().pressed = 0;
    //         }
    //         else
    //         {
    //             point_t text_size;
    //             dialogSys.current_text = dialogSys.text.length();
    //             dialogSys.text_done = true;
    //             dialogSys.indicator_anim = 0.f;
    //             dialogSys.animation.fill({ F_TAU_4, 0 });
    //             parent.font->getExtent(dialogSys.text.c_str(), FONTSIZE, text_size);
    //             dialogSys.height = text_size.height;
    //         }
    //     if (dialogSys.choices && dialogSys.chosen >= 0)     /// Choose a branch
    //         if (parent.get_pointer().touch >= 1.f) {
    //             //dialogSys.FetchNextLine(db);
    //             parent.get_pointer().touch = 0.f;
    //             parent.get_pointer().pressed = 0;
    //         }
    // }
    // else if (parent.game_mode == 1)       /// Non-dialog mode
    // {
    //     if (player.possesed)
    //     switch (player.action) {
    //         case Player::run:
    //         case Player::move:
    //             if (parent.get_pointer().pressed) {
    //                 float dir = atan2f(parent.height * .5462f - parent.get_pointer().pos.y,
    //                                    parent.get_pointer().pos.x - parent.width * .5f);
    //                 player.possesed->add_motion(dir, Player::run == player.action ? .54f + .2f * fabsf(cosf(dir)) : .25f + .1f * fabsf(cosf(dir)));
    //             }
    //             else player.action = Player::none;
    //             break;
    //         case Player::look:
    //             if (!parent.get_pointer().pressed) {
    //                 if (Database::instance->mMeta.hl != nullptr) {
    //                     if (Database::instance->mMeta.reachable) {
    //                         float dir = atan2f(Database::instance->mMeta.hl->mY - player.possesed->mY,
    //                                            player.possesed->mX - Database::instance->mMeta.hl->mX);
    //                         Database::instance->mMeta.hl->mFacing =
    //                         static_cast<uint8_t>(
    //                                 ((383
    //                                   - static_cast<int>(dir * 180.f * F_1_PI)
    //                                  ) / 45 + 2) % 8);
    //                         player.possesed->mFacing = (Database::instance->mMeta.hl->mFacing + 4) % 8;
    //                         parent.game_mode = 0;
    //                         dialogSys.Clear();
    //                         dialogSys.next_line = Database::instance->get_vn_line("jacobtest");
    //                         dialogSys.FetchNextLine(*Database::instance);
    //                         dialogSys.bg_transition = .5f;
    //                     }
    //                     Database::instance->mMeta.hl = nullptr;
    //                 }
    //                 player.action = Player::none;
    //             }
    //             break;
    //         default:
    //             if (parent.get_pointer().single_press)
    //                 if (sqrtf(powf((parent.height * .5462f - parent.get_pointer().pos.y) * 2.f, 2) + powf(parent.width * .5f - parent.get_pointer().pos.x, 2)) < 100) {
    //                     player.action = Player::look;
    //                     Database::instance->mMeta.hl = nullptr;
    //                 }
    //                 else if (parent.get_pointer().double_tap)
    //                     player.action = Player::run;
    //                 else
    //                     player.action = Player::move;
    //     }
    // }
    light_lock.lock();
    // if (worker_destruction_flag) {
    //     worker_destruction_flag = false;
    //     worker_thread.reset(new ::thread(PlayRoomWorker, this));
    // }
    return true;
}

// static bool is_reachable(const object &o, const object &n) {
//     return sqrt(violet::sqr(o.pos.x - n.pos.x) + violet::sqr(o.pos.y - n.pos.y) * 1.2f) < TILE_WIDTH * 1.2f;
// }

void red_room::post_step(::overlay& parent)
{
    if (game_mode == 1) {
        if (player.possesed) {
            using enum_t = player_t::_action_enum_;
            point_t dest(parent.gl.draw_size.x * .5f, parent.gl.draw_size.y * .5462f);
            if (player.action == enum_t::look) {
                const float dist = std::min(dest ^ parent.get_pointer().pos, parent.gl.draw_size.y * .16f) * 3,
                        dir = atan2f(parent.get_pointer().pos.y - dest.y, dest.x - parent.get_pointer().pos.x);
                dest.y *= 1.245f;
                dest.x += cosf(dir) * dist;
                dest.y -= sinf(dir) * dist;
            }
            dest -= player.possesed->pos;
            camera_pos.x += (dest.x - camera_pos.x) * .0386f;
            camera_pos.y += (dest.y - camera_pos.y) * .0386f;
            camera_scale += ((player.action == enum_t::look ? .82f : .4f) - camera_scale) * .026f;
            // if (player.action == enum_t::look) {
            //     static int find_delay = 0;
            //     if (++find_delay > 8) {
            //         Database::instance->mMeta.hl = nullptr;
            //         const auto ppreal = (parent.mCamera % Database::instance->projectionMatrix % mat4x4_t::translate(1, -1) % mat4x4_t::scale(RoomWidth * .5f, RoomHeight * -.5f)).Invert() * parent.get_pointer().pos;
            //         for (auto f = parent.GetFirstObject(); f != nullptr; f = f->next_obj_ptr)
            //             if (f->mClickable)
            //                 switch (f->mClickable) {
            //                     case 1:
            //                         if ((point_t(f->mX, f->mY - 50) ^ ppreal) < 100)
            //                             Database::instance->mMeta.hl = f;
            //                         break;
            //                     case 2:
            //                         if (IsInside(f->mX - 65, f->mY - 250, f->mX + 65, f->mY + 15, ppreal))
            //                             Database::instance->mMeta.hl = f;
            //                         break;
            //                 }
            //         if (Database::instance->mMeta.hl != nullptr)
            //             Database::instance->mMeta.reachable = is_reachable(*Database::instance->mMeta.hl, *player.possesed);
            //         find_delay = 0;
            //     }
            // }
        }
    }
    camera = mat4x4_t::translate(camera_pos) *
            mat4x4_t::scale(camera_scale, camera_scale, point_t(parent.gl.draw_size.x * .5f, parent.gl.draw_size.y * .5f));

    if (--darkness_update_timer < 0) {
        // if (parent.db.current_darkness != nullptr)
        //     glDeleteTextures(1, &DATAB->current_darkness->i);
        // else
        //     DATAB->current_darkness = new image_t(0, DARKNESS_TEXTURE_SIZE, DARKNESS_TEXTURE_SIZE, DARKNESS_TEXTURE_SIZE, DARKNESS_TEXTURE_SIZE);
        // glGenTextures(1, &DATAB->current_darkness->i);
        // glBindTexture(GL_TEXTURE_2D, DATAB->current_darkness->i);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST = no smoothing
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_LINEAR = smoothing
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, DARKNESS_TEXTURE_SIZE, DARKNESS_TEXTURE_SIZE, 0,
        //              GL_ALPHA, GL_UNSIGNED_BYTE, *darkness_data.m);

        // glBindTexture(GL_TEXTURE_2D, 0);
        darkness_update_timer = 1;
    }
    light_lock.unlock();
}

void red_room::draw(const graphics::core& gl) const
{
    gl.pnormal.use();
    gl.pnormal.set_view_transform(camera);
    gl.pnormal.set_identity();
    gl.pnormal.set_color(1, 1, 1, 1);
    gl.pfill.use();
    gl.pfill.set_view_transform(camera);
    gl.pfill.set_identity();
    gl.pfill.set_color(1, 1, 1, 1);
}

void red_room::post_draw(const graphics::core& gl) const
{
    const float fadebox[] = {
            0, 0,  static_cast<float>(gl.draw_size.x), 0,
            0, static_cast<float>(gl.draw_size.y), static_cast<float>(gl.draw_size.x), static_cast<float>(gl.draw_size.y)
    };
    gl.pnormal.use();
    gl.pnormal.set_identity();
//     if (DATAB->current_darkness) {
//         SetColor(color_t::Greyscale(0.15f, darkness));
//         glActiveTexture(GL_TEXTURE0);
//         glBindTexture(GL_TEXTURE_2D, DATAB->current_darkness->i);
//         glUniform1i(Database::instance->mTextureHandle, 0);
//         glVertexAttribPointer(Database::instance->position_handle, 2, GL_FLOAT, GL_FALSE, 0, fadebox);
//         glVertexAttribPointer(Database::instance->texture_position_handle, 2, GL_FLOAT, GL_FALSE, 0, squareVerts);
//         glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//     }
//     glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, 1);
//     if (dialogSys.bg_transition < 1.f) {
//         if (dialogSys.bg_prev)
//         {
//             SetTransform(dialogSys.mat_bg_prev);
//             if (dialogSys.bg == nullptr && dialogSys.bg_transition > .5f)
//                 glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, 1.f - (dialogSys.bg_transition -.5f) * 2.f);
//             else
//                 glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, 1);
//             draw_image(*dialogSys.bg_prev);
//         }
//         if (dialogSys.bg)
//         {
//             SetTransform(dialogSys.mat_bg);
//             glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, dialogSys.bg_transition * dialogSys.bg_transition);
//             draw_image(*dialogSys.bg);
//         }
//     } else {
//         if (dialogSys.bg)
//         {
//             SetTransform(dialogSys.mat_bg);
//             glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, 1);
//             draw_image(*dialogSys.bg);
//         }
//     }

//     if (parent.game_mode == 1) {
//         glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, 1);
//         char txt[128];
//         auto pos = world.GetPos(player.possesed->mX, player.possesed->mY);
//         snprintf(txt, 128, "Current tile: %ld, %ld", pos.x, pos.y);
//         draw_text(txt, 5, 5, 24);
//         if (player.action == Player::look && Database::instance->mMeta.hl && !Database::instance->mMeta.reachable) {
//             point_t(Database::instance->mMeta.hl->mX, Database::instance->mMeta.hl->mY - 100);
//             if (pt.x < 70) pt.x = 70;
//             else if (pt.x > RoomWidth - 70) pt.x = RoomWidth - 70;
//             pt.y = sinf(((pt.y / (RoomHeight *.5f)) - 1) * F_TAU_4) * 200;
//             if (pt.y < 30) pt.y = 30;
//             else if (pt.y > RoomHeight - 30) pt.y = RoomHeight - 30;
//             glVertexAttrib4f(DATAB->color_handle, 0, 0, 0, .6f);
//             SetIdentity();
//             fill_round_rectangle(rect_t(pt.x - 70, pt.y - 20, pt.x + 70, pt.y + 20), 12);
//             glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, .8f);
//             draw_text("ZA DALEKO", pt, 32, TextAlign::Center, TextAlign::Center);
//             glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, 1);
//         }
//     }
//     else if (parent.game_mode == 0) {

//         for (auto &&it : dialogSys.port)
//         {
//             if (it.transition < 1.f)
//             {
//                 if (it.id_prev > 0)
//                 {
//                     SetTransform(it.mat_prev * mat4x4_t::translate(0, it.transition * it.transition * 150.f));
//                     glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, 1.f - it.transition);
//                     draw_sprite(DATAB->portraits[it.id_prev - 1]);
//                 }
//                 if (it.id > 0 && dialogSys.bg_transition >= 1.f)
//                 {
//                     SetTransform(it.mat * mat4x4_t::translate(0, powf(1.f - it.transition, 2) * 150.f));
//                     glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, it.transition);
//                     draw_sprite(DATAB->portraits[it.id - 1]);
//                 }
//             }
//             else if (dialogSys.bg_transition < 1.f)
//             {
//                 if (it.id > 0)
//                 {
//                     float tr = fabsf(dialogSys.bg_transition - .5f) * 2.f;
//                     SetTransform(it.mat);
//                     glVertexAttrib4f(DATAB->color_handle, 1.f, 1.f, 1.f, powf(tr, 4));
//                     draw_sprite(DATAB->portraits[it.id - 1]);
//                 }
//             }
//             else if (it.id > 0)
//             {
//                 SetTransform(it.mat);
//                 glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, 1);
//                 if (it.id <= DATAB->portraits.size())
//                     draw_sprite(DATAB->portraits[it.id - 1]);
//             }
//         }
//         if (dialogSys.bg_transition >= 1.f)
//         {
//             SetIdentity();
//             if (dialogSys.choices)
//             {
//                 float boxes_height = dialogSys.choices->size() * DIALOG_CHOICE_PADDING;
//                 float hh = (RoomHeight - boxes_height) / 2.f;
//                 if (DATAB->image_id_fade) {
//                     glVertexAttrib4f(DATAB->color_handle, 0, 0, 0, 1);
//                     glActiveTexture(GL_TEXTURE0);
//                     glBindTexture(GL_TEXTURE_2D, DATAB->image_id_fade);
//                     glUniform1i(Database::instance->mTextureHandle, 0);
//                     glVertexAttribPointer(Database::instance->position_handle, 2, GL_FLOAT, GL_FALSE, 0, fadebox);
//                     glVertexAttribPointer(Database::instance->texture_position_handle, 2, GL_FLOAT, GL_FALSE, 0, squareVerts);
//                     glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//                 }
//                 for (unsigned i = 0; i < dialogSys.choices->size(); ++i, hh += DIALOG_CHOICE_PADDING)
//                 {
//                     SetIdentity();
//                     glVertexAttrib4f(DATAB->color_handle, .05f, .05f, .05f, .8f);
//                     rect_t rect(0, hh, RoomWidth, hh + DIALOG_CHOICE_PADDING - 10);
//                     fill_rectangle(rect);
//                     if (i == dialogSys.chosen && parent.get_pointer().touch > 0.06f) {
//                         GOLD_BRUSH; // GOLD
//                         draw_text(DATAB->text.c_str() + (*dialogSys.choices)[i].first, rect, FONTSIZE * (.866f + .333f * parent.get_pointer().touch), Center, Center);
//                     } else {
//                         glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, 1);
//                         draw_text(DATAB->text.c_str() + (*dialogSys.choices)[i].first, rect, FONTSIZE * .866f, Center, Center);
//                     }
//                 }
//             }
//             else
//             {
//                 if (DATAB->image_id_fade) {
//                     const float rleft = RoomWidth * .033f, rtop = RoomHeight - 66.f - dialogSys.height_displayed, rright = RoomWidth * .963f, rbottom = RoomHeight + 5.f;
//                     const float textbox[] = {
//                             rleft, rtop,  rright, rtop,
//                             rleft, rbottom, rright, rbottom
//                     };
//                     glVertexAttrib4f(DATAB->color_handle, 0, 0, 0, .89f);
//                     glActiveTexture(GL_TEXTURE0);
//                     glBindTexture(GL_TEXTURE_2D, DATAB->image_id_fade);
//                     glUniform1i(Database::instance->mTextureHandle, 0);
//                     glVertexAttribPointer(Database::instance->position_handle, 2, GL_FLOAT, GL_FALSE, 0, textbox);
//                     glVertexAttribPointer(Database::instance->texture_position_handle, 2, GL_FLOAT, GL_FALSE, 0, squareVerts);
//                     glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//                 }

//                 static const color_t color_text_shadow{ 0, 0, 0, .8f }, color_text{ 1, 1, 1, 1 };

//                 //glVertexAttrib4f(DATAB->color_handle, 0, 0, 0, .8f);
//                 DrawText_CustomAnimation1(dialogSys.text.c_str(), dialogSys.current_text, RoomWidth * .052f + .66f, RoomHeight - 8.f - dialogSys.height_displayed + .66f, FONTSIZE, color_text_shadow, dialogSys.animation);
//                 //glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, 1);
//                 DrawText_CustomAnimation1(dialogSys.text.c_str(), dialogSys.current_text, RoomWidth * .052f, RoomHeight - 8.f - dialogSys.height_displayed, FONTSIZE, color_text, dialogSys.animation);
//                 if (dialogSys.current_name_displayed)
//                 {
//                     const rect_t name_display(RoomWidth * .08f, RoomHeight - 55.f - dialogSys.height_displayed, RoomWidth * .08f + 245.f, RoomHeight - 20.f - dialogSys.height_displayed);
//                     SetIdentity();
//                     glVertexAttrib4f(DATAB->color_handle, 0, 0, 0, .5f);
//                     fill_round_rectangle(name_display, fabsf(name_display.bottom - name_display.top) * .4f); // Make rounded with 5.f radius
//                     glVertexAttrib4f(DATAB->color_handle, 1, 1, .9f, 1);
//                     draw_text(dialogSys.current_name_displayed, name_display, FONTSIZE, Center, Center);
//                 }
//                 if (dialogSys.text_done)
//                 {
//                     float alph = fabsf(dialogSys.indicator_anim);
//                     SetTransform(mat4x4_t::scale(1, 1) * mat4x4_t::translate(RoomWidth * .899f, RoomHeight - 10.f + alph * 3.f));
//                     glVertexAttrib4f(DATAB->color_handle, 1, 1, 1, alph / 2.f + .1f);
//                     const float indicator[] = { -10, -5, 10, -5, 0, 5 };

//                     glUniform1i(Database::instance->mUseTextures, 0);
//                     glVertexAttribPointer(Database::instance->position_handle, 2, GL_FLOAT, GL_FALSE, 0, indicator);
//                     glDrawArrays(GL_TRIANGLES, 0, 3);
//                     glUniform1i(Database::instance->mUseTextures, 1);
//                 }
//             }
//         }
// //       else
// //       {
// //           FLOAT tr = fabsf(my_vn.bg_transition - .5f) * 2.f;
// //           SetTransform(mat4x4_t::scale(2.f, 2.f));
// //           glVertexAttrib4f(DATAB->color_handle, 0, 0, 0, tr < .8f ? 0 : (tr - .8f) * 4.f);
// //           fill_rectangle(text_field);
// //       }
//     }

    // parent.gl.set_identity();
    // parent.gl.set_color(1, 1, 1, 1);
}

void red_room::_dialog_system_type_::build_bg_matrix(float _RWidth, float _RHeight, float imgw, float imgh)
{
    const float sc = std::max(_RWidth / imgw, _RHeight / imgh);
    mat_bg = mat4x4_t::translate((_RWidth - imgw) / 2, (_RHeight - imgh) / 2) * mat4x4_t::scale(sc, sc, {_RWidth / 2.f, _RHeight / 2.f});
}

}  // namespace idle

