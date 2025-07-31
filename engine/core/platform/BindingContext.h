#pragma once

#include "core/platform/Signal.h"
#include "core/platform/Gamepad.h"
#include "core/platform/WindowEvents.h"

namespace oly
{
	class InputController
	{
		OLY_SOFT_REFERENCE_BASE_DECLARATION(InputController);

	public:
		virtual ~InputController() = default;

		using Handler = bool(InputController::*)(input::Signal);
		using ConstHandler = bool(InputController::*)(input::Signal) const;
	};

#define OLY_INPUT_CONTROLLER_HEADER(Class)\
	OLY_SOFT_REFERENCE_PUBLIC(Class)

	namespace input
	{
		struct KeyBinding
		{
			int key;
			int required_mods = 0;
			int forbidden_mods = 0;

			bool matches(int key, int mods) const
			{
				if (key != this->key)
					return false;
				if ((mods & required_mods) != required_mods)
					return false;
				if ((mods & forbidden_mods) != 0)
					return false;
				return true;
			}

			bool operator==(const KeyBinding&) const = default;
		};

		struct MouseButtonBinding
		{
			int button;
			int required_mods = 0;
			int forbidden_mods = 0;

			bool matches(int button, int mods) const
			{
				if (button != this->button)
					return false;
				if ((mods & required_mods) != required_mods)
					return false;
				if ((mods & forbidden_mods) != 0)
					return false;
				return true;
			}

			bool operator==(const MouseButtonBinding&) const = default;
		};

		struct GamepadButtonBinding
		{
			GamepadButton button;

			bool matches(GamepadButton button) const
			{
				return button == this->button;
			}

			bool operator==(const GamepadButtonBinding&) const = default;
		};

		struct GamepadAxis1DBinding
		{
			GamepadAxis1D axis;
			float deadzone = 0.0f;

			bool matches(GamepadAxis1D axis, float value) const
			{
				return axis == this->axis && glm::abs(value) >= glm::abs(deadzone);
			}

			bool operator==(const GamepadAxis1DBinding&) const = default;
		};

		struct GamepadAxis2DBinding
		{
			GamepadAxis2D axis;
			float deadzone = 0.0f;

			bool matches(GamepadAxis2D axis, glm::vec2 value) const
			{
				return axis == this->axis && glm::dot(value, value) >= deadzone * deadzone;
			}

			bool operator==(const GamepadAxis2DBinding&) const = default;
		};

		struct CursorPosBinding
		{
			bool operator==(const CursorPosBinding&) const = default;
		};

		struct ScrollBinding
		{
			bool operator==(const ScrollBinding&) const = default;
		};
	}

