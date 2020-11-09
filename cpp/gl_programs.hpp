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
#include <math_defines.hpp>

namespace graphics
{

struct unique_texture
{
private:
    GLuint value;

public:
    unique_texture(GLuint) noexcept;

    unique_texture(const unique_texture&) = delete;
    unique_texture& operator=(const unique_texture&) = delete;

    unique_texture(unique_texture&&) noexcept;
    unique_texture& operator=(unique_texture&&) noexcept;

    ~unique_texture() noexcept;

    GLuint get() const noexcept;
};

struct program_t
{
    GLuint program_id = 0;

private:
    GLuint position_handle = 0;
    GLint model_handle = 0,
          view_handle = 0,
          color_handle = 0;

public:
    void set_transform(const idle::mat4x4_t& f) const noexcept;

    void set_transform(const idle::mat4x4_noopt_t& f) const noexcept;

    void set_identity(void) const noexcept;

    void set_view_transform(const idle::mat4x4_t& f) const noexcept;

    void set_view_transform(const idle::mat4x4_noopt_t& f) const noexcept;

    void set_view_identity(void) const noexcept;

    void set_color(const idle::color_t& c) const noexcept;

    void set_color(const idle::color_t& c, float custom_alpha) const noexcept;

    void position_vertex(const GLfloat *f) const noexcept;

    void use() const noexcept;

    void prepare() noexcept;
};

struct textured_program_t : program_t
{
private:
    GLuint texture_position_handle = 0;

public:
    void texture_vertex(const GLfloat *f) const noexcept;

    void prepare() noexcept;
};

struct noise_program_t : textured_program_t
{
private:
    GLint secondary_color_handle = 0,
          tertiary_color_handle = 0,
          quaternary_color_handle = 0,
          noise_seed_handle = 0;

public:
    void prepare() noexcept;

    void set_secondary_color(const idle::color_t& c) const noexcept;

    void set_tertiary_color(const idle::color_t& c) const noexcept;

    void set_quaternary_color(const idle::color_t& c) const noexcept;

    void set_seed(idle::point_t seed) const noexcept;
};

struct gradient_program_t : program_t
{
private:
    GLuint interpolation_handle = 0;
    GLint secondary_color_handle = 0;

public:
    void prepare() noexcept;

    void set_secondary_color(const idle::color_t& c) const noexcept;

    void set_secondary_color(const idle::color_t& c, float alpha) const noexcept;

    void interpolation_vertex(const GLfloat *f) const noexcept;
};

struct double_base_program_t
{
private:
    GLuint destination_handle = 0;
    GLint interpolation_handle = 0;

public:
    void destination_vertex(const GLfloat *f) const noexcept;

    void set_interpolation(GLfloat x) const noexcept;

    void prepare_headless(GLuint prog) noexcept;
};

struct double_solid_program_t : program_t, double_base_program_t
{
    void prepare() noexcept;
};

struct double_vertex_program_t : textured_program_t, double_base_program_t
{
    void prepare() noexcept;
};

struct text_program_t : textured_program_t
{
private:
    GLint font_offset_handle = 0;

public:
    void set_text_offset(idle::point_t offset) const noexcept;

    void prepare() noexcept;
};

struct fullbg_program_t : program_t
{
private:
    GLuint offset_handle = 0, resolution_handle = 0;

public:
    void set_offset(GLfloat x) const noexcept;

    void set_resolution(idle::point_t res) const noexcept;

    void prepare() noexcept;
};

}
