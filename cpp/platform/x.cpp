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
#include <memory>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "opengl_core_adaptive.hpp"
#include <GL/glx.h>
#include <cstdlib>
#include <atomic>

#include <log.hpp>
#include "context.hpp"
#include "asset_access.hpp"
#include <almost_cpp20.hpp>

namespace platform
{
namespace
{

struct x11_display
{
    Display                 *display;
    Window                  window;
    GLXContext              context;
    Colormap                colormap;

    x11_display() = delete;

    static const x11_display& cast(const context::data_t& data) noexcept
    {
        return *std::launder(reinterpret_cast<const x11_display*>(data));
    }

    static x11_display& cast(context::data_t& data) noexcept
    {
        return *std::launder(reinterpret_cast<x11_display*>(data));
    }
};

#ifdef X11_USE_CLIENTMESSAGE
constinit Atom wmDeleteMessage;
#endif

int x_fatal_error_handler(Display *) noexcept
{
    LOGE(u8"\U0001F480 (Display destroyed)");
    return 0;
}

int x_error_handler(Display *dpy, XErrorEvent *ev) noexcept
{
    char str[250];
    XGetErrorText(dpy, ev->error_code, str, 250);
    LOGE("%s", str);
    return 0;
}

static_assert(sizeof(x11_display) <= sizeof(context::data_t));

constexpr unsigned initial_width = 1280, initial_height = 720;

bool create_window(x11_display& x) noexcept
{
    x.display = nullptr;

    XSetWindowAttributes setwindowattributes;

    GLint attributes[]
    {
        GLX_X_RENDERABLE  , True,
        GLX_DRAWABLE_TYPE , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE   , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE , GLX_TRUE_COLOR,
        GLX_RED_SIZE      , 4,
        GLX_GREEN_SIZE    , 4,
        GLX_BLUE_SIZE     , 4,
        GLX_ALPHA_SIZE    , 4,
        GLX_DEPTH_SIZE    , 16,
        None
    };

    std::unique_ptr<Display, decltype(&XCloseDisplay)> new_display{ XOpenDisplay(nullptr), XCloseDisplay };

    if (!new_display)
    {
        LOGE("Cannot connect to X server");
        return false;
    }

    if (int glx_major, glx_minor;
        !glXQueryVersion(new_display.get(), &glx_major, &glx_minor)
        || ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1))
    {
        LOGE("Invalid GLX version");
        return false;
    }

    using x_free = decltype([](void* ptr){ XFree(ptr); });

