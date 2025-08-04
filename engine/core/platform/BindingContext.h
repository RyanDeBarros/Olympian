#pragma once

#include "core/platform/Signal.h"
#include "core/platform/Gamepad.h"
#include "core/platform/WindowEvents.h"
#include "core/platform/EventHandler.h"
#include "core/containers/FixedVector.h"

#include <vector>
#include <array>
#include <optional>
#include <memory>

namespace oly
{
	namespace input::internal
	{
		class InputBindingContext;
	}

	class InputController
	{
		friend class input::internal::InputBindingContext;
		mutable std::vector<input::SignalID> signals;

	public:
		InputController() = default;
		InputController(const InputController&);
		InputController(InputController&&) noexcept;
		virtual ~InputController();
		InputController& operator=(const InputController&);
		InputController& operator=(InputController&&) noexcept;

		using Handler = bool(InputController::*)(input::Signal);
		using ConstHandler = bool(InputController::*)(input::Signal) const;

		void bind(input::SignalID signal, InputController::Handler handler);
		void bind(input::SignalID signal, InputController::ConstHandler handler) const;
		void unbind(input::SignalID signal) const;
	};

#define OLY_INPUT_CONTROLLER_HEADER(Class)\
	OLY_SOFT_REFERENCE_PUBLIC(Class)

	namespace platform
	{
		class Platform;
	}

	namespace input
	{
		struct ModifierBase
		{
			std::array<bool, 3> invert = { false, false, false };
			glm::vec3 multiplier = glm::vec3(1.0f);
			enum class Swizzle
			{
				NONE,
				YX,
				XZY,
				YXZ,
				YZX,
				ZXY,
				ZYX
			} swizzle = Swizzle::NONE;

			bool modify(bool value) const;
			float modify(float value) const;
			glm::vec2 modify(glm::vec2 value) const;
			glm::vec3 modify(glm::vec3 value) const;

			bool operator==(const ModifierBase&) const = default;
		};

		struct Axis0DModifier : public ModifierBase
		{
			enum class Conversion
			{
				NONE,
				TO_1D,
				TO_2D,
				TO_3D
			} conversion = Conversion::NONE;

			Signal signal(Phase phase, bool value, Signal::Source source) const;

			bool operator==(const Axis0DModifier&) const = default;
		};

		struct Axis1DModifier : public ModifierBase
		{
			enum class Conversion
			{
				NONE,
				TO_0D,
				TO_2D,
				TO_3D
			} conversion = Conversion::NONE;

			Signal signal(Phase phase, float value, Signal::Source source) const;

			bool operator==(const Axis1DModifier&) const = default;
		};

		struct Axis2DModifier : public ModifierBase
		{
			enum class Conversion
			{
				NONE,
				TO_0D_X,
				TO_0D_Y,
				TO_0D_XY,
				TO_1D_X,
				TO_1D_Y,
				TO_1D_XY,
				TO_3D_0,
				TO_3D_1
			} conversion = Conversion::NONE;

			Signal signal(Phase phase, glm::vec2 value, Signal::Source source) const;

			bool operator==(const Axis2DModifier&) const = default;
		};

		struct KeyBinding
		{
			int key;
			int required_key_mods = 0;
			int forbidden_key_mods = 0;
			Axis0DModifier modifier;

			std::optional<Signal> signal(Phase phase, int key, int mods) const
			{
				if (key != this->key)
					return std::nullopt;
				if ((mods & required_key_mods) != required_key_mods)
					return std::nullopt;
				if ((mods & forbidden_key_mods) != 0)
					return std::nullopt;

				return modifier.signal(phase, phase != input::Phase::COMPLETED, input::Signal::Signal::KEYBOARD);
			}

			bool operator==(const KeyBinding&) const = default;
		};

		struct MouseButtonBinding
		{
			int button;
			int required_button_mods = 0;
			int forbidden_button_mods = 0;
			Axis0DModifier modifier;

			std::optional<Signal> signal(Phase phase, int button, int mods) const
			{
				if (button != this->button)
					return std::nullopt;
				if ((mods & required_button_mods) != required_button_mods)
					return std::nullopt;
				if ((mods & forbidden_button_mods) != 0)
					return std::nullopt;

				return modifier.signal(phase, phase != input::Phase::COMPLETED, input::Signal::Signal::MOUSE);
			}

			bool operator==(const MouseButtonBinding&) const = default;
		};

		struct GamepadButtonBinding
		{
			GamepadButton button;
			Axis0DModifier modifier;

			std::optional<Signal> signal(Phase phase, GamepadButton button, int controller) const
			{
				if (button != this->button)
					return std::nullopt;

				return modifier.signal(phase, phase != input::Phase::COMPLETED, input::Signal::Source(input::Signal::Source::JOYSTICK_BASE + controller));
			}

			bool operator==(const GamepadButtonBinding&) const = default;
		};

		struct GamepadAxis1DBinding
		{
			GamepadAxis1D axis;
			float deadzone = 0.0f;
			Axis1DModifier modifier;

			std::optional<Signal> signal(Phase phase, GamepadAxis1D axis, float value, int controller) const
			{
				if (axis != this->axis || glm::abs(value) < glm::abs(deadzone))
					return std::nullopt;

				return modifier.signal(phase, value, input::Signal::Source(input::Signal::Source::JOYSTICK_BASE + controller));
			}

			bool operator==(const GamepadAxis1DBinding&) const = default;
		};

		struct GamepadAxis2DBinding
		{
			GamepadAxis2D axis;
			float deadzone = 0.0f;
			Axis2DModifier modifier;

