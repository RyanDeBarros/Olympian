#include "Window.h"

#include "core/base/Errors.h"
#include "core/base/Assert.h"
#include "core/context/Context.h"

namespace oly::platform
{
    void WindowHint::window_hint() const
    {
        OLY_ASSERT(window.context_version_major > 4 ||
            (window.context_version_major == 4 && window.context_version_minor >= 5));

        glfwWindowHint(GLFW_RESIZABLE, window.resizable);
        glfwWindowHint(GLFW_VISIBLE, window.visible);
        glfwWindowHint(GLFW_DECORATED, window.decorated);
        glfwWindowHint(GLFW_FOCUSED, window.focused);
        glfwWindowHint(GLFW_AUTO_ICONIFY, window.auto_iconify);
        glfwWindowHint(GLFW_FLOATING, window.floating);
        glfwWindowHint(GLFW_MAXIMIZED, window.maximized);
        glfwWindowHint(GLFW_CENTER_CURSOR, window.center_cursor);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, window.transparent_framebuffer);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, window.focus_on_show);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, window.scale_to_monitor);
        glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, window.scale_framebuffer);
        glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, window.mouse_passthrough);
        glfwWindowHint(GLFW_POSITION_X, window.position_x);
        glfwWindowHint(GLFW_POSITION_Y, window.position_y);
        glfwWindowHint(GLFW_REFRESH_RATE, window.refresh_rate);
        glfwWindowHint(GLFW_STEREO, window.stereo);
        glfwWindowHint(GLFW_SRGB_CAPABLE, window.srgb_capable);
        glfwWindowHint(GLFW_DOUBLEBUFFER, window.double_buffer);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, window.context_version_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, window.context_version_minor);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, window.opengl_forward_compat);
        glfwWindowHint(GLFW_CONTEXT_DEBUG, window.context_debug);
        glfwWindowHint(GLFW_OPENGL_PROFILE, window.opengl_profile);
    }

    void WindowHint::context_hint() const
    {
        glfwSwapInterval(context.swap_interval);
        glClearColor(context.clear_color.r, context.clear_color.g, context.clear_color.b, context.clear_color.a);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    internal::RootWindowResizeHandler::RootWindowResizeHandler(Window& window)
        : window(window)
    {
    }

    bool internal::RootWindowResizeHandler::block(const input::WindowResizeEventData& data)
    {
        window.size_ = { data.w, data.h };
        
        const float now = (float)glfwGetTime();
        const float elapsed = now - last_update_;
        
        if (elapsed < resizing_frame_length)
            return true;
        else
        {
            last_update_ = now;
            return false;
        }
    }

    bool internal::RootWindowResizeHandler::consume(const input::WindowResizeEventData& data)
    {
        return !context::internal::render_frame();
    }

    Window::HandlerList::HandlerList(Window& window)
        : window_resize(window)
    {
    }

    Window::Window(
        int width,
        int height,
        const char* title,
        const WindowHint& hint,
        GLFWmonitor* monitor,
        GLFWwindow* share)
        : size_(width, height)
        , handlers(*this)
    {
        hint.window_hint();
        win_ = glfwCreateWindow(width, height, title, monitor, share);
        if (!win_)
        {
            _OLY_ENGINE_LOG_FATAL("PLATFORM") << "glfwCreateWindow() failed." << kLog.nl;
            throw Error(ErrorCode::WindowCreation);
        }

        make_context_current();
        if (glewInit() != GLEW_OK)
        {
            _OLY_ENGINE_LOG_FATAL("PLATFORM") << "glewInit() failed." << kLog.nl;
            throw Error(ErrorCode::GlewInit);
        }
        
        hint.context_hint();

        input::init_handlers(win_);
        glfwSetWindowUserPointer(win_, this);
        glViewport(0, 0, width, height);
    }

    Window::Window(Window&& o) noexcept
        : win_(o.win_), size_(o.size_), handlers(*this)
    {
        glfwSetWindowUserPointer(win_, this);
        o.win_ = nullptr;
    }

    Window::~Window()
    {
        glfwDestroyWindow(win_);
    }

    Window& Window::operator=(Window&& o) noexcept
    {
        if (this != &o)
        {
            glfwDestroyWindow(win_);
            win_ = o.win_;
            glfwSetWindowUserPointer(win_, this);
            size_ = o.size_;
            o.win_ = nullptr;
        }

        return *this;
    }

    Window::operator GLFWwindow*() const
    {
        return win_;
    }

    glm::ivec2 Window::get_size() const
    {
        return size_;
    }

    int Window::get_width() const
    {
        return size_.x;
    }

    int Window::get_height() const
    {
        return size_.y;
    }

    void Window::set_size(glm::ivec2 size)
    {
        size_ = size;
        glfwSetWindowSize(win_, size.x, size.y);
    }

    void Window::set_width(int width)
    {
        size_.x = width;
        glfwSetWindowSize(win_, width, 0);
    }

    void Window::set_height(int height)
    {
        size_.y = height;
        glfwSetWindowSize(win_, 0, height);
    }

    float Window::aspect_ratio() const
    {
        return float(size_.x) / size_.y;
    }

    void Window::make_context_current() const
    {
        glfwMakeContextCurrent(win_);
    }

    bool Window::should_close() const
    {
        return glfwWindowShouldClose(win_);
    }

    void Window::should_close(bool close) const
    {
        glfwSetWindowShouldClose(win_, close);
    }

    void Window::swap_buffers() const
    {
        glfwSwapBuffers(win_);
    }

    glm::vec2 Window::get_cursor_screen_position() const
    {
        double x, y;
        glfwGetCursorPos(win_, &x, &y);
        return { (float)x, (float)y };
    }

    glm::vec2 Window::screen_to_view_coordinates(glm::vec2 screen_pos) const
    {
        return { screen_pos.x - 0.5f * size_.x, 0.5f * size_.y - screen_pos.y };
    }

    glm::vec2 Window::get_cursor_view_position() const
    {
        return screen_to_view_coordinates(get_cursor_screen_position());
    }

    glm::vec2 Window::view_to_screen_coordinates(glm::vec2 view_pos) const
    {
        return { view_pos.x + 0.5f * size_.x, 0.5f * size_.y - view_pos.y };
    }
}
