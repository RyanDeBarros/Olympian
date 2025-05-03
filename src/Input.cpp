#include "Input.h"

#include "rendering/core/Window.h"
#include "Context.h"

namespace oly
{
	namespace input
	{
		static void character(GLFWwindow* window, unsigned int codepoint)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.character.handle({ codepoint });
		}

		static void char_mods(GLFWwindow* window, unsigned int codepoint, int mods)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.char_mods.handle({ codepoint, mods });
		}

		static void cursor_enter(GLFWwindow* window, int entered)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.cursor_enter.handle({ (bool)entered });
		}

		static void cursor_pos(GLFWwindow* window, double x, double y)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.cursor_pos.handle({ x, y });
		}

		static void drop(GLFWwindow* window, int num_paths, const char** paths)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.path_drop.handle({ num_paths, paths });
		}

		static void framebuffer_size(GLFWwindow* window, int w, int h)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.framebuffer_resize.handle({ w, h });
		}

		static void key(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.key.handle({ key, scancode, action, mods });
		}

		static void mouse_button(GLFWwindow* window, int button, int action, int mods)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.mouse_button.handle({ button, action, mods });
		}

		static void scroll(GLFWwindow* window, double xoff, double yoff)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.scroll.handle({ xoff, yoff });
		}

		static void window_close(GLFWwindow* window)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.window_close.handle({});
		}

		static void window_content_scale(GLFWwindow* window, float x_scale, float y_scale)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.window_content_scale.handle({ x_scale, y_scale });
		}

		static void window_focus(GLFWwindow* window, int focused)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.window_focus.handle({ (bool)focused });
		}

		static void window_iconify(GLFWwindow* window, int iconified)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.window_iconify.handle({ (bool)iconified });
		}

		static void window_maximize(GLFWwindow* window, int maximized)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.window_maximize.handle({ (bool)maximized });
		}

		static void window_pos(GLFWwindow* window, int x, int y)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.window_pos.handle({ x, y });
		}

		static void window_refresh(GLFWwindow* window)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.window_refresh.handle({});
		}

		static void window_size(GLFWwindow* window, int width, int height)
		{
			static_cast<rendering::Window*>(glfwGetWindowUserPointer(window))->handlers.window_resize.handle({ width, height });
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

		static void joystick(int jid, int jevent)
		{
			static_cast<Gamepad*>(glfwGetJoystickUserPointer(jid))->handler.handle({ jevent });
		}

		void init_joystick_handler()
		{
			glfwSetJoystickCallback(joystick);
		}

		void BindingContext::poll(GLFWwindow* window)
		{
			if (cpos_poll.cursor_moving && (TIME.now<double>() > cpos_poll.callback_time))
			{
				double x, y;
				glfwGetCursorPos(window, &x, &y);
				InputSignal2D signal{};
				signal.v = { (float)x, (float)y };
				signal.phase = Phase::COMPLETED;
				for (const auto& binding : cpos_bindings)
				{
					if (dispatch(binding.first, signal))
						break;
				}
				cpos_poll.cursor_moving = false;
			}
		}

		bool BindingContext::get_phase(int action, Phase& phase) const
		{
			switch (action)
			{
			case GLFW_PRESS:
				phase = Phase::STARTED;
				return true;
			case GLFW_RELEASE:
				phase = Phase::COMPLETED;
				return true;
			case GLFW_REPEAT:
				phase = Phase::ONGOING;
				return true;
			default:
				return false;
			}
		}

		bool BindingContext::dispatch(SignalID id, InputSignal0D signal) const
		{
			auto it = handler_0d_map.find(id);
			if (it != handler_0d_map.end())
				if ((it->second.controller->*it->second.handler)(signal))
					return true;
			return false;
		}

		bool BindingContext::dispatch(SignalID id, InputSignal2D signal) const
		{
			auto it = handler_2d_map.find(id);
			if (it != handler_2d_map.end())
				if ((it->second.controller->*it->second.handler)(signal))
					return true;
			return false;
		}

		bool BindingContext::consume(const KeyEventData& data)
		{
			InputSignal0D signal{};
			if (!get_phase(data.action, signal.phase))
				return false;
			for (const auto& binding : key_bindings)
			{
				if (binding.second.matches(data.key, data.mods) && dispatch(binding.first, signal))
					return true;
			}
			return false;
		}

		bool BindingContext::consume(const MouseButtonEventData& data)
		{
			InputSignal0D signal{};
			if (!get_phase(data.action, signal.phase))
				return false;
			for (const auto& binding : mb_bindings)
			{
				if (binding.second.matches(data.button, data.mods) && dispatch(binding.first, signal))
					return true;
			}
			return false;
		}

		bool BindingContext::consume(const CursorPosEventData& data)
		{
			InputSignal2D signal{};
			signal.v = { (float)data.x, (float)data.y };
			if (cpos_poll.cursor_moving)
				signal.phase = Phase::ONGOING;
			else
				signal.phase = Phase::STARTED;
			cpos_poll.cursor_moving = true;
			cpos_poll.callback_time = TIME.now<double>();

			for (const auto& binding : cpos_bindings)
			{
				if (dispatch(binding.first, signal))
					return true;
			}
			return false;
		}
	}
}