			std::optional<Signal> signal(Phase phase, GamepadAxis2D axis, glm::vec2 value, int controller) const
			{
				if (axis != this->axis || glm::dot(value, value) < deadzone * deadzone)
					return std::nullopt;

				return modifier.signal(phase, value, input::Signal::Source(input::Signal::Source::JOYSTICK_BASE + controller));
			}

			bool operator==(const GamepadAxis2DBinding&) const = default;
		};

		struct CursorPosBinding
		{
			Axis2DModifier modifier;

			Signal signal(Phase phase, glm::vec2 pos) const
			{
				return modifier.signal(phase, pos, input::Signal::Signal::MOUSE);
			}

			bool operator==(const CursorPosBinding&) const = default;
		};

		struct ScrollBinding
		{
			Axis2DModifier modifier;

			Signal signal(Phase phase, glm::vec2 scroll) const
			{
				return modifier.signal(phase, scroll, input::Signal::Signal::MOUSE);
			}

			bool operator==(const ScrollBinding&) const = default;
		};

		namespace internal
		{
			class InputBindingContext : public EventHandler<input::KeyEventData>, public EventHandler<input::MouseButtonEventData>,
				public EventHandler<input::CursorPosEventData>, public EventHandler<input::ScrollEventData>
			{
#define BINDING_STRUCTURE(Binding) std::unordered_map<input::SignalID, std::vector<Binding>>

				BINDING_STRUCTURE(input::KeyBinding) key_bindings;
				BINDING_STRUCTURE(input::MouseButtonBinding) mb_bindings;
				BINDING_STRUCTURE(input::GamepadButtonBinding) gmpd_button_bindings;
				BINDING_STRUCTURE(input::GamepadAxis1DBinding) gmpd_axis_1d_bindings;
				BINDING_STRUCTURE(input::GamepadAxis2DBinding) gmpd_axis_2d_bindings;
				BINDING_STRUCTURE(input::CursorPosBinding) cpos_bindings;
				BINDING_STRUCTURE(input::ScrollBinding) scroll_bindings;

#undef BINDING_STRUCTURE

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

				struct ControllerHandler
				{
					const InputController* controller = nullptr;

					ControllerHandler(InputController* controller) : controller(controller) {}
					ControllerHandler(const InputController* controller) : controller(controller) {}
					virtual ~ControllerHandler() = default;

					virtual bool invoke(input::Signal) const = 0;
				};

				struct HandlerRef : ControllerHandler
				{
					InputController::Handler handler = nullptr;

					HandlerRef(InputController& controller, InputController::Handler handler) : ControllerHandler(&controller), handler(handler) {}

					bool invoke(input::Signal signal) const override { return (const_cast<InputController*>(controller)->*handler)(signal); }
				};

				struct ConstHandlerRef : ControllerHandler
				{
					InputController::ConstHandler handler = nullptr;

					ConstHandlerRef(const InputController& controller, InputController::ConstHandler handler) : ControllerHandler(&controller), handler(handler) {}

					bool invoke(input::Signal signal) const override { return (controller->*handler)(signal); }
				};

				friend class InputController;
				std::unordered_map<input::SignalID, std::unique_ptr<ControllerHandler>> handler_map;

			public:
				InputBindingContext(unsigned int num_gamepads);
				InputBindingContext(const InputBindingContext&) = delete;
				InputBindingContext(InputBindingContext&&) = delete;
				~InputBindingContext();

			private:
				friend class platform::Platform;
				void attach_key(EventHandler<input::KeyEventData>* parent) { EventHandler<input::KeyEventData>::attach(parent); }
				void attach_mouse_button(EventHandler<input::MouseButtonEventData>* parent) { EventHandler<input::MouseButtonEventData>::attach(parent); }
				void attach_cursor_pos(EventHandler<input::CursorPosEventData>* parent) { EventHandler<input::CursorPosEventData>::attach(parent); }
				void attach_scroll(EventHandler<input::ScrollEventData>* parent) { EventHandler<input::ScrollEventData>::attach(parent); }
				void detach_key() { EventHandler<input::KeyEventData>::detach(); }
				void detach_mouse_button() { EventHandler<input::MouseButtonEventData>::detach(); }
				void detach_cusor_pos() { EventHandler<input::CursorPosEventData>::detach(); }
				void detach_scroll() { EventHandler<input::ScrollEventData>::detach(); }

#define REGISTER_SIGNAL(Binding, binding_structure)\
			void register_signal_binding(input::SignalID signal, Binding binding) { binding_structure[signal].push_back(binding); }\
			void unregister_signal_binding(input::SignalID signal, Binding binding)\
			{ std::vector<Binding>& vector = binding_structure[signal];\
				vector.erase(std::find(vector.begin(), vector.end(), binding)); }

			public:
				REGISTER_SIGNAL(input::KeyBinding, key_bindings);
				REGISTER_SIGNAL(input::MouseButtonBinding, mb_bindings);
				REGISTER_SIGNAL(input::GamepadButtonBinding, gmpd_button_bindings);
				REGISTER_SIGNAL(input::GamepadAxis1DBinding, gmpd_axis_1d_bindings);
				REGISTER_SIGNAL(input::GamepadAxis2DBinding, gmpd_axis_2d_bindings);
				REGISTER_SIGNAL(input::CursorPosBinding, cpos_bindings);
				REGISTER_SIGNAL(input::ScrollBinding, scroll_bindings);

#undef REGISTER_SIGNAL

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
}
