#include "Input.h"

#include "core/base/Context.h"
#include "core/util/Time.h"
#include "core/platform/Window.h"

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

		BindingContext::BindingContext(int num_gamepads)
			: gamepad_polls(glm::clamp(num_gamepads, 0, GLFW_JOYSTICK_LAST))
		{
		}

		void BindingContext::poll()
		{
			poll_cursor_pos();
			poll_scroll();

			for (int g = 0; g < (int)gamepad_polls.size(); ++g)
				context::platform().gamepad(g).poll();
			
			for (int g = 0; g < (int)gamepad_polls.size(); ++g)
				for (int i = 0; i <= GamepadButton::LAST; ++i)
					poll_gamepad_button(g, i);

			for (int g = 0; g < (int)gamepad_polls.size(); ++g)
				for (int i = 0; i <= GamepadAxis1D::LAST; ++i)
					poll_gamepad_axis_1d(g, i);

			for (int g = 0; g < (int)gamepad_polls.size(); ++g)
				for (int i = 0; i <= GamepadAxis2D::LAST; ++i)
					poll_gamepad_axis_2d(g, i);
		}

		void BindingContext::poll_cursor_pos()
		{
			if (cpos_poll.moving && (TIME.now<double>() > cpos_poll.callback_time))
			{
				cpos_poll.moving = false;
				double x, y;
				glfwGetCursorPos(context::platform().window(), &x, &y);
				Signal signal(Phase::COMPLETED, { (float)x, (float)y }, Signal::Source::MOUSE);
				for (const auto& binding : cpos_bindings)
					if (dispatch(binding.first, signal))
						break;
			}
		}

		void BindingContext::poll_scroll()
		{
			if (scroll_poll.moving && (TIME.now<double>() > scroll_poll.callback_time))
			{
				scroll_poll.moving = false;
				Signal signal(Phase::COMPLETED, glm::vec2{}, Signal::Source::MOUSE);
				for (const auto& binding : scroll_bindings)
					if (dispatch(binding.first, signal))
						break;
			}
		}

		void BindingContext::poll_gamepad_button(int controller, int button)
		{
			int state = context::platform().gamepad(controller).button_state(GamepadButton(button));
			ButtonPoll& bpoll = gamepad_polls[controller].button_polls[button];
			int prev_action = bpoll.action;
			bpoll.action = state;

			Phase phase;
			if (prev_action == GLFW_PRESS)
			{
				if (state == GLFW_PRESS)
					phase = Phase::ONGOING;
				else
					phase = Phase::COMPLETED;
			}
			else
			{
				if (state == GLFW_PRESS)
					phase = Phase::STARTED;
				else
					return;
			}

			Signal signal(phase, state != GLFW_RELEASE, Signal::Source(Signal::Source::JOYSTICK + controller));
			for (const auto& binding : gmpd_button_bindings)
				if (binding.second.matches(GamepadButton(button)) && dispatch(binding.first, signal))
					break;
		}

		void BindingContext::poll_gamepad_axis_1d(int controller, int axis)
		{
			float state = context::platform().gamepad(controller).axis_1d_state(GamepadAxis1D(axis));
			Axis1DPoll& apoll = gamepad_polls[controller].axis_1d_polls[axis];
			float prev_axis = apoll.axis;
			apoll.axis = state;

			Phase phase;
			if (prev_axis != state)
			{
				if (apoll.moving)
					phase = Phase::ONGOING;
				else
				{
					phase = Phase::STARTED;
					apoll.moving = true;
				}
			}
			else
			{
				if (apoll.moving)
				{
					phase = Phase::COMPLETED;
					apoll.moving = false;
				}
				else
					return;
			}

			Signal signal(phase, state, Signal::Source(Signal::Source::JOYSTICK + controller));
			for (const auto& binding : gmpd_axis_1d_bindings)
				if (binding.second.matches(GamepadAxis1D(axis), state) && dispatch(binding.first, signal))
					break;
		}

		void BindingContext::poll_gamepad_axis_2d(int controller, int axis)
		{
			glm::vec2 state = context::platform().gamepad(controller).axis_2d_state(GamepadAxis2D(axis));
			Axis2DPoll& apoll = gamepad_polls[controller].axis_2d_polls[axis];
			glm::vec2 prev_axis = apoll.axis;
			apoll.axis = state;

			Phase phase;
			if (prev_axis != state)
			{
				if (apoll.moving)
					phase = Phase::ONGOING;
				else
				{
					phase = Phase::STARTED;
					apoll.moving = true;
				}
			}
			else
			{
				if (apoll.moving)
				{
					phase = Phase::COMPLETED;
					apoll.moving = false;
				}
				else
					return;
			}

			Signal signal(phase, state, Signal::Source(Signal::Source::JOYSTICK + controller));
			for (const auto& binding : gmpd_axis_2d_bindings)
				if (binding.second.matches(GamepadAxis2D(axis), state) && dispatch(binding.first, signal))
					break;
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

		bool BindingContext::dispatch(SignalID id, Signal signal) const
		{
			auto it = handler_map.find(id);
			return it != handler_map.end() && (it->second.controller->*it->second.handler)(signal);
		}

		bool BindingContext::consume(const KeyEventData& data)
		{
			Phase phase;
			if (!get_phase(data.action, phase))
				return false;
			Signal signal(phase, phase != Phase::COMPLETED, Signal::Signal::KEYBOARD);
			for (const auto& binding : key_bindings)
			{
				if (binding.second.matches(data.key, data.mods) && dispatch(binding.first, signal))
					return true;
			}
			return false;
		}

		bool BindingContext::consume(const MouseButtonEventData& data)
		{
			Phase phase;
			if (!get_phase(data.action, phase))
				return false;
			Signal signal(phase, phase != Phase::COMPLETED, Signal::Signal::MOUSE);
			for (const auto& binding : mb_bindings)
			{
				if (binding.second.matches(data.button, data.mods) && dispatch(binding.first, signal))
					return true;
			}
			return false;
		}

		bool BindingContext::consume(const CursorPosEventData& data)
		{
			Phase phase;
			if (cpos_poll.moving)
				phase = Phase::ONGOING;
			else
				phase = Phase::STARTED;
			cpos_poll.moving = true;
			cpos_poll.callback_time = TIME.now<double>();

			Signal signal(phase, { (float)data.x, (float)data.y }, Signal::Signal::MOUSE);
			for (const auto& binding : cpos_bindings)
			{
				if (dispatch(binding.first, signal))
					return true;
			}
			return false;
		}

		bool BindingContext::consume(const ScrollEventData& data)
		{
			Phase phase;
			if (scroll_poll.moving)
				phase = Phase::ONGOING;
			else
				phase = Phase::STARTED;
			scroll_poll.moving = true;
			scroll_poll.callback_time = TIME.now<double>();

			Signal signal(phase, { (float)data.xoff, (float)data.yoff }, Signal::Signal::MOUSE);
			for (const auto& binding : scroll_bindings)
			{
				if (dispatch(binding.first, signal))
					return true;
			}
			return false;
		}
	}
}
