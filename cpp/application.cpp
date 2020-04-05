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

#include <cstdio>
#include <future>
#include "platform/display.hpp"
#include "gl.hpp"
#include "application.hpp"
#include "drawable.hpp"
#include "draw_text.hpp"
#include "room_variant.hpp"
#include "room_controller.hpp"
#include "lodge.hpp"
#include "database.hpp"

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

    bool perform_load = false;

    if (!!window.commands.size())
    {
        constexpr auto log_prefix = "command::%s";

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
                opengl.shutdown_was_requested = true;
                return false;

            case ::platform::command::PausePressed:
                LOGI(log_prefix, "PausePressed");
                room_ctrl.slumber();
                if (!pause)
                    pause.emplace();
                break;
            }

        window.commands.clear();

        if (perform_load && !opengl.setup_graphics()) return false;
    }

    if (window.resize_request)
    {
        const auto now = std::chrono::system_clock::now();
        if (now > last_resize + std::chrono::seconds(1))
        {
            const auto& rq = *window.resize_request;
            opengl.resize(rq.w, rq.h, rq.q, rq.r);

            window.resize_request.reset();

            room_ctrl.resize({
                static_cast<float>(opengl.draw_size.x),
                static_cast<float>(opengl.draw_size.y)
            });

            last_resize = now;
        }
    }

    return !(perform_load && !load()) && !opengl.shutdown_was_requested;
}


namespace
{

GLint setup_buffer_frame(const graphics::render_buffer_t& rb, const math::point2<int> internal_size, const idle::color_t& bg)
{
    GLint default_frame_buffer = 0;
    gl::GetIntegerv(gl::FRAMEBUFFER_BINDING, &default_frame_buffer);
    gl::BindFramebuffer(gl::FRAMEBUFFER, rb.buffer_frame);
    gl::Viewport(0, 0, internal_size.x, internal_size.y);
    gl::ClearColor(bg.r, bg.g, bg.b, 1);
    gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
    return default_frame_buffer;
}

class render_guard
{
    GLint default_frame_buffer = 0;
    ::platform::window& window;

public:
    render_guard(::platform::window& win)
        : default_frame_buffer(setup_buffer_frame(*opengl.render_buffer, opengl.internal_size, ::platform::background))
        , window(win)
    {}

    ~render_guard()
    {
        gl::BindFramebuffer(gl::FRAMEBUFFER, default_frame_buffer);
        gl::Viewport(0, 0, opengl.screen_size.x, opengl.screen_size.y);
        opengl.prog.render_final.draw_buffer(*opengl.render_buffer);
        window.buffer_swap();
    }
};

void draw_pause_menu(const graphics::core& gl, const float pause_menu_alpha, const graphics::render_buffer_t& rb, const graphics::render_buffer_t& brb)
{
    gl.prog.normal.use();
    gl.prog.normal.set_identity();
    gl.prog.normal.set_view_identity();

    const float v[]
    {
        0, 0,
        static_cast<float>(gl.draw_size.x), 0,
        0, static_cast<float>(gl.draw_size.y),
        static_cast<float>(gl.draw_size.x), static_cast<float>(gl.draw_size.y)
    };

    const float t[]
    {
        0, rb.texture_h,
        rb.texture_w, rb.texture_h,
        0, 0,
        rb.texture_w, 0
    };

    const float tb[]
    {
        0, brb.texture_h,
        brb.texture_w, brb.texture_h,
        0, 0,
        brb.texture_w, 0
    };

    gl::ActiveTexture(gl::TEXTURE0);
    if (pause_menu_alpha < .966f)
    {
        gl.prog.normal.set_color({1, 1, 1, 1 - pause_menu_alpha});
        gl::BindTexture(gl::TEXTURE_2D, rb.texture);
        gl.prog.normal.position_vertex(v);
        gl.prog.normal.texture_vertex(t);
        gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
    }

    gl.prog.normal.set_color({1, 1 - pause_menu_alpha * .2f, 1 - pause_menu_alpha * .1f, pause_menu_alpha * .668f});
    gl::BindTexture(gl::TEXTURE_2D, brb.texture);
    gl.prog.normal.position_vertex(v);
    gl.prog.normal.texture_vertex(tb);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);

    const float yshift = (1 - pause_menu_alpha) * 20;
    const auto rect = idle::rect_t(gl.draw_size.x / 2 - 220.f, gl.draw_size.y / 2 - 60.f, gl.draw_size.x / 2 + 220.f, gl.draw_size.y / 2 + 60.f);
    gl.prog.text.use();
    gl.prog.text.set_color(1, .133f, .196f, pause_menu_alpha);

    idle::draw_text<idle::TextAlign::Center>(gl, "paused",
                        idle::point_t(gl.draw_size.x / 2.f, rect.top + 10.f + yshift), 55);

    idle::draw_text<idle::TextAlign::Center, idle::TextAlign::Center>(gl, "resume",
                        idle::point_t(gl.draw_size.x / 2.f, rect.bottom - 35.f + yshift), 14 * pause_menu_alpha + 16);
}

