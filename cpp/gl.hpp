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

#include <atomic>
#include <memory>
#include <optional>
#include <math.hpp>
#include <log.hpp>

#include "platform/pointer.hpp"
#include "platform/opengl_core_adaptive.hpp"
#include "gl_programs.hpp"
#include "fonts.hpp"

namespace graphics
{

constexpr idle::color_t black{0, 0, 0, 1};

using buffer_size = math::point2<unsigned>;

bool assert_opengl_errors() noexcept;

struct render_buffer_t
{
    GLuint buffer_frame = 0, texture = 0, buffer_depth = 0;
    GLfloat texture_w, texture_h;
    buffer_size internal_size;

    render_buffer_t(buffer_size size, GLint quality) noexcept;

    render_buffer_t(render_buffer_t&&)=delete;

    render_buffer_t(const render_buffer_t&)=delete;

    ~render_buffer_t() noexcept;
};

struct render_program_t
{
    GLuint program = 0;

private:
    GLuint texture_position_handle = 0, position_handle = 0;

public:
    void draw_buffer(const render_buffer_t& source) const noexcept;

    void use() const noexcept;

    void prepare() noexcept;
};

struct blur_render_program_t : render_program_t
{
private:
    GLint direction_handle = 0, resolution_handle = 0, radius_handle = 0;

public:
    void prepare() noexcept;

    void set_radius(GLfloat x) const noexcept;

    void set_resolution(GLfloat x) const noexcept;

    void set_direction(GLfloat x, GLfloat y) const noexcept;
};

struct masked_render_program_t : render_program_t
{
private:
    GLint mask_offset_handle = 0;

public:
    void prepare() noexcept;

    void set_offsets(GLfloat ratio1, GLfloat ratio2, GLfloat buffer_height, GLfloat subbuffer_width) const noexcept;
};

struct core
{
    struct program_container_t
    {
        render_program_t render_final;
        masked_render_program_t render_masked;
        blur_render_program_t render_blur;

        textured_program_t normal;
        program_t fill;
        double_vertex_program_t double_normal;
        double_solid_program_t double_fill;
        text_program_t text;
        fullbg_program_t fullbg;
        noise_program_t noise;
        gradient_program_t gradient;

    } prog;

    GLint render_quality = gl::LINEAR;
    std::unique_ptr<const render_buffer_t> render_buffer_masked;
    idle::point_t draw_size{0, 0};
    buffer_size screen_size{0, 0}, viewport_size{0, 0};
    math::point2<GLfloat> translate_vector;

    std::array<GLfloat, 8> draw_bounds_verts;

    struct
    {
        std::optional<fonts::font_t> regular, title;

    } fonts;

    static constexpr std::array<float, 8> texture_bounds_verts
    {
        0, 0,
        1, 0,
        0, 1,
        1, 1
    };

    void copy_projection_matrix(const idle::mat4x4_noopt_t&) const noexcept;

    bool setup_graphics() noexcept;

    bool resize(buffer_size window_size) noexcept;

    std::unique_ptr<const render_buffer_t> new_render_buffer(unsigned divider = 1) const noexcept;

    void clean() noexcept;

    void view_normal() const noexcept;

    void view_mask() const noexcept;

    void view_distortion() const noexcept;
};


} // namespace graphics
