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

#include <vector>
#include <zlib.hpp>
#include <math.hpp>
#include "gl.hpp"
#include <cstring>
#include <embedded_shaders.hpp>

// void printGLString(const char *name, GLenum s)
// {
//     LOGI("GL %s = %s\n", name, reinterpret_cast<const char *>(gl::GetString(s)));
// }

namespace graphics
{
namespace
{

#ifndef NDEBUG
void detailed_report_opengl_errors(const char* op, const char *const file, const int line) noexcept
{
    if (assert_opengl_errors())
    {
        LOGE("Reported \'%s\' at %s:%i", op, file, line);
    }
}

#define report_opengl_errors(what) detailed_report_opengl_errors((what), __FILE__, __LINE__)
#else
#define report_opengl_errors(x) ((void)0)
#endif

GLuint load_shader(const GLenum shaderType, const char* const pSource) noexcept
{
    if (const GLuint shader = gl::CreateShader(shaderType))
    {
        gl::ShaderSource(shader, 1, &pSource, nullptr);
        gl::CompileShader(shader);
        GLint compiled = 0;
        gl::GetShaderiv(shader, gl::COMPILE_STATUS, &compiled);

        if (compiled)
        {
            return shader;
        }
        else
        {
            GLint infoLen = 0;
            gl::GetShaderiv(shader, gl::INFO_LOG_LENGTH, &infoLen);

            if (infoLen)
            {
                char buf[infoLen];
                gl::GetShaderInfoLog(shader, infoLen, nullptr, buf);
                LOGE("Could not compile shader %d:\n%s", shaderType, buf);
            }
            gl::DeleteShader(shader);
        }
    }
    return 0;
}

GLuint create_program(const char* const pVertexSource, const char* const pFragmentSource) noexcept
{
    const GLuint vertexShader = load_shader(gl::VERTEX_SHADER, pVertexSource);
    if (!vertexShader)
        return 0;

    const GLuint pixelShader = load_shader(gl::FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader)
        return 0;

    if (const GLuint program = gl::CreateProgram())
    {
        gl::AttachShader(program, vertexShader);
        gl::AttachShader(program, pixelShader);
        gl::LinkProgram(program);
        GLint linkStatus = gl::FALSE_;
        gl::GetProgramiv(program, gl::LINK_STATUS, &linkStatus);

        if (linkStatus != gl::TRUE_)
        {
            GLint bufLength = 0;
            gl::GetProgramiv(program, gl::INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char buf[bufLength];
                gl::GetProgramInfoLog(program, bufLength, nullptr, buf);
                LOGE("Could not link program:\n%s", buf);
            }
            gl::DeleteProgram(program);
            return 0;
        }
        return program;
    }
    return 0;
}

template<std::size_t Size> // shaders::source_info::size_uncompressed
class shader_compiler
{
    using buffer_type = std::array<char, Size>;

    bool failed;
    buffer_type buffer;

public:
    shader_compiler(const std::string_view view) noexcept
        : failed(!decompress(view))
    {
    }

private:
    const char * data(const unsigned int addr) const noexcept
    {
        return &buffer[addr];
    }

    bool decompress(const std::string_view source) noexcept
    {
        if (const auto buf = idle::zlib<std::vector<unsigned char>>(source, false, true);
                    buf && buf->size() == Size)
        {
            ::memcpy(buffer.data(), buf->data(), Size);
            return true;
        }
        return false;
    }

public:
    GLuint compile(const unsigned int v, const unsigned int f) noexcept
    {
        if (!has_failed())
        {
            if (const auto r = create_program(data(v), data(f)))
            {
                return r;
            }

            failed = true;
        }
        return 0;
    }