    GLXFBConfig bestFbc; // Get framebuffers
    int fbcount;
    if (const std::unique_ptr<GLXFBConfig[], x_free> fbc{
                glXChooseFBConfig(new_display.get(), DefaultScreen(new_display.get()), attributes, &fbcount)
            })
    {
        int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

        for (int i = 0; i < fbcount; ++i)
        {
            if (const std::unique_ptr<XVisualInfo, x_free> vi{
                        glXGetVisualFromFBConfig(new_display.get(), fbc[i])
                    })
            {
                int samp_buf, samples;
                glXGetFBConfigAttrib(new_display.get(), fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
                glXGetFBConfigAttrib(new_display.get(), fbc[i], GLX_SAMPLES, &samples);

                if (best_fbc < 0 || (samp_buf && samples > best_num_samp))
                {
                    best_fbc = i;
                    best_num_samp = samples;
                }

                if (worst_fbc < 0 || (!samp_buf || samples < worst_num_samp))
                {
                    worst_fbc = i;
                    worst_num_samp = samples;
                }
            }
        }
        bestFbc = fbc[best_fbc];
    }
    else
    {
        LOGE("Failed to retrieve a framebuffer config");
        return false;
    }

    const std::unique_ptr<XVisualInfo, x_free> visualinfo{ glXGetVisualFromFBConfig(new_display.get(), bestFbc) };

    if(!visualinfo)
    {
        LOGE("No appropriate visual found");
        return false;
    }

    x.colormap = XCreateColormap(new_display.get(), RootWindow(new_display.get(), visualinfo->screen), visualinfo->visual, AllocNone);

    setwindowattributes.colormap = x.colormap;
    setwindowattributes.event_mask = ButtonPressMask | StructureNotifyMask | ButtonReleaseMask |
                          //EnterWindowMask | LeaveWindowMask |
                          KeyPressMask | // KeyReleaseMask |
                          PointerMotionMask | Button1MotionMask; // | VisibilityChangeMask | ExposureMask;

    x.window = XCreateWindow(new_display.get(),
            RootWindow(new_display.get(), visualinfo->screen),
            0, 0,
            initial_width, initial_height,
            0, visualinfo->depth,
            InputOutput, visualinfo->visual,
            CWColormap | CWEventMask,
            &setwindowattributes);

    if(!x.window)
    {
        LOGE("Failed to create a window");
        XFreeColormap(new_display.get(), x.colormap);
        return false;
    }
    XStoreName(new_display.get(), x.window, "idle/crimson");
    XMapWindow(new_display.get(), x.window);


    auto default_error_handler = XSetErrorHandler(x_error_handler);
    XSetIOErrorHandler(x_fatal_error_handler);

    x.context = glXCreateContext(new_display.get(), visualinfo.get(), NULL, True);

    if (!x.context)
    {
        XDestroyWindow(new_display.get(), x.window);
        XFreeColormap(new_display.get(), x.colormap);
        LOGE("Failed to create a proper gl context");
        return false;
    }

#ifdef X11_USE_CLIENTMESSAGE
    wmDeleteMessage = XInternAtom(new_display.get(), "WM_DELETE_WINDOW", False);
    XSetWMProtocols(new_display.get(), x.window, &wmDeleteMessage, 1);
#endif

    // Sync to ensure any errors generated are processed.
    XSync(new_display.get(), False);
    XSetErrorHandler(default_error_handler);

    if (!glXIsDirect(new_display.get(), x.context))
    {
        LOGE(u8"Indirect GLX rendering context obtained (\U0001f937 why\?\?)");
    }

    glXMakeCurrent(new_display.get(), x.window, x.context);


    if (const static gl::exts::LoadTest glTest = gl::sys::LoadFunctions(); !glTest)
    {
        LOGE("Failed to load crucial OpenGL functions");
        glXMakeCurrent(new_display.get(), None, NULL);
        glXDestroyContext(new_display.get(), x.context);
        XDestroyWindow(new_display.get(), x.window);
        XFreeColormap(new_display.get(), x.colormap);
        return false;
    }
    else if (const auto amt = glTest.GetNumMissing(); amt > 0)
    {
        LOGE("Number of functions that failed to load: %i.", amt);
    }

    x.display = new_display.release();
    return true;
}

} // namespace

context::context() noexcept
{
    LOGDD("context::context");

    if (!create_window(x11_display::cast(data)))
    {
        commands.insert(command::close_window);
    }
    else
    {
        resize_request.emplace(initial_width, initial_height);
        commands.insert(command::init_window);
        commands.insert(command::gained_focus); // TODO: Make X handle focus as to reduce resource usage when idle
    }
}

void context::buffer_swap() noexcept
{
    auto& x = x11_display::cast(data);
    glXSwapBuffers(x.display, x.window);
}

void context::event_loop_back(bool) noexcept
{
    auto& x = x11_display::cast(data);
    XEvent xev;
    {
        // Without this query call the mouse might not work at all
        Window root_window;
        int root_x, root_y;
        unsigned int mask;
        XQueryPointer(x.display, x.window, &root_window, &root_window, &root_x, &root_y, &root_x, &root_y, &mask);
    }

    while(!commands.is_full() && XQLength(x.display))
    {
        XNextEvent(x.display, &xev);
        switch (xev.type) {
            case ButtonPress:
            case ButtonRelease:
                cursor.pos = { static_cast<float>(xev.xbutton.x), static_cast<float>(xev.xbutton.y) };
                cursor.pressed = xev.xbutton.type == ButtonPress;
                cursor_update = true;
                break;

            case EnterNotify:
            case MotionNotify:
            case LeaveNotify:
                cursor.pos = { static_cast<float>(xev.xmotion.x), static_cast<float>(xev.xmotion.y) };
                cursor_update = true;
                break;

            case ConfigureNotify:
                glXMakeCurrent(x.display, x.window, x.context);
                resize_request.emplace(xev.xconfigure.width, xev.xconfigure.height);
                break;

            case KeyPress:
                if (xev.xkey.keycode == 0x9)
                {
                    commands.insert(command::pause_pressed);
                }
                break;

#ifdef X11_USE_CLIENTMESSAGE
            case ClientMessage:
                commands.insert(command::close_window);
                return;
#endif

            default:
                break;
        }
    }
}

context::~context() noexcept
{
    LOGDD("context::~context");
    terminate_display();
}

bool context::has_opengl() const noexcept
{
    auto& x = x11_display::cast(data);
    return !!x.display;
}

void context::terminate_display() noexcept
{
    auto& x = x11_display::cast(data);

    LOGD("context::terminate_display");

    if (x.display)
    {
        glXMakeCurrent(x.display, None, NULL);
        glXDestroyContext(x.display, x.context);
        XDestroyWindow(x.display, x.window);
        XFreeColormap(x.display, x.colormap);
        XCloseDisplay(x.display);
        x.display = nullptr;
    }
}

asset::operator bool() const noexcept
{
    return !!size;
}

std::string_view asset::view() const noexcept
{
    return { reinterpret_cast<const char*>(ptr.get()), size };
}

asset asset::hold(std::string path) noexcept
{
    path.insert(0, "assets/");

    if (const std::unique_ptr<FILE, decltype([](FILE* f){ std::fclose(f); })> f{ std::fopen(path.c_str(), "rb") })
    {
        std::fseek(f.get(), 0, SEEK_END);

        const auto size = std::ftell(f.get());
        if (size <= 0 || std::fseek(f.get(), 0, SEEK_SET) != 0)
            return {};

        LOGD("\"%s\" - loaded %ld %s",
                path.c_str(),
                size >= 1024 ? size / 1024 : size,
                size >= 1024 ? "KB" : "bytes");

        auto ptr = std::make_unique<unsigned char[]>(size);

        if (std::fread(ptr.get(), 1, size, f.get()) == static_cast<size_t>(size))
        {
            return { std::move(ptr), static_cast<size_t>(size) };
        }
    }

    LOGE("Couldn't get a hold of \"%s\"", path.c_str());
    return {};
}

asset asset::hold(const char * path) noexcept
{
    return hold(std::string{path});
}

}  // namespace platform