	namespace platform
	{
		class InputBindingContext : public EventHandler<input::KeyEventData>, public EventHandler<input::MouseButtonEventData>,
			public EventHandler<input::CursorPosEventData>, public EventHandler<input::ScrollEventData>
		{
			std::vector<std::pair<input::SignalID, input::KeyBinding>> key_bindings;
			std::vector<std::pair<input::SignalID, input::MouseButtonBinding>> mb_bindings;
			std::vector<std::pair<input::SignalID, input::GamepadButtonBinding>> gmpd_button_bindings;
			std::vector<std::pair<input::SignalID, input::GamepadAxis1DBinding>> gmpd_axis_1d_bindings;
			std::vector<std::pair<input::SignalID, input::GamepadAxis2DBinding>> gmpd_axis_2d_bindings;
			std::vector<std::pair<input::SignalID, input::CursorPosBinding>> cpos_bindings;
			std::vector<std::pair<input::SignalID, input::ScrollBinding>> scroll_bindings;

			struct CallbackPoll
			{
				double callback_time = 0.0;
				bool moving = false;
			};
			CallbackPoll cpos_poll, scroll_poll;

			struct ButtonPoll
			{
				int action = GLFW_RELEASE;
			};
			struct Axis1DPoll
			{
				float axis = 0.0f;
				bool moving = false;
			};
			struct Axis2DPoll
			{
				glm::vec2 axis = { 0.0f, 0.0f };
				bool moving = false;
			};
			struct GamepadPoll
			{
				std::array<ButtonPoll, input::GamepadButton::LAST + 1> button_polls;
				std::array<Axis1DPoll, input::GamepadAxis1D::LAST + 1> axis_1d_polls;
				std::array<Axis2DPoll, input::GamepadAxis2D::LAST + 1> axis_2d_polls;
			};
			FixedVector<GamepadPoll> gamepad_polls;

			struct HandlerRef
			{
				InputController::Handler handler = nullptr;
				SoftReference<InputController> controller = nullptr;
			};
			struct ConstHandlerRef
			{
				InputController::ConstHandler handler = nullptr;
				ConstSoftReference<InputController> controller = nullptr;
			};

			std::unordered_map<input::SignalID, std::variant<HandlerRef, ConstHandlerRef>> handler_map;

			friend class Platform;
			InputBindingContext(int num_gamepads);
			InputBindingContext(const InputBindingContext&) = delete;
			InputBindingContext(InputBindingContext&&) = delete;

			void attach_key(EventHandler<input::KeyEventData>* parent) { EventHandler<input::KeyEventData>::attach(parent); }
			void attach_mouse_button(EventHandler<input::MouseButtonEventData>* parent) { EventHandler<input::MouseButtonEventData>::attach(parent); }
			void attach_cursor_pos(EventHandler<input::CursorPosEventData>* parent) { EventHandler<input::CursorPosEventData>::attach(parent); }
			void attach_scroll(EventHandler<input::ScrollEventData>* parent) { EventHandler<input::ScrollEventData>::attach(parent); }
			void detach_key() { EventHandler<input::KeyEventData>::detach(); }
			void detach_mouse_button() { EventHandler<input::MouseButtonEventData>::detach(); }
			void detach_cusor_pos() { EventHandler<input::CursorPosEventData>::detach(); }
			void detach_scroll() { EventHandler<input::ScrollEventData>::detach(); }

#define REG_SIGNAL(Binding, vector) void register_signal(input::SignalID signal, input::Binding binding) { vector.push_back({ signal, binding }); }\
									void unregister_signal(input::SignalID signal, input::Binding binding)\
											{ vector.erase(std::find(vector.begin(), vector.end(), std::make_pair(signal, binding))); }

		public:
			REG_SIGNAL(KeyBinding, key_bindings);
			REG_SIGNAL(MouseButtonBinding, mb_bindings);
			REG_SIGNAL(GamepadButtonBinding, gmpd_button_bindings);
			REG_SIGNAL(GamepadAxis1DBinding, gmpd_axis_1d_bindings);
			REG_SIGNAL(GamepadAxis2DBinding, gmpd_axis_2d_bindings);
			REG_SIGNAL(CursorPosBinding, cpos_bindings);
			REG_SIGNAL(ScrollBinding, scroll_bindings);

#undef REG_SIGNAL

			void bind(input::SignalID signal, InputController::Handler handler, const SoftReference<InputController>& controller) { handler_map[signal] = HandlerRef{ handler, controller }; }
			void bind(input::SignalID signal, InputController::ConstHandler handler, const ConstSoftReference<InputController>& controller) { handler_map[signal] = ConstHandlerRef{ handler, controller }; }
			void unbind(input::SignalID signal, InputController::Handler handler, const SoftReference<InputController>& controller)
			{
				auto it = handler_map.find(signal);
				if (it != handler_map.end())
				{
					if (std::visit([handler, &controller](auto&& ref) {
						if constexpr (visiting_class_is<decltype(ref), HandlerRef>)
							return ref.handler == handler && ref.controller == controller;
						else
							return false;
						}, it->second))
						handler_map.erase(it);
				}
			}
			void unbind(input::SignalID signal, InputController::ConstHandler handler, const ConstSoftReference<InputController>& controller)
			{
				auto it = handler_map.find(signal);
				if (it != handler_map.end())
				{
					if (std::visit([handler, controller](auto&& ref) {
						if constexpr (visiting_class_is<decltype(ref), ConstHandlerRef>)
							return ref.handler == handler && ref.controller == controller;
						else
							return false;
						}, it->second))
						handler_map.erase(it);
				}
			}
			void unbind(input::SignalID signal) { handler_map.erase(signal); }

			// call poll() after glfwPollEvents() but before TIME.sync()
			void poll();

		private:
			void poll_cursor_pos();
			void poll_scroll();
			void poll_gamepad_button(int controller, int button);
			void poll_gamepad_axis_1d(int controller, int axis);
			void poll_gamepad_axis_2d(int controller, int axis);

			bool get_phase(int action, input::Phase& phase) const;
			bool dispatch(input::SignalID id, input::Signal signal);

		public:
			bool consume(const input::KeyEventData& data) override;
			bool consume(const input::MouseButtonEventData& data) override;
			bool consume(const input::CursorPosEventData& data) override;
			bool consume(const input::ScrollEventData& data) override;
		};
	}
}
