#include "Window.h"

#include "core/base/Errors.h"
#include "core/base/Assert.h"

namespace oly::platform
{
	void WindowHint::window_hint() const
	{
		OLY_ASSERT(window.context_version_major > 4 || (window.context_version_major == 4 && window.context_version_minor >= 5));

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

	Window::Window(int width, int height, const char* title, const WindowHint& hint, GLFWmonitor* monitor, GLFWwindow* share)
		: size(width, height)
	{
		hint.window_hint();
		w = glfwCreateWindow(width, height, title, monitor, share);
		if (!w)
			throw Error(ErrorCode::WINDOW_CREATION);
		make_context_current();
		if (glewInit() != GLEW_OK)
			throw oly::Error(oly::ErrorCode::GLEW_INIT);
		hint.context_hint();
		input::init_handlers(w);
		glfwSetWindowUserPointer(w, this);
		glViewport(0, 0, width, height);
	}

	Window::Window(Window&& other) noexcept
		: w(other.w), size(other.size)
	{
		glfwSetWindowUserPointer(w, this);
		other.w = nullptr;
	}

	Window::~Window()
	{
		glfwDestroyWindow(w);
	}

	Window& Window::operator=(Window&& other) noexcept
	{
		if (this != &other)
		{
			glfwDestroyWindow(w);
			w = other.w;
			glfwSetWindowUserPointer(w, this);
			size = other.size;
			other.w = nullptr;
		}
		return *this;
	}

	void Window::set_size(glm::ivec2 size)
	{
		this->size = size;
		glfwSetWindowSize(w, size.x, size.y);
	}

	void Window::set_width(int width)
	{
		size.x = width;
		glfwSetWindowSize(w, width, 0);
	}

	void Window::set_height(int height)
	{
		size.y = height;
		glfwSetWindowSize(w, 0, height);
	}

	float Window::aspect_ratio() const
	{
		return float(size.x) / size.y;
	}

	void Window::refresh_size()
	{
		glfwGetWindowSize(w, &size.x, &size.y);
	}

	void Window::make_context_current() const
	{
		glfwMakeContextCurrent(w);
	}

	bool Window::should_close() const
	{
		return glfwWindowShouldClose(w);
	}

	void Window::should_close(bool close) const
	{
		glfwSetWindowShouldClose(w, close);
	}

	void Window::swap_buffers() const
	{
		glfwSwapBuffers(w);
	}

}