    bool has_failed() const noexcept
    {
        return failed;
    }
};

template<typename Call>
void apply_to_all(core::program_container_t& con, const Call& call) noexcept
{
    call(con.render_final);
    call(con.render_masked);
    call(con.render_blur);

    call(con.normal);
    call(con.fill);
    call(con.double_normal);
    call(con.double_fill);
    call(con.text);
    call(con.fullbg);
    call(con.noise);
    call(con.gradient);
}

bool compile_shaders(core::program_container_t& prog) noexcept
{
    using source = shaders::source_info;
    shader_compiler<source::size_uncompressed> sc{ shaders::get_view() };

    prog.render_final.program = sc.compile(source::pos_renderv, source::pos_renderf);
    prog.render_masked.program = sc.compile(source::pos_renderv, source::pos_maskedf);
    prog.render_blur.program = sc.compile(source::pos_renderv, source::pos_simpleblurf);

    prog.normal.program_id = sc.compile(source::pos_normv, source::pos_normf);
    prog.double_normal.program_id = sc.compile(source::pos_doublenormv, source::pos_normf);
    prog.double_fill.program_id = sc.compile(source::pos_doublesolidv, source::pos_solidf);
    prog.fill.program_id = sc.compile(source::pos_solidv, source::pos_solidf);
    prog.text.program_id = sc.compile(source::pos_textv, source::pos_textf);
    prog.fullbg.program_id = sc.compile(source::pos_solidv, source::pos_fullbgf);
    prog.noise.program_id = sc.compile(source::pos_normv, source::pos_noisef);
    prog.gradient.program_id = sc.compile(source::pos_gradientv, source::pos_gradientf);


    if (sc.has_failed())
    {
        LOGE("Shader compilation failed.\n" "How unfortunate.");
        return false;
    }
    return true;
}

template<typename...Progs>
bool programs_are_functional(const Progs& ... progs) noexcept
{
    return (true && ... && !!progs.program_id);
}

buffer_size appropriate_size(const buffer_size size) noexcept
{
    buffer_size u{ 64, 64 };
    while (u.x < size.x) u.x *= 2;
    while (u.y < size.y) u.y *= 2;
    return u;
}

GLuint load_attribute(const GLuint program, const char* const variable_name) noexcept
{
    const auto handle = static_cast<GLuint>(gl::GetAttribLocation(program, variable_name));
    gl::EnableVertexAttribArray(handle);
    report_opengl_errors(variable_name);
    return handle;
}

GLint load_uniform(const GLuint program, const char* const variable_name) noexcept
{
    const auto handle = gl::GetUniformLocation(program, variable_name);
    report_opengl_errors(variable_name);
    return handle;
}

}  // namespace

bool core::setup_graphics() noexcept
{
    gl::Disable(gl::CULL_FACE);
    gl::Disable(gl::DEPTH_TEST);
    gl::BlendFuncSeparate(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA, gl::ONE, gl::ONE_MINUS_SRC_ALPHA);
    gl::Enable(gl::BLEND);

    if (!compile_shaders(prog))
    {
        return false;
    }

    apply_to_all(prog, [] (auto& program) { program.prepare(); });

#if LOG_LEVEL > 3
    // Check openGL on the system
    constexpr std::pair<GLenum, const char *> opengl_info[] {
        { gl::VENDOR, "Vendor" },
        { gl::RENDERER, "Renderer" },
        { gl::VERSION, "Version" }
        //, gl::EXTENSIONS
    };

    for (const auto [name, label] : opengl_info) {
        LOGD("OpenGL %s: %s", label, gl::GetString(name));
    }
#endif
    return true;
}

bool core::resize(const buffer_size window_size) noexcept
{
    if (window_size.x * 4 / 5 < window_size.y)
    {
        LOGW("Vertical mode has been disabled");
        return false;
    }

    LOGI("Window size set to %i x %i", window_size.x, window_size.y);

    constexpr unsigned draw_height = 360;
    constexpr unsigned main_buffer_height = 720;

    draw_size = { static_cast<float>(draw_height * window_size.x / window_size.y), static_cast<float>(draw_height) };
    viewport_size = { main_buffer_height * window_size.x / window_size.y, main_buffer_height };
    screen_size = window_size;
    translate_vector = draw_size / math::point_cast<float>(window_size);

    draw_bounds_verts = {
        0, 0,
        draw_size.x, 0,
        0, draw_size.y,
        draw_size.x, draw_size.y
    };

    if (!programs_are_functional(
                prog.normal,
                prog.fill,
                prog.double_normal,
                prog.double_fill,
                prog.text,
                prog.fullbg,
                prog.noise
            )) return false;

    copy_projection_matrix(math::matrices::orthof_static<-1, 1, float>(0, draw_size.x, 0, draw_size.y));

    prog.fullbg.use();
    prog.fullbg.set_resolution(math::point_cast<float>(window_size));

    const auto actual_padded_pixel_size = appropriate_size(viewport_size);
    auto masked_size = viewport_size;
    masked_size.y *= 4;
    masked_size.y /= 3;

    render_buffer_masked = std::make_unique<render_buffer_t>(masked_size, render_quality);

    prog.render_blur.use();
    prog.render_blur.set_radius(1.f);
    prog.render_blur.set_resolution(
            static_cast<float>(viewport_size.y)
            / static_cast<float>(actual_padded_pixel_size.y)
            / draw_size.y);

    prog.render_masked.use();
    prog.render_masked.set_offsets(
            3.f / 4.f,
            1.f / 3.f,
            render_buffer_masked->texture_h * (3.f / 4.f), // y offset
            render_buffer_masked->texture_w * (1.f / 3.f) // x offset
        );

    return true;
}


void program_t::set_transform(const idle::mat4x4_t& f) const noexcept
{
    gl::UniformMatrix4fv(model_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(f));
}

void program_t::set_transform(const idle::mat4x4_noopt_t& f) const noexcept
{
    gl::UniformMatrix4fv(model_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(f));
}

static constexpr auto identity_mat = idle::mat4x4_t{};

void program_t::set_identity(void) const noexcept
{
    gl::UniformMatrix4fv(model_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(identity_mat));
}

void program_t::set_view_transform(const idle::mat4x4_t& f) const noexcept
{
    gl::UniformMatrix4fv(view_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(f));
}

void program_t::set_view_transform(const idle::mat4x4_noopt_t& f) const noexcept
{
    gl::UniformMatrix4fv(view_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(f));
}

void program_t::set_view_identity(void) const noexcept
{
    gl::UniformMatrix4fv(view_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(identity_mat));
}

void render_program_t::use() const noexcept
{
    gl::UseProgram(program);
}

void program_t::use() const noexcept
{
    gl::UseProgram(program_id);
}

void program_t::set_color(const idle::color_t& c) const noexcept
{
    gl::Uniform4f(color_handle, c.r, c.g, c.b, c.a);
}

void program_t::set_color(const idle::color_t& c, GLfloat custom_alpha) const noexcept
{
    gl::Uniform4f(color_handle, c.r, c.g, c.b, custom_alpha);
}

void program_t::position_vertex(const GLfloat *f) const noexcept
{
    gl::VertexAttribPointer(position_handle, 2, gl::FLOAT, gl::FALSE_, 0, f);
}

void textured_program_t::texture_vertex(const GLfloat *f) const noexcept
{
    gl::VertexAttribPointer(texture_position_handle, 2, gl::FLOAT, gl::FALSE_, 0, f);
}

void double_base_program_t::destination_vertex(const GLfloat *f) const noexcept
{
    gl::VertexAttribPointer(destination_handle, 2, gl::FLOAT, gl::FALSE_, 0, f);
}

void text_program_t::set_text_offset(const idle::point_t offset) const noexcept
{
    gl::Uniform2f(font_offset_handle, offset.x, offset.y);
}

void double_base_program_t::set_interpolation(const GLfloat x) const noexcept
{
    gl::Uniform1f(interpolation_handle, x);
}

void fullbg_program_t::set_offset(const GLfloat x) const noexcept
{
    gl::Uniform1f(offset_handle, x);
}

void fullbg_program_t::set_resolution(const idle::point_t res) const noexcept
{
    gl::Uniform2f(resolution_handle, res.x, res.y);
}

void noise_program_t::set_secondary_color(const idle::color_t& c) const noexcept
{
    gl::Uniform4f(secondary_color_handle, c.r, c.g, c.b, c.a);
}

void noise_program_t::set_tertiary_color(const idle::color_t& c) const noexcept
{
    gl::Uniform4f(tertiary_color_handle, c.r, c.g, c.b, c.a);
}

void noise_program_t::set_seed(const idle::point_t seed) const noexcept
{
    gl::Uniform2f(noise_seed_handle, seed.x, seed.y);
}

void gradient_program_t::set_secondary_color(const idle::color_t& c) const noexcept
{
    gl::Uniform4f(secondary_color_handle, c.r, c.g, c.b, c.a);
}

void gradient_program_t::set_secondary_color(const idle::color_t& c, const float alpha) const noexcept
{
    gl::Uniform4f(secondary_color_handle, c.r, c.g, c.b, alpha);
}

void gradient_program_t::interpolation_vertex(const GLfloat *f) const noexcept
{
    gl::VertexAttribPointer(interpolation_handle, 1, gl::FLOAT, gl::FALSE_, 0, f);
}

void blur_render_program_t::set_direction(const GLfloat x, const GLfloat y) const noexcept
{
    gl::Uniform2f(direction_handle, x, y);
}

void blur_render_program_t::set_radius(const GLfloat rad) const noexcept
{
    gl::Uniform1f(radius_handle, rad);
}

void blur_render_program_t::set_resolution(const GLfloat res) const noexcept
{
    gl::Uniform1f(resolution_handle, res);
}

void masked_render_program_t::set_offsets(const GLfloat ratio1, const GLfloat ratio2, const GLfloat buffer_height, const GLfloat subbuffer_width) const noexcept
{
    gl::Uniform4f(mask_offset_handle, ratio1, ratio2, buffer_height, subbuffer_width);
}

void render_program_t::prepare() noexcept
{
    use();
    position_handle = load_attribute(program, "attr_pos");
    texture_position_handle = load_attribute(program, "attr_mapped_vec");
}

void program_t::prepare() noexcept
{
    use();
    position_handle = load_attribute(program_id, "attr_pos");
    model_handle = load_uniform(program_id, "u_modelm");
    view_handle = load_uniform(program_id, "u_viewm");
    color_handle = load_uniform(program_id, "u_color");

    set_identity();
    set_view_identity();
    report_opengl_errors("program_t::prepare()");
}

void textured_program_t::prepare() noexcept
{
    program_t::prepare();
    texture_position_handle = load_attribute(program_id,"attr_mapped_vec");
    report_opengl_errors("textured_program_t::prepare()");
}

void noise_program_t::prepare() noexcept
{
    textured_program_t::prepare();
    secondary_color_handle = load_uniform(program_id, "u_color_2");
    tertiary_color_handle = load_uniform(program_id, "u_color_3");
    noise_seed_handle = load_uniform(program_id, "u_seed");

    report_opengl_errors("noise_program_t::prepare()");
}

void gradient_program_t::prepare() noexcept
{
    program_t::prepare();
    secondary_color_handle = load_uniform(program_id, "u_color_2");
    interpolation_handle = load_attribute(program_id,"attr_gradient");
    report_opengl_errors("gradient_program_t::prepare()");
}

void text_program_t::prepare() noexcept
{
    textured_program_t::prepare();
    font_offset_handle = load_uniform(program_id, "u_offset");
    report_opengl_errors("text_program_t::prepare()");
}

void fullbg_program_t::prepare() noexcept
{
    program_t::prepare();
    offset_handle = load_uniform(program_id, "u_speed");
    resolution_handle = load_uniform(program_id, "u_resolution");

    set_offset(0);
    report_opengl_errors("fullbg_program_t::prepare()");
}

void double_base_program_t::prepare_headless(const GLuint prog) noexcept
{
    destination_handle = load_attribute(prog,"attr_dest_pos");
    interpolation_handle = load_uniform(prog, "u_inter");

    set_interpolation(0);
    gl::EnableVertexAttribArray(destination_handle);
    report_opengl_errors("double_base_program_t::prepare()");
}

void double_solid_program_t::prepare() noexcept
{
    program_t::prepare();
    double_base_program_t::prepare_headless(program_id);
}

void double_vertex_program_t::prepare() noexcept
{
    textured_program_t::prepare();
    double_base_program_t::prepare_headless(program_id);
}

void blur_render_program_t::prepare() noexcept
{
    render_program_t::prepare();
    direction_handle = load_uniform(program, "u_direction");
    resolution_handle = load_uniform(program, "u_resolution");
    radius_handle = load_uniform(program, "u_radius");

    set_direction(1, 0);
    set_radius(1.f);
}

void masked_render_program_t::prepare() noexcept
{
    render_program_t::prepare();
    mask_offset_handle = load_uniform(program, "u_offset");
}

void render_program_t::draw_buffer(const render_buffer_t& src) const noexcept
{
    constexpr float v[]
    {
        -1.f, -1.f,  1.f, -1.f,
        -1.f, 1.f, 1.f, 1.f
    };

    const float t[]
    {
        0, 0,
        src.texture_w, 0,
        0, src.texture_h,
        src.texture_w, src.texture_h
    };

    use();

    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, src.texture);
    gl::VertexAttribPointer(position_handle, 2, gl::FLOAT, gl::FALSE_, 0, v);
    gl::VertexAttribPointer(texture_position_handle, 2, gl::FLOAT, gl::FALSE_, 0, t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}



static void set_projection_matrix(const program_t& prog, const idle::mat4x4_noopt_t& mat) noexcept
{
    prog.use();
    gl::UniformMatrix4fv(load_uniform(prog.program_id, "u_projm"), 1, gl::FALSE_, static_cast<const GLfloat*>(mat));
}

void core::copy_projection_matrix(const idle::mat4x4_noopt_t& projection_matrix) const noexcept
{
    set_projection_matrix(prog.normal, projection_matrix);
    set_projection_matrix(prog.fill, projection_matrix);
    set_projection_matrix(prog.double_normal, projection_matrix);
    set_projection_matrix(prog.double_fill, projection_matrix);
    set_projection_matrix(prog.text, projection_matrix);
    set_projection_matrix(prog.fullbg, projection_matrix);
    set_projection_matrix(prog.noise, projection_matrix);
    set_projection_matrix(prog.gradient, projection_matrix);
}

std::unique_ptr<const render_buffer_t> core::new_render_buffer(const unsigned div) const noexcept
{
    return std::make_unique<const render_buffer_t>(viewport_size / div, render_quality);
}

render_buffer_t::~render_buffer_t() noexcept
{
    LOGDD("Destroying fr%i/t%i/d%i", buffer_frame, texture, buffer_depth);
    gl::DeleteFramebuffers(1, &buffer_frame);
    gl::DeleteTextures(1, &texture);
    gl::DeleteRenderbuffers(1, &buffer_depth);
}

render_buffer_t::render_buffer_t(const buffer_size tex_size, const GLint quality) noexcept
    : internal_size(tex_size)
{
    gl::GenFramebuffers(1, &buffer_frame);
    gl::GenTextures(1, &texture);
    gl::GenRenderbuffers(1, &buffer_depth);

    report_opengl_errors("render_buffer_t::render_buffer_t");

    gl::BindTexture(gl::TEXTURE_2D, texture);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, quality);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, quality);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);

    const auto real_size = appropriate_size(tex_size);
    texture_w = static_cast<GLfloat>(tex_size.x) / static_cast<GLfloat>(real_size.x);
    texture_h = static_cast<GLfloat>(tex_size.y) / static_cast<GLfloat>(real_size.y);

    LOGDD("Creating fr%i/t%i/d%i, input{%d, %d}, tex{%.3f, %.3f}, real{%d, %d} (at least ~%.1fMB)",
            buffer_frame, texture, buffer_depth, tex_size.x, tex_size.y, texture_w, texture_h, real_size.x, real_size.y,
            real_size.x * real_size.y * sizeof(idle::color_t) / math::sqr<float>(1024) / 8.f);

    gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, real_size.x, real_size.y, 0, gl::RGBA, gl::UNSIGNED_BYTE, nullptr);
    gl::BindRenderbuffer(gl::RENDERBUFFER, buffer_depth);
    gl::RenderbufferStorage(gl::RENDERBUFFER, gl::DEPTH_COMPONENT16, real_size.x, real_size.y);
    gl::BindFramebuffer(gl::FRAMEBUFFER, buffer_frame);
    gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0, gl::TEXTURE_2D, texture, 0);
    gl::FramebufferRenderbuffer(gl::FRAMEBUFFER, gl::DEPTH_ATTACHMENT, gl::RENDERBUFFER, buffer_depth);

    report_opengl_errors("render_buffer_t::render_buffer_t");

    gl::BindTexture(gl::TEXTURE_2D, 0);
    gl::BindRenderbuffer(gl::RENDERBUFFER, 0);
    gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
}

