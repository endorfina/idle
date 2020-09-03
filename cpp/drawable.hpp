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

#include <memory>
#include <math.hpp>
#include "gl_programs.hpp"
#include "gl.hpp"

namespace idle
{

class image_t
{
    point_t tex;
    GLuint i;
    size_t width, height;

protected:
    image_t() noexcept;

    image_t(GLuint id, size_t w, size_t h) noexcept;

    image_t(GLuint id, size_t w, size_t h, size_t u2, size_t v2) noexcept;

public:
    ~image_t() noexcept;

    image_t(const image_t&) = delete;

    image_t(image_t&&) noexcept;

    static image_t load_from_assets_immediate(const char * fn, GLint quality=gl::LINEAR) noexcept;

    static image_t load_from_assets(const char * fn, GLint quality=gl::LINEAR) noexcept;

    static image_t load_from_memory(GLsizei w, GLsizei h,
        GLint i, GLenum f,
        GLint q, GLint r,
        std::unique_ptr<unsigned char[]> pix) noexcept;

    static void load_topmost_queued_picture() noexcept;

    void draw(const graphics::textured_program_t& prog) const noexcept;

    void draw(const graphics::textured_program_t& prog, point_t p) const noexcept;

    void draw(const graphics::textured_program_t& prog, point_t p, const rect_t &rect) const noexcept;

    void draw(const graphics::textured_program_t& prog, const rect_t &rect) const noexcept;

    GLuint get_gl_id() const noexcept;

    GLuint release() noexcept;

    template<typename Ret>
    math::point2<Ret> get_size() const {
        return { static_cast<Ret>(width), static_cast<Ret>(height) };
    }
};

void fill_screen(const graphics::core& gl, const graphics::program_t& prog) noexcept;

void fill_rectangle(const graphics::program_t& prog, const rect_t &rect) noexcept;
void fill_rectangle(const graphics::program_t& prog, point_t rect) noexcept;

void draw_rectangle(const graphics::program_t& prog, const rect_t &rect) noexcept;
void draw_rectangle(const graphics::program_t& prog, point_t rect) noexcept;

void fill_round_rectangle(const graphics::program_t& prog, const rect_t &rect, const float radius) noexcept;

void fill_circle(const graphics::program_t& prog, point_t center, const float radius, const unsigned int steps = 24) noexcept;

void draw_circle(const graphics::program_t& prog, point_t center, const float radius, const unsigned int steps = 24) noexcept;

}  // namespace idle