void wait_one_frame_with_skipping(std::chrono::steady_clock::time_point& new_time)
{
    using namespace std::chrono_literals;
    constexpr auto minimum_elapsed_duration = std::chrono::duration_cast<
                        std::chrono::steady_clock::time_point::duration>(1.0s / APPLICATION_FPS);

    if (std::chrono::steady_clock::now() - new_time < 1.0s)
    {
        new_time += minimum_elapsed_duration;
        std::this_thread::sleep_until(new_time);
    }
    else
    {
        new_time += minimum_elapsed_duration * APPLICATION_FPS;
    }
}

}  // namespace


void application::draw()
{
    if (pause)
    {
        if (!pause->buffers[1])
        {
            constexpr unsigned downscale = 4;
            std::optional<graphics::render_buffer_t> intermediate_buffer;

            opengl.new_render_buffer(pause->buffers[0]);
            opengl.new_render_buffer(pause->buffers[1], downscale);
            opengl.new_render_buffer(intermediate_buffer, downscale);

            opengl.prog.render_blur.use();
            opengl.prog.render_blur.set_radius(3.f);
            opengl.prog.render_blur.set_direction(0.f, 1.f);

            const auto def = setup_buffer_frame(*pause->buffers[0], opengl.internal_size, platform::background);

            room_ctrl.draw_frame(opengl);

            setup_buffer_frame(*intermediate_buffer, opengl.internal_size / downscale, platform::background);
            opengl.prog.render_blur.draw_buffer(*pause->buffers[0]);

            opengl.prog.render_blur.use();
            opengl.prog.render_blur.set_direction(1.f, 0.f);

            setup_buffer_frame(*pause->buffers[1], opengl.internal_size / downscale, platform::background);
            opengl.prog.render_blur.draw_buffer(*intermediate_buffer);

            gl::BindFramebuffer(gl::FRAMEBUFFER, def);
        }

        render_guard _{window};
        draw_pause_menu(opengl, pause->animation, *pause->buffers[0], *pause->buffers[1]);
    }
    else
    {
        render_guard _{window};
        room_ctrl.draw_frame(opengl);
    }
}

#define PRINT_SIZE(obj) LOGD("# sizeof " #obj " = %zu", sizeof(obj))

int application::real_main()
{
#ifdef __clang__
    LOGD("%s", "### CLANG " __clang_version__);
#elif __GNUC__
    LOGD("### GCC %d.%d", __GNUC__, __GNUC_MINOR__);
#endif

    PRINT_SIZE(platform::window);
    PRINT_SIZE(window.commands);
    PRINT_SIZE(idle::controller);
    PRINT_SIZE(graphics::core);
    PRINT_SIZE(graphics::program_t);
    PRINT_SIZE(graphics::textured_program_t);
    PRINT_SIZE(idle::mat4x4_t);
    PRINT_SIZE(font_t);
    PRINT_SIZE(std::optional<font_t>);
    PRINT_SIZE(graphics::render_buffer_t);
    PRINT_SIZE(std::optional<graphics::render_buffer_t>);
    PRINT_SIZE(platform::pointer);
    PRINT_SIZE(std::atomic<platform::pointer>);

    auto app_time = std::chrono::steady_clock::now();

    while (execute_commands(false) && room_ctrl.execute_pending_room_change(opengl))
    {
        if (pause)
        {
            if (window.cursor.single_press)
            {
                const auto p = opengl.pointer.load(std::memory_order_relaxed).pos;
                if (fabsf(p.x - opengl.draw_size.x / 2) < 150.f
                        && fabsf(p.y - (opengl.draw_size.y / 2 + 30.f)) < 60.f)
                {
                    pause.reset();
                    pointer.clear(window.cursor);
                    room_ctrl.wake(opengl, app_time);
                    continue;
                }
            }
            pause->animation += (1 - pause->animation) / 20;
        }

        if (window.has_gl_context())
        {
            if (update_display)
            {
                draw();
            }

            idle::image_t::load_topmost_queued_picture();
        }

        pointer.update(window.cursor);

        auto cursor = window.cursor;
        cursor.pos *= opengl.translate_vector;
        opengl.pointer.store(cursor, std::memory_order_relaxed);

        wait_one_frame_with_skipping(app_time);
    }
    return 0;
}

bool application::load()
{
    using namespace std::chrono_literals;
    constexpr auto minimum_elapsed = 1.0s / (APPLICATION_FPS / 2 + 5);

    if (room_ctrl.get_crashed())
        return false;

    LOGI("Asset refresh requested");

    idle::lodge la {
        idle::image_t::load_from_assets_immediate("path4368.png"),
        idle::image_t::load_from_assets_immediate("space-1.png")
    };

    room_ctrl.slumber();
    room_ctrl.join_worker();

    std::chrono::time_point<std::chrono::steady_clock> lt = std::chrono::steady_clock::now();
    std::promise<bool> loader_callback;
    auto loader_result = loader_callback.get_future();

    std::thread loader_thread {
            [this] (std::promise<bool> promise)
            {
                promise.set_value(idle::load_everything(window, opengl));
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

        if (window.has_gl_context())
        {
            idle::image_t::load_topmost_queued_picture();

            if (update_display && std::chrono::steady_clock::now() - lt > minimum_elapsed)
            {
                lt += std::chrono::duration_cast<std::chrono::steady_clock::time_point::duration>(minimum_elapsed);
                render_guard _{window};
                la.draw(opengl);
            }
        }
    }

    loader_thread.join();

    room_ctrl.default_room_if_none_set();

    return loader_result.get();
}

}  // namespace isolation