void core::clean() noexcept
{
    fonts.regular.reset();
    fonts.title.reset();
    render_buffer_masked.reset();
}

void core::view_normal() const noexcept
{
    gl::Viewport(0, 0, viewport_size.x, viewport_size.y);
}

void core::view_mask() const noexcept
{
    gl::Viewport(0, viewport_size.y, viewport_size.x / 3, viewport_size.y / 3);
}

void core::view_distortion() const noexcept
{
    gl::Viewport(viewport_size.x / 3, viewport_size.y, viewport_size.x / 3, viewport_size.y / 3);
}

unique_texture::unique_texture(const GLuint val) noexcept : value{val}
{
}

unique_texture::unique_texture(unique_texture&& other) noexcept : value{other.value}
{
    other.value = GLuint(-1);
}

unique_texture& unique_texture::operator=(unique_texture&& other) noexcept
{
    value = other.value;
    other.value = GLuint(-1);
    return *this;
}

unique_texture::~unique_texture() noexcept
{
    if (value != GLuint(-1))
    {
        LOGDD("Destroying unique texture #%u", value);
        gl::DeleteTextures(1, &value);
    }
}

GLuint unique_texture::get() const noexcept
{
    return value;
}

bool assert_opengl_errors() noexcept
{
    if (auto error = gl::GetError(); error != gl::NO_ERROR_)
    {
        do {
            LOGE("GL Error \t0x%04x", error);
            error = gl::GetError();
        } while (error != gl::NO_ERROR_);
        return true;
    }
    return false;
}

} // namespace graphics
