#include "BindingContext.h"

#include "core/context/Platform.h"
#include "core/util/Time.h"

namespace oly
{
	InputController::InputController(const InputController& other)
	{
		// don't copy signals
	}
	
	InputController::InputController(InputController&& other) noexcept
		: signals(std::move(other.signals))
	{
		auto& handler_map = context::input_binding_context().handler_map;
		for (input::SignalID signal : signals)
			handler_map.find(signal)->second->controller = this;
	}
	
	InputController::~InputController()
	{
		auto& handler_map = context::input_binding_context().handler_map;
		for (input::SignalID signal : signals)
			handler_map.erase(signal);
	}
	
	InputController& InputController::operator=(const InputController& other)
	{
		if (this != &other)
		{
			auto& handler_map = context::input_binding_context().handler_map;
			for (input::SignalID signal : signals)
				handler_map.erase(signal);
			// don't copy signals
			signals.clear();
		}
		return *this;
	}
	
	InputController& InputController::operator=(InputController&& other) noexcept
	{
		if (this != &other)
		{
			auto& handler_map = context::input_binding_context().handler_map;
			for (input::SignalID signal : signals)
				handler_map.erase(signal);
			signals = std::move(other.signals);
			for (input::SignalID signal : signals)
				handler_map.find(signal)->second->controller = this;
		}
		return *this;
	}

	void InputController::bind(input::SignalID signal, InputController::Handler handler)
	{
		auto& handler_map = context::input_binding_context().handler_map;
		auto it = handler_map.find(signal);
		if (it != handler_map.end())
		{
			const InputController* existing_controller = it->second->controller;
			if (existing_controller != this)
			{
				auto sig = std::find(existing_controller->signals.begin(), existing_controller->signals.end(), signal);
				existing_controller->signals.erase(sig);
				signals.push_back(signal);
			}
			it->second = std::make_unique<input::internal::InputBindingContext::HandlerRef>(*this, handler);
		}
		else
		{
			signals.push_back(signal);
			handler_map[signal] = std::make_unique<input::internal::InputBindingContext::HandlerRef>(*this, handler);
		}
	}

	void InputController::bind(input::SignalID signal, InputController::ConstHandler handler) const
	{
		auto& handler_map = context::input_binding_context().handler_map;
		auto it = handler_map.find(signal);
		if (it != handler_map.end())
		{
			const InputController* existing_controller = it->second->controller;
			if (existing_controller != this)
			{
				auto sig = std::find(existing_controller->signals.begin(), existing_controller->signals.end(), signal);
				existing_controller->signals.erase(sig);
				signals.push_back(signal);
			}
			it->second = std::make_unique<input::internal::InputBindingContext::ConstHandlerRef>(*this, handler);
		}
		else
		{
			signals.push_back(signal);
			handler_map[signal] = std::make_unique<input::internal::InputBindingContext::ConstHandlerRef>(*this, handler);
		}
	}

	void InputController::unbind(input::SignalID signal) const
	{
		auto& handler_map = context::input_binding_context().handler_map;
		auto it = handler_map.find(signal);
		if (it != handler_map.end() && it->second->controller == this)
		{
			auto sig = std::find(signals.begin(), signals.end(), signal);
			signals.erase(sig);
			handler_map.erase(it);
		}
	}

	namespace input
	{
		bool ModifierBase::modify(bool value) const
		{
			if (invert[0])
				value = !value;

			return value;
		}

		float ModifierBase::modify(float value) const
		{
			if (invert[0])
				value = 1.0f - value;

			value *= multiplier.x;

			return value;
		}

		glm::vec2 ModifierBase::modify(glm::vec2 value) const
		{
			if (invert[0])
				value.x = 1.0f - value.x;
			if (invert[1])
				value.y = 1.0f - value.y;

			value.x *= multiplier.x;
			value.y *= multiplier.y;

			if (swizzle == Swizzle::YX)
				std::swap(value.x, value.y);

			return value;
		}

		glm::vec3 ModifierBase::modify(glm::vec3 value) const
		{
			if (invert[0])
				value.x = 1.0f - value.x;
			if (invert[1])
				value.y = 1.0f - value.y;
			if (invert[2])
				value.z = 1.0f - value.z;

			value *= multiplier;

			switch (swizzle)
			{
			case Swizzle::XZY:
				std::swap(value.y, value.z);
				break;
			case Swizzle::YXZ:
				std::swap(value.x, value.y);
				break;
			case Swizzle::YZX:
				std::swap(value.x, value.y);
				std::swap(value.y, value.z);
				break;
			case Swizzle::ZXY:
				std::swap(value.y, value.z);
				std::swap(value.x, value.y);
				break;
			case Swizzle::ZYX:
				std::swap(value.x, value.z);
				break;
			}

			return value;
		}

		Signal Axis0DModifier::signal(Phase phase, bool value, Signal::Source source) const
		{
			switch (conversion)
			{
			case Conversion::NONE:
				return Signal(phase, modify(value), source);
			case Conversion::TO_1D:
				return Signal(phase, modify(float(value)), source);
			case Conversion::TO_2D:
				return Signal(phase, modify(glm::vec2(float(value))), source);
			case Conversion::TO_3D:
				return Signal(phase, modify(glm::vec3(float(value))), source);
			}
			throw Error(ErrorCode::UNSUPPORTED_SWITCH_CASE);
		}

		Signal Axis1DModifier::signal(Phase phase, float value, Signal::Source source) const
		{
			switch (conversion)
			{
			case Conversion::NONE:
				return Signal(phase, modify(value), source);
			case Conversion::TO_0D:
				return Signal(phase, modify(value != 0.0f), source);
			case Conversion::TO_2D:
				return Signal(phase, modify(glm::vec2(value)), source);
			case Conversion::TO_3D:
				return Signal(phase, modify(glm::vec3(value)), source);
			}
			throw Error(ErrorCode::UNSUPPORTED_SWITCH_CASE);
		}

		Signal Axis2DModifier::signal(Phase phase, glm::vec2 value, Signal::Source source) const
		{
			switch (conversion)
			{
			case Conversion::NONE:
				return Signal(phase, modify(value), source);
			case Conversion::TO_0D_X:
				return Signal(phase, modify(value.x != 0.0f), source);
			case Conversion::TO_0D_Y:
				return Signal(phase, modify(value.y != 0.0f), source);
			case Conversion::TO_0D_XY:
				return Signal(phase, modify(value.x != 0.0f || value.y != 0.0f), source);
			case Conversion::TO_1D_X:
				return Signal(phase, modify(value.x), source);
			case Conversion::TO_1D_Y:
				return Signal(phase, modify(value.y), source);
			case Conversion::TO_1D_XY:
				return Signal(phase, modify(glm::length(value)), source);
			case Conversion::TO_3D_0:
				return Signal(phase, modify(glm::vec3(value, 0.0f)), source);
			case Conversion::TO_3D_1:
				return Signal(phase, modify(glm::vec3(value, 1.0f)), source);
			}
			throw Error(ErrorCode::UNSUPPORTED_SWITCH_CASE);
		}

		namespace internal
		{
			InputBindingContext::InputBindingContext(unsigned int num_gamepads)
				: gamepad_polls(std::min(num_gamepads, (unsigned int)GLFW_JOYSTICK_LAST))
			{
			}

			InputBindingContext::~InputBindingContext()
			{
				for (auto& [signal_id, handler] : handler_map)
					handler->controller->signals.clear();
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
				auto it = handler_map.find(id);
				if (it != handler_map.end())
					return it->second->invoke(signal);
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
	}
}
