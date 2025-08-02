#include "WindowEvents.h"

#include "core/platform/Window.h"

namespace oly
{
	namespace input
	{
		static void character(GLFWwindow* window, unsigned int codepoint)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.character.handle({ codepoint });
		}

		static void char_mods(GLFWwindow* window, unsigned int codepoint, int mods)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.char_mods.handle({ codepoint, mods });
		}

		static void cursor_enter(GLFWwindow* window, int entered)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.cursor_enter.handle({ (bool)entered });
		}

		static void cursor_pos(GLFWwindow* window, double x, double y)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.cursor_pos.handle({ x, y });
		}

		static void drop(GLFWwindow* window, int num_paths, const char** paths)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.path_drop.handle({ num_paths, paths });
		}

		static void framebuffer_size(GLFWwindow* window, int w, int h)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.framebuffer_resize.handle({ w, h });
		}

		static void key(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.key.handle({ key, scancode, action, mods });
		}

		static void mouse_button(GLFWwindow* window, int button, int action, int mods)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.mouse_button.handle({ button, action, mods });
		}

		static void scroll(GLFWwindow* window, double xoff, double yoff)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.scroll.handle({ xoff, yoff });
		}

		static void window_close(GLFWwindow* window)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.window_close.handle({});
		}

		static void window_content_scale(GLFWwindow* window, float x_scale, float y_scale)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.window_content_scale.handle({ x_scale, y_scale });
		}

		static void window_focus(GLFWwindow* window, int focused)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.window_focus.handle({ (bool)focused });
		}

		static void window_iconify(GLFWwindow* window, int iconified)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.window_iconify.handle({ (bool)iconified });
		}

		static void window_maximize(GLFWwindow* window, int maximized)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.window_maximize.handle({ (bool)maximized });
		}

		static void window_pos(GLFWwindow* window, int x, int y)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.window_pos.handle({ x, y });
		}

		static void window_refresh(GLFWwindow* window)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.window_refresh.handle({});
		}

		static void window_size(GLFWwindow* window, int width, int height)
		{
			static_cast<platform::Window*>(glfwGetWindowUserPointer(window))->handlers.window_resize.handle({ width, height });
		}

		void init_handlers(GLFWwindow* w)
		{
			glfwSetCharCallback(w, character);
			glfwSetCharModsCallback(w, char_mods);
			glfwSetCursorEnterCallback(w, cursor_enter);
			glfwSetCursorPosCallback(w, cursor_pos);
			glfwSetDropCallback(w, drop);
			glfwSetFramebufferSizeCallback(w, framebuffer_size);
			glfwSetKeyCallback(w, key);
			glfwSetMouseButtonCallback(w, mouse_button);
			glfwSetScrollCallback(w, scroll);
			glfwSetWindowCloseCallback(w, window_close);
			glfwSetWindowContentScaleCallback(w, window_content_scale);
			glfwSetWindowFocusCallback(w, window_focus);
			glfwSetWindowIconifyCallback(w, window_iconify);
			glfwSetWindowMaximizeCallback(w, window_maximize);
			glfwSetWindowPosCallback(w, window_pos);
			glfwSetWindowRefreshCallback(w, window_refresh);
			glfwSetWindowSizeCallback(w, window_size);
		}
	}
}
