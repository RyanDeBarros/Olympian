#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Input.h"

namespace oly
{
	namespace rendering
	{
		struct WindowHint
		{
			struct
			{
				bool resizable = true;
				bool visible = true;
				bool decorated = true;
				bool focused = true;
				bool auto_iconify = true;
				bool floating = false;
				bool maximized = false;
				bool center_cursor = true;
				bool transparent_framebuffer = false;
				bool focus_on_show = true;
				bool scale_to_monitor = false;
				bool scale_framebuffer = true;
				bool mouse_passthrough = false;
				unsigned int position_x = GLFW_ANY_POSITION;
				unsigned int position_y = GLFW_ANY_POSITION;
				int refresh_rate = GLFW_DONT_CARE;
				bool stereo = false;
				bool srgb_capable = false;
				bool double_buffer = true;
				int context_version_major = 4; // 1
				int context_version_minor = 5; // 0
				bool opengl_forward_compat = false;
				bool context_debug = false;
				int opengl_profile = GLFW_OPENGL_CORE_PROFILE; // GLFW_OPENGL_ANY_PROFILE
			} window;

			struct
			{
				int swap_interval = 1;
				glm::vec4 clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
			} context;

			void window_hint() const;
			void context_hint() const;
		};

		class Window
		{
			GLFWwindow* w;
			glm::ivec2 size;

		public:
			Window(int width, int height, const char* title, const WindowHint& hint = {}, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);
			Window(const Window&) = delete;
			Window(Window&&) noexcept;
			~Window();
			Window& operator=(Window&&) noexcept;

			operator GLFWwindow* () const { return w; }
			glm::ivec2 get_size() const { return size; }
			int get_width() const { return size.x; }
			int get_height() const { return size.y; }
			void set_size(glm::ivec2 size);
			void set_width(int width);
			void set_height(int height);

			glm::vec4 projection_bounds() const;

			void make_context_current() const;
			bool should_close() const;
			void should_close(bool close) const;
			void swap_buffers() const;

			struct
			{
				EventHandler<input::CharEventData> character;
				EventHandler<input::CharModsEventData> char_mods;
				EventHandler<input::CursorEnterEventData> cursor_enter;
				EventHandler<input::CursorPosEventData> cursor_pos;
				EventHandler<input::PathDropEventData> path_drop;
				EventHandler<input::FramebufferResizeEventData> framebuffer_resize;
				EventHandler<input::KeyEventData> key;
				EventHandler<input::MouseButtonEventData> mouse_button;
				EventHandler<input::ScrollEventData> scroll;
				EventHandler<input::WindowCloseEventData> window_close;
				EventHandler<input::WindowContentScaleEventData> window_content_scale;
				EventHandler<input::WindowFocusEventData> window_focus;
				EventHandler<input::WindowIconifyEventData> window_iconify;
				EventHandler<input::WindowMaximizeEventData> window_maximize;
				EventHandler<input::WindowPosEventData> window_pos;
				EventHandler<input::WindowRefreshEventData> window_refresh;
				EventHandler<input::WindowResizeEventData> window_resize;
			} handlers;
		};
	}
}
