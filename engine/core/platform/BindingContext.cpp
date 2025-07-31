#include "BindingContext.h"

#include "core/base/Context.h"
#include "core/util/Time.h"

namespace oly::platform
{
	InputBindingContext::InputBindingContext(unsigned int num_gamepads)
		: gamepad_polls(std::min(num_gamepads, (unsigned int)GLFW_JOYSTICK_LAST))
	{
	}

	void InputBindingContext::poll()
	{
		poll_cursor_pos();
		poll_scroll();

		for (int g = 0; g < (int)gamepad_polls.size(); ++g)
			context::get_platform().gamepad(g).poll();

		for (int g = 0; g < (int)gamepad_polls.size(); ++g)
			for (int i = 0; i <= input::GamepadButton::LAST; ++i)
				poll_gamepad_button(g, i);

		for (int g = 0; g < (int)gamepad_polls.size(); ++g)
			for (int i = 0; i <= input::GamepadAxis1D::LAST; ++i)
				poll_gamepad_axis_1d(g, i);

		for (int g = 0; g < (int)gamepad_polls.size(); ++g)
			for (int i = 0; i <= input::GamepadAxis2D::LAST; ++i)
				poll_gamepad_axis_2d(g, i);
	}

	void InputBindingContext::poll_cursor_pos()
	{
		if (cpos_poll.moving && (TIME.now<double>() > cpos_poll.callback_time))
		{
			cpos_poll.moving = false;
			double x, y;
			glfwGetCursorPos(context::get_platform().window(), &x, &y);
			input::Signal signal(input::Phase::COMPLETED, { (float)x, (float)y }, input::Signal::Source::MOUSE);
			for (const auto& binding : cpos_bindings)
				if (dispatch(binding.first, signal))
					break;
		}
	}

	void InputBindingContext::poll_scroll()
	{
		if (scroll_poll.moving && (TIME.now<double>() > scroll_poll.callback_time))
		{
			scroll_poll.moving = false;
			input::Signal signal(input::Phase::COMPLETED, glm::vec2{}, input::Signal::Source::MOUSE);
			for (const auto& binding : scroll_bindings)
				if (dispatch(binding.first, signal))
					break;
		}
	}

	void InputBindingContext::poll_gamepad_button(int controller, int button)
	{
		int state = context::get_platform().gamepad(controller).button_state(input::GamepadButton(button));
		ButtonPoll& bpoll = gamepad_polls[controller].button_polls[button];
		int prev_action = bpoll.action;
		bpoll.action = state;

		input::Phase phase;
		if (prev_action == GLFW_PRESS)
		{
			if (state == GLFW_PRESS)
				phase = input::Phase::ONGOING;
			else
				phase = input::Phase::COMPLETED;
		}
		else
		{
			if (state == GLFW_PRESS)
				phase = input::Phase::STARTED;
			else
				return;
		}

		for (const auto& [id, bindings] : gmpd_button_bindings)
		{
			for (const input::GamepadButtonBinding& binding : bindings)
			{
				std::optional<input::Signal> signal = binding.signal(phase, input::GamepadButton(button), controller);
				if (signal)
					dispatch(id, *signal);
			}
		}
	}

	void InputBindingContext::poll_gamepad_axis_1d(int controller, int axis)
	{
		float state = context::get_platform().gamepad(controller).axis_1d_state(input::GamepadAxis1D(axis));
		Axis1DPoll& apoll = gamepad_polls[controller].axis_1d_polls[axis];
		float prev_axis = apoll.axis;
		apoll.axis = state;

		input::Phase phase;
		if (prev_axis != state)
		{
			if (apoll.moving)
				phase = input::Phase::ONGOING;
			else
			{
				phase = input::Phase::STARTED;
				apoll.moving = true;
			}
		}
		else
		{
			if (apoll.moving)
			{
				phase = input::Phase::COMPLETED;
				apoll.moving = false;
			}
			else
				return;
		}

		for (const auto& [id, bindings] : gmpd_axis_1d_bindings)
		{
			for (const input::GamepadAxis1DBinding& binding : bindings)
			{
				std::optional<input::Signal> signal = binding.signal(phase, input::GamepadAxis1D(axis), state, controller);
				if (signal)
					dispatch(id, *signal);
			}
		}
	}

