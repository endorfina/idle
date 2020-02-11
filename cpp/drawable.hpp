/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of the Idle.

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
#include <memory>
#include <math.hpp>
#include "gl_programs.hpp"

namespace idle
{

class image_t
{
    point_t tex;
    GLuint i;
    size_t width, height;

    friend struct sprite_t;

protected:
    image_t();

    image_t(GLuint id, size_t w, size_t h);

    image_t(GLuint id, size_t w, size_t h, size_t u2, size_t v2);

public:
    ~image_t();

    image_t(const image_t&) = delete;

    image_t(image_t&&);

    static image_t load_from_assets_immediate(const char * fn, GLint quality=gl::LINEAR);

    static image_t load_from_assets(const char * fn, GLint quality=gl::LINEAR);

    static image_t load_from_memory(GLsizei w, GLsizei h,
        GLint i, GLenum f,
        GLint q, GLint r,
        std::unique_ptr<unsigned char[]> pix);

    static void load_topmost_queued_picture();

    void draw(const graphics::textured_program_t& prog) const;

    void draw(const graphics::textured_program_t& prog, point_t p) const;

    void draw(const graphics::textured_program_t& prog, point_t p, const rect_t &rect) const;

    void draw(const graphics::textured_program_t& prog, const rect_t &rect) const;

    GLuint get_gl_id() const;

    GLuint release();

    template<typename Ret>
    math::point2<Ret> get_size() const {
        return { static_cast<Ret>(width), static_cast<Ret>(height) };
    }
};

struct sprite_t {
    const GLuint image;
    rect_t rect;
    point_t center, size, pixel;

    sprite_t(GLuint i, const rect_t &r, point_t p, point_t s, point_t tx)
            : image(i), rect(r), center(p), size(s), pixel(tx) {}

    void draw(const graphics::textured_program_t& prog) const;

    void draw(const graphics::textured_program_t& prog, point_t p) const;

    static void add_subsprite(std::vector<sprite_t> &out, const image_t& image, unsigned x, unsigned y, unsigned w, unsigned h, point_t c, point_t o);
};

void fill_rectangle(const graphics::program_t& prog, const rect_t &rect);
void fill_rectangle(const graphics::program_t& prog, point_t rect);

void draw_rectangle(const graphics::program_t& prog, const rect_t &rect);
void draw_rectangle(const graphics::program_t& prog, point_t rect);

void fill_round_rectangle(const graphics::program_t& prog, const rect_t &rect, const float radius);

void fill_circle(const graphics::program_t& prog, point_t center, const float radius, const unsigned int steps = 24);

void draw_circle(const graphics::program_t& prog, point_t center, const float radius, const unsigned int steps = 24);

}  // namespace idle

