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

#include <math.hpp>
#include "platform/opengl_core_adaptive.hpp"

#define TYPE_REMOVE_CVR(x) std::remove_cv_t<std::remove_reference_t<decltype(x)>>

namespace idle
{
using color_t = math::color<GLfloat>;
using point_t = math::point2<GLfloat>;
using point_3d_t = math::point3<GLfloat>;
using mat4x4_t = math::matrix4x4<GLfloat, false>;
using mat4x4_noopt_t = math::matrix4x4<GLfloat, true>;
using rect_t = math::rect<GLfloat>;

enum class TextAlign { Near, Center, Far };

constexpr float square_coordinates[8] = {
    0, 0, 1, 0,
    0, 1, 1, 1
};

struct text_animation_data
{
    float scale, rotation;
};

}  // namespace idle

namespace graphics
{

struct unique_texture
{
    GLuint value;

    unique_texture(GLuint);

    unique_texture(const unique_texture&) = delete;
    unique_texture& operator=(const unique_texture&) = delete;

    unique_texture(unique_texture&&);
    unique_texture& operator=(unique_texture&&);

    ~unique_texture();
};

struct program_t
{
    GLuint pid = 0;
    GLuint position_handle = 0;
    GLint model_handle = 0, view_handle = 0, color_handle = 0;

    void set_transform(const idle::mat4x4_t& f) const;

    void set_transform(const idle::mat4x4_noopt_t& f) const;

    void set_identity(void) const;

    void set_view_transform(const idle::mat4x4_t& f) const;

    void set_view_transform(const idle::mat4x4_noopt_t& f) const;

    void set_view_identity(void) const;

    void set_color(const idle::color_t& c) const;

    void set_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.f) const;

    void set_color(const idle::color_t& c, float custom_alpha) const;

    void position_vertex(const GLfloat *f) const;

    void use() const;

    void collect_variables();

    void prepare() const;
};

struct textured_program_t : program_t
{
    GLuint texture_position_handle = 0;

    void texture_vertex(const GLfloat *f) const;

    void collect_variables();

    void prepare() const;
};

struct double_vertex_program_t : textured_program_t
{
    GLuint destination_handle = 0;
    GLint interpolation_handle = 0;

    void destination_vertex(const GLfloat *f) const;

    void set_interpolation(const GLfloat x) const;

    void collect_variables();

    void prepare() const;
};

struct text_program_t : textured_program_t
{
    GLint font_offset_handle = 0;

    void set_text_offset(const GLfloat x, const GLfloat y) const;

    void collect_variables();
};

struct fullbg_program_t : program_t
{
    GLuint offset_handle = 0, resolution_handle = 0;

    void set_offset(const GLfloat x) const;

    void set_resolution(const GLfloat w, const GLfloat h) const;

    void collect_variables();
};

}