	void InputBindingContext::poll_gamepad_axis_2d(int controller, int axis)
	{
		glm::vec2 state = context::get_platform().gamepad(controller).axis_2d_state(input::GamepadAxis2D(axis));
		Axis2DPoll& apoll = gamepad_polls[controller].axis_2d_polls[axis];
		glm::vec2 prev_axis = apoll.axis;
		apoll.axis = state;

		input::Phase phase;
		if (prev_axis != state)
		{
			if (apoll.moving)
				phase = input::Phase::ONGOING;
			else
			{
				phase = input::Phase::STARTED;
				apoll.moving = true;
			}
		}
		else
		{
			if (apoll.moving)
			{
				phase = input::Phase::COMPLETED;
				apoll.moving = false;
			}
			else
				return;
		}

		for (const auto& [id, bindings] : gmpd_axis_2d_bindings)
		{
			for (const input::GamepadAxis2DBinding& binding : bindings)
			{
				std::optional<input::Signal> signal = binding.signal(phase, input::GamepadAxis2D(axis), state, controller);
				if (signal)
					dispatch(id, *signal);
			}
		}
	}

	bool InputBindingContext::get_phase(int action, input::Phase& phase) const
	{
		switch (action)
		{
		case GLFW_PRESS:
			phase = input::Phase::STARTED;
			return true;
		case GLFW_RELEASE:
			phase = input::Phase::COMPLETED;
			return true;
		case GLFW_REPEAT:
			phase = input::Phase::ONGOING;
			return true;
		default:
			return false;
		}
	}

	bool InputBindingContext::dispatch(input::SignalID id, input::Signal signal)
	{
		static const auto invalid_controller = [](auto&& ref) { return !ref.controller; };

		auto it = handler_map.find(id);
		if (it != handler_map.end())
		{
			if (std::visit(invalid_controller, it->second))
			{
				handler_map.erase(it);
				return false;
			}
			else
				return std::visit([signal](auto&& ref) { return (ref.controller.get()->*ref.handler)(signal); }, it->second);
		}
		else
			return false;
	}

	bool InputBindingContext::consume(const input::KeyEventData& data)
	{
		input::Phase phase;
		if (!get_phase(data.action, phase))
			return false;

		bool consumed = false;
		for (const auto& [id, bindings] : key_bindings)
		{
			for (const input::KeyBinding& binding : bindings)
			{
				std::optional<input::Signal> signal = binding.signal(phase, data.key, data.mods);
				if (signal && dispatch(id, *signal))
					consumed = true;
			}
		}
		return consumed;
	}

	bool InputBindingContext::consume(const input::MouseButtonEventData& data)
	{
		input::Phase phase;
		if (!get_phase(data.action, phase))
			return false;

		bool consumed = false;
		for (const auto& [id, bindings] : mb_bindings)
		{
			for (const input::MouseButtonBinding& binding : bindings)
			{
				std::optional<input::Signal> signal = binding.signal(phase, data.button, data.mods);
				if (signal && dispatch(id, *signal))
					consumed = true;
			}
		}
		return consumed;
	}

	bool InputBindingContext::consume(const input::CursorPosEventData& data)
	{
		input::Phase phase;
		if (cpos_poll.moving)
			phase = input::Phase::ONGOING;
		else
			phase = input::Phase::STARTED;
		cpos_poll.moving = true;
		cpos_poll.callback_time = TIME.now<double>();

		bool consumed = false;
		for (const auto& [id, bindings] : cpos_bindings)
		{
			for (const input::CursorPosBinding& binding : bindings)
			{
				if (dispatch(id, binding.signal(phase, { (float)data.x, (float)data.y })))
					consumed = true;
			}
		}
		return consumed;
	}

	bool InputBindingContext::consume(const input::ScrollEventData& data)
	{
		input::Phase phase;
		if (scroll_poll.moving)
			phase = input::Phase::ONGOING;
		else
			phase = input::Phase::STARTED;
		scroll_poll.moving = true;
		scroll_poll.callback_time = TIME.now<double>();

		bool consumed = false;
		for (const auto& [id, bindings] : scroll_bindings)
		{
			for (const input::ScrollBinding& binding : bindings)
			{
				if (dispatch(id, binding.signal(phase, { (float)data.xoff, (float)data.yoff })))
					consumed = true;
			}
		}
		return consumed;
	}
}
