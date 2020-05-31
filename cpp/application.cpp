﻿/*
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

#include <cstdio>
#include <future>
#include "application.hpp"
#include "drawable.hpp"
#include "draw_text.hpp"
#include "room_variant.hpp"
#include "room_controller.hpp"
#include "lodge.hpp"
#include "freetype_glue.hpp"
#include "config.hpp"
#include "platform/asset_access.hpp"

namespace
{

// These need to be global to allow for proper clean up with X11

idle::controller room_ctrl;
graphics::core opengl;

}  // namespace

namespace isolation
{

bool application::execute_commands(const bool is_nested)
{
    window.event_loop_back(!update_display);

    bool shutdown_was_requested = false;
    bool perform_load = false;

    if (!!window.commands.size())
    {
        [[maybe_unused]] constexpr auto log_prefix = "command::%s";

        for (const auto cmd : window.commands)
            switch (cmd)
            {
            case ::platform::command::InitWindow:
                LOGI(log_prefix, "InitWindow");

                if (!is_nested) perform_load = true;
                break;

            case ::platform::command::SaveState:
                LOGI(log_prefix, "SaveState");
                break;

            case ::platform::command::GainedFocus:
                LOGI(log_prefix, "GainedFocus");
                update_display = true;
                break;

            case ::platform::command::LostFocus:
                LOGI(log_prefix, "LostFocus");
                if (update_display)
                {
                    draw();
                    update_display = false;
                }
                break;

            case ::platform::command::GLCleanUp:
                LOGI(log_prefix, "GLCleanUp");
                update_display = false;
                if (pause)
                {
                    for (auto& it : pause->buffers)
                        it.reset();
                }
                opengl.clean();
                window.terminate_display();
                break;

            case ::platform::command::CloseWindow:
                LOGI(log_prefix, "CloseWindow");
                opengl.clean();
                window.terminate_display();
                shutdown_was_requested = true;
                return false;

            case ::platform::command::PausePressed:
                LOGI(log_prefix, "PausePressed");
                room_ctrl.sleep();
                if (!pause)
                    pause.emplace(4);
                break;
            }

        window.commands.clear();

        if (perform_load && !opengl.setup_graphics()) return false;
    }

    if (window.cursor_update)
    {
        room_ctrl.cached_cursor.store({window.cursor.pos * opengl.translate_vector, window.cursor.pressed}, std::memory_order_relaxed);
        window.cursor_update = false;
    }

    if (window.resize_request)
    {
        const auto now = std::chrono::system_clock::now();

        if (now > earliest_available_resize)
        {
            const auto request = std::move(*window.resize_request);
            window.resize_request.reset();

            const auto resize_result = opengl.resize({request.w, request.h});

            blank_display = !resize_result;

            if (!!resize_result)
            {
                room_ctrl.resize(math::point_cast<GLfloat>(opengl.draw_size));
                earliest_available_resize = now + std::chrono::milliseconds(500);
            }
        }
    }

    return !(perform_load && !load()) && !shutdown_was_requested;
}


namespace
{

GLint setup_unmasked_buffer_frame(const graphics::render_buffer_t& rb, const idle::color_t& bg)
{
    GLint default_frame_buffer;
    gl::GetIntegerv(gl::FRAMEBUFFER_BINDING, &default_frame_buffer);
    gl::BindFramebuffer(gl::FRAMEBUFFER, rb.buffer_frame);

    gl::Viewport(0, 0, rb.internal_size.x, rb.internal_size.y);
    gl::ClearColor(bg.r, bg.g, bg.b, 1);
    gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

    return default_frame_buffer;
}

void fill_frame_with_color(const idle::color_t& color)
{
    opengl.prog.fill.use();
    opengl.prog.fill.set_identity();
    opengl.prog.fill.set_view_identity();
    opengl.prog.fill.set_color(color);
    opengl.prog.fill.position_vertex(opengl.draw_bounds_verts.data());
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

GLint setup_drawing_buffer_frame(const graphics::render_buffer_t& rb, const idle::color_t& bg)
{
    GLint default_frame_buffer;
    gl::GetIntegerv(gl::FRAMEBUFFER_BINDING, &default_frame_buffer);
    gl::BindFramebuffer(gl::FRAMEBUFFER, rb.buffer_frame);

    opengl.view_mask();

    gl::ClearColor(bg.r, bg.g, bg.b, 1);
    gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

    fill_frame_with_color({1,1,1,1});

    opengl.view_normal();
    return default_frame_buffer;
}

class render_guard
{
    GLint default_frame_buffer = 0;

public:
    render_guard()
        : default_frame_buffer(setup_drawing_buffer_frame(*opengl.render_buffer_masked, ::platform::background))
    {}

    ~render_guard()
    {
        gl::BindFramebuffer(gl::FRAMEBUFFER, default_frame_buffer);
        gl::Viewport(0, 0, opengl.screen_size.x, opengl.screen_size.y);
        opengl.prog.render_masked.draw_buffer(*opengl.render_buffer_masked);
    }
};

auto wait_one_frame_with_skipping(std::chrono::steady_clock::time_point new_time)
{
    using namespace std::chrono_literals;
    constexpr auto minimum_elapsed_duration = std::chrono::duration_cast<
                        std::chrono::steady_clock::time_point::duration>(1.0s / idle::application_frames_per_second);

    if (std::chrono::steady_clock::now() - new_time < 1.0s)
    {
        new_time += minimum_elapsed_duration;
        std::this_thread::sleep_until(new_time);
    }
    else
    {
        new_time += minimum_elapsed_duration * idle::application_frames_per_second;
    }
    return new_time;
}

}  // namespace

void pause_menu::draw() const
{
    const float glare = std::sin(shift);
    const float glare_sqr = math::sqr(glare);

    opengl.prog.normal.use();
    opengl.prog.normal.set_identity();
    opengl.prog.normal.set_view_identity();

    const float t[]
    {
        0, buffers[0]->texture_h,
        buffers[0]->texture_w, buffers[0]->texture_h,
        0, 0,
        buffers[0]->texture_w, 0
    };

    const float tb[]
    {
        0, buffers[1]->texture_h,
        buffers[1]->texture_w, buffers[1]->texture_h,
        0, 0,
        buffers[1]->texture_w, 0
    };

    gl::ActiveTexture(gl::TEXTURE0);
    opengl.prog.normal.set_color({1, 1, 1, 1 - fadein_alpha * .333f});
    gl::BindTexture(gl::TEXTURE_2D, buffers[0]->texture);
    opengl.prog.normal.position_vertex(opengl.draw_bounds_verts.data());
    opengl.prog.normal.texture_vertex(t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);

    opengl.prog.normal.set_color({1, 1 - fadein_alpha * .2f, 1 - fadein_alpha * .1f, fadein_alpha * .9f});
    gl::BindTexture(gl::TEXTURE_2D, buffers[1]->texture);
    opengl.prog.normal.texture_vertex(tb);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);

    const float y_shift = opengl.draw_size.y / 2 - 60.f + (1 - fadein_alpha) * 20;

    opengl.prog.text.use();
    opengl.prog.text.set_color({1, .733f, .796f, fadein_alpha});

    idle::draw_text<idle::text_align::center>(opengl, "paused",
            {opengl.draw_size.x / 2.f, y_shift}, 64);

    if (fadein_alpha > .8f)
    {
        opengl.prog.text.set_color({1, .733f, .796f, (fadein_alpha - .8f) / .2f * (1 - glare_sqr * .5f)});

        idle::draw_text<idle::text_align::center>(opengl, "press to resume",
                {opengl.draw_size.x / 2.f, 80.f + y_shift}, 20);
    }
}

pause_menu::pause_menu(const unsigned blur_downscale)
    : buffers
    {
        opengl.new_render_buffer(),
        opengl.new_render_buffer(blur_downscale)
    }
{
    const graphics::render_buffer_t intermediate_buffer(opengl.render_buffer_masked->internal_size, opengl.render_quality);
    const auto intermediate_masked_buffer = opengl.new_render_buffer(blur_downscale);
    const auto def = setup_drawing_buffer_frame(intermediate_buffer, platform::background);

    room_ctrl.draw_frame(opengl);

    setup_unmasked_buffer_frame(*buffers[0], platform::background);
    opengl.prog.render_masked.draw_buffer(intermediate_buffer);

    opengl.prog.render_blur.use();
    opengl.prog.render_blur.set_radius(2);
    opengl.prog.render_blur.set_direction(0, 1);

    setup_unmasked_buffer_frame(*intermediate_masked_buffer, graphics::black);
    opengl.prog.render_blur.draw_buffer(*buffers[0]);

    opengl.prog.render_blur.use();
    opengl.prog.render_blur.set_direction(1, 0);

    setup_unmasked_buffer_frame(*buffers[1], graphics::black);
    opengl.prog.render_blur.draw_buffer(*intermediate_masked_buffer);

    gl::BindFramebuffer(gl::FRAMEBUFFER, def);
}

void application::draw()
{
    if (blank_display)
    {
        gl::ClearColor(0, 0, 0, 1);
        gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
    }
    else if (pause)
    {
        render_guard rg;
        pause->draw();
    }
    else
    {
        render_guard rg;
        room_ctrl.draw_frame(opengl);
    }
    window.buffer_swap();
}

#define PRINT_SIZE(obj) LOGD("# sizeof " #obj " = %zu", sizeof(obj))

int application::real_main()
{
    application app;

#ifdef __clang__
    LOGD("%s", "### CLANG " __clang_version__);
#elif __GNUC__
    LOGD("### GCC %d.%d", __GNUC__, __GNUC_MINOR__);
#endif

    PRINT_SIZE(platform::context);
    PRINT_SIZE(idle::controller);
    PRINT_SIZE(idle::mat4x4_t);

    while (app.execute_commands(false))
    {
        if (app.pause)
        {
            if (app.window.cursor.pressed)
            {
                const auto p = app.window.cursor.pos * opengl.translate_vector;
                if (fabsf(p.x - opengl.draw_size.x / 2) < 150.f
                        && fabsf(p.y - (opengl.draw_size.y / 2 + 30.f)) < 60.f)
                {
                    app.pause.reset();
                    room_ctrl.awaken(app.clock);
                    continue;
                }
            }

            app.pause->fadein_alpha += (1 - app.pause->fadein_alpha) / 30;

            app.pause->shift += .029f;
            if (app.pause->shift > F_TAU)
                app.pause->shift -= F_TAU;
        }

        if (app.window.has_opengl())
        {
            if (app.update_display)
            {
                app.draw();
            }

            idle::image_t::load_topmost_queued_picture();
        }

        app.clock = wait_one_frame_with_skipping(app.clock);
    }
    return 0;
}

bool application::load()
{
    using namespace std::chrono_literals;
    constexpr auto minimum_elapsed = 1.0s / (idle::application_frames_per_second / 2 + 5);

    room_ctrl.sleep();

    if (room_ctrl.haiku.has_crashed())
        return false;

    LOGI("Asset refresh requested");

    idle::lodge la {
        idle::image_t::load_from_assets_immediate("path4368.png"),
        idle::image_t::load_from_assets_immediate("space-1.png")
    };

    std::chrono::time_point<std::chrono::steady_clock> lt = std::chrono::steady_clock::now();
    std::promise<bool> loader_callback;
    auto loader_result = loader_callback.get_future();

    std::thread loader_thread {
            [] (std::promise<bool> promise)
            {
                bool success = false;

                if (const auto fontfile = platform::asset::hold(idle::config::font_asset))
                {
                    if (auto opt = fonts::freetype_glue{}(fontfile.view(), fonts::texture_quality::ok))
                    {
                        LOGD("Font acquired!");

                        auto image = graphics::unique_texture(idle::image_t::load_from_memory(
                                    opt->width, opt->width,
#ifdef __ANDROID__
                                    gl::LUMINANCE, gl::LUMINANCE,
#else
                                    gl::R8, gl::RED,
#endif
                                    gl::LINEAR, gl::CLAMP_TO_EDGE, std::move(opt->pixels)).release());

                        opengl.font.emplace(std::move(opt->map), opt->cell_size, std::move(image));
                        success = true;
                    }
                }

                promise.set_value(success);
            },
            std::move(loader_callback)
        };

    while (loader_result.wait_for(std::chrono::milliseconds(5)) != std::future_status::ready)
    {
        if (!execute_commands(true))
        {
            LOGE("Loader called shut down.");
            loader_thread.join();
            return false;
        }

        if (window.has_opengl())
        {
            idle::image_t::load_topmost_queued_picture();

            if (update_display && std::chrono::steady_clock::now() - lt > minimum_elapsed)
            {
                lt += std::chrono::duration_cast<std::chrono::steady_clock::time_point::duration>(minimum_elapsed);
                {
                    render_guard rg;
                    la.draw(opengl);
                }
                window.buffer_swap();
            }
        }
    }

    loader_thread.join();

#ifdef IDLE_COMPILE_FONT_DEBUG_SCREEN
    if (!!window.has_opengl())
    {
        {
            render_guard rg;

            opengl.prog.normal.use();
            opengl.prog.normal.set_identity();
            opengl.prog.normal.set_view_identity();

            gl::ActiveTexture(gl::TEXTURE0);
            opengl.prog.normal.set_color({1, 1, 1, 1});
            gl::BindTexture(gl::TEXTURE_2D, opengl.font->texture.get());
            opengl.prog.normal.position_vertex(opengl.draw_bounds_verts.data());
            opengl.prog.normal.texture_vertex(opengl.texture_bounds_verts.data());
            gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
        }
        window.buffer_swap();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
#endif

    room_ctrl.clear_monostate();
    room_ctrl.awaken(clock);

    return loader_result.get();
}

}  // namespace isolation

