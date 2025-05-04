#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <variant>
#include <unordered_map>
#include <string>
#include <array>

#include "Events.h"
#include "util/FixedVector.h"

namespace oly
{
	namespace input
	{
		struct CharEventData
		{
			unsigned int codepoint;
		};

		struct CharModsEventData
		{
			unsigned int codepoint;
			int mods;
		};

		struct CursorEnterEventData
		{
			bool entered;
		};

		struct CursorPosEventData
		{
			double x, y;
		};

		struct PathDropEventData
		{
			int count;
			const char** paths;
		};

		struct FramebufferResizeEventData
		{
			int w, h;
		};

		struct KeyEventData
		{
			int key;
			int scancode;
			int action;
			int mods;
		};

		struct MouseButtonEventData
		{
			int button;
			int action;
			int mods;
		};

		struct ScrollEventData
		{
			double xoff, yoff;
		};

		struct WindowCloseEventData
		{
		};

		struct WindowContentScaleEventData
		{
			float x, y;
		};

		struct WindowFocusEventData
		{
			bool focused;
		};

		struct WindowIconifyEventData
		{
			bool iconified;
		};

		struct WindowMaximizeEventData
		{
			bool maximized;
		};

		struct WindowPosEventData
		{
			int x, y;
		};

		struct WindowRefreshEventData
		{
		};

		struct WindowResizeEventData
		{
			int w, h;
		};

		extern void init_handlers(GLFWwindow* w);

		struct GamepadEventData
		{
			int jevent;
		};

		struct GamepadButton
		{
			static constexpr int LAST = GLFW_GAMEPAD_BUTTON_LAST;

		private:
			int v;

		public:
			explicit GamepadButton(int v) { this->v = glm::clamp(v, 0, LAST); }
			operator int() const { return v; }

			static const GamepadButton A;
			static const GamepadButton B;
			static const GamepadButton X;
			static const GamepadButton Y;
			static const GamepadButton CROSS;
			static const GamepadButton CIRCLE;
			static const GamepadButton SQUARE;
			static const GamepadButton TRIANGLE;
			static const GamepadButton LEFT_BUMPER;
			static const GamepadButton RIGHT_BUMPER;
			static const GamepadButton BACK;
			static const GamepadButton START;
			static const GamepadButton GUIDE;
			static const GamepadButton LEFT_THUMB;
			static const GamepadButton RIGHT_THUMB;
			static const GamepadButton DPAD_UP;
			static const GamepadButton DPAD_RIGHT;
			static const GamepadButton DPAD_DOWN;
			static const GamepadButton DPAD_LEFT;
		};

		inline const GamepadButton GamepadButton::A = GamepadButton(GLFW_GAMEPAD_BUTTON_A);
		inline const GamepadButton GamepadButton::B = GamepadButton(GLFW_GAMEPAD_BUTTON_B);
		inline const GamepadButton GamepadButton::X = GamepadButton(GLFW_GAMEPAD_BUTTON_X);
		inline const GamepadButton GamepadButton::Y = GamepadButton(GLFW_GAMEPAD_BUTTON_Y);
		inline const GamepadButton GamepadButton::CROSS = GamepadButton(GLFW_GAMEPAD_BUTTON_CROSS);
		inline const GamepadButton GamepadButton::CIRCLE = GamepadButton(GLFW_GAMEPAD_BUTTON_CIRCLE);
		inline const GamepadButton GamepadButton::SQUARE = GamepadButton(GLFW_GAMEPAD_BUTTON_SQUARE);
		inline const GamepadButton GamepadButton::TRIANGLE = GamepadButton(GLFW_GAMEPAD_BUTTON_TRIANGLE);
		inline const GamepadButton GamepadButton::LEFT_BUMPER = GamepadButton(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER);
		inline const GamepadButton GamepadButton::RIGHT_BUMPER = GamepadButton(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER);
		inline const GamepadButton GamepadButton::BACK = GamepadButton(GLFW_GAMEPAD_BUTTON_BACK);
		inline const GamepadButton GamepadButton::START = GamepadButton(GLFW_GAMEPAD_BUTTON_START);
		inline const GamepadButton GamepadButton::GUIDE = GamepadButton(GLFW_GAMEPAD_BUTTON_GUIDE);
		inline const GamepadButton GamepadButton::LEFT_THUMB = GamepadButton(GLFW_GAMEPAD_BUTTON_LEFT_THUMB);
		inline const GamepadButton GamepadButton::RIGHT_THUMB = GamepadButton(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB);
		inline const GamepadButton GamepadButton::DPAD_UP = GamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_UP);
		inline const GamepadButton GamepadButton::DPAD_RIGHT = GamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT);
		inline const GamepadButton GamepadButton::DPAD_DOWN = GamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_DOWN);
		inline const GamepadButton GamepadButton::DPAD_LEFT = GamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_LEFT);

		struct GamepadAxis1D
		{
			static constexpr int LAST = GLFW_GAMEPAD_AXIS_LAST;

		private:
			int v;

		public:
			explicit GamepadAxis1D(int v) { this->v = glm::clamp(v, 0, LAST); }
			operator int() const { return v; }

			static const GamepadAxis1D LEFT_X;
			static const GamepadAxis1D LEFT_Y;
			static const GamepadAxis1D RIGHT_X;
			static const GamepadAxis1D RIGHT_Y;
			static const GamepadAxis1D LEFT_TRIGGER;
			static const GamepadAxis1D RIGHT_TRIGGER;
		};

		inline const GamepadAxis1D GamepadAxis1D::LEFT_X = GamepadAxis1D(GLFW_GAMEPAD_AXIS_LEFT_X);
		inline const GamepadAxis1D GamepadAxis1D::LEFT_Y = GamepadAxis1D(GLFW_GAMEPAD_AXIS_LEFT_Y);
		inline const GamepadAxis1D GamepadAxis1D::RIGHT_X = GamepadAxis1D(GLFW_GAMEPAD_AXIS_RIGHT_X);
		inline const GamepadAxis1D GamepadAxis1D::RIGHT_Y = GamepadAxis1D(GLFW_GAMEPAD_AXIS_RIGHT_Y);
		inline const GamepadAxis1D GamepadAxis1D::LEFT_TRIGGER = GamepadAxis1D(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER);
		inline const GamepadAxis1D GamepadAxis1D::RIGHT_TRIGGER = GamepadAxis1D(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER);

		struct GamepadAxis2D
		{
			static constexpr int LAST = 1;

		private:
			int v;

		public:
			explicit GamepadAxis2D(int v) { this->v = glm::clamp(v, 0, LAST); }
			operator int() const { return v; }

			static const GamepadAxis2D LEFT_XY;
			static const GamepadAxis2D RIGHT_XY;
		};

		inline const GamepadAxis2D GamepadAxis2D::LEFT_XY = GamepadAxis2D(0);
		inline const GamepadAxis2D GamepadAxis2D::RIGHT_XY = GamepadAxis2D(1);

		class Gamepad
		{
			GLFWgamepadstate g;
			int c;

		public:
			EventHandler<GamepadEventData> handler;

			Gamepad(int controller) : c(controller) { poll(); }

			void poll() { glfwGetGamepadState(c, &g); }
			int controller() const { return c; }
			bool connected() const { return glfwJoystickPresent(c); }
			bool has_mapping() const { return glfwJoystickIsGamepad(c); }
			int button_state(GamepadButton button) const { return g.buttons[button]; }
			float axis_1d_state(GamepadAxis1D axis) const { return g.axes[axis]; }
			glm::vec2 axis_2d_state(GamepadAxis2D axis) const
			{
				if (axis == GamepadAxis2D::LEFT_XY)
					return { g.axes[GamepadAxis1D::LEFT_X], g.axes[GamepadAxis1D::LEFT_Y] };
				else if (axis == GamepadAxis2D::RIGHT_XY)
					return { g.axes[GamepadAxis1D::RIGHT_X], g.axes[GamepadAxis1D::RIGHT_Y] };
				else
					return {};
			}
			const char* name() const { return glfwGetJoystickName(c); }
			const char* identifier() const { return glfwGetJoystickGUID(c); }
			void set_handler() { glfwSetJoystickUserPointer(c, this); }
		};

		extern void init_joystick_handler();

		typedef unsigned int SignalID;

		class SignalTable
		{
			SignalID next = 1;
			std::unordered_map<std::string, SignalID> table;

		public:
			SignalID get(const std::string& name)
			{
				auto it = table.find(name);
				if (it != table.end())
					return it->second;
				return table.emplace(name, next++).first->second;
			}
		};

		enum class Phase
		{
			STARTED,
			COMPLETED,
			ONGOING
		};

		struct Signal
		{
			Phase phase;

		private:
			enum class Type
			{
				BOOL,
				AXIS1D,
				AXIS2D
			} type;

			union
			{
				bool bool_value;
				float axis_1d_value;
				glm::vec2 axis_2d_value;
			};

			enum Source
			{
				KEYBOARD = -2,
				MOUSE = -1,
				JOYSTICK = 0,
			} source;

			friend class BindingContext;
			Signal(Phase phase, bool v, Source source) : phase(phase), type(Type::BOOL), bool_value(v), source(source) {}
			Signal(Phase phase, float v, Source source) : phase(phase), type(Type::AXIS1D), axis_1d_value(v), source(source) {}
			Signal(Phase phase, glm::vec2 v, Source source) : phase(phase), type(Type::AXIS2D), axis_2d_value(v), source(source) {}

		public:
			template<typename T>
			T get() const
			{
				static_assert(false, "Signal::get<T>() does not support the invoked type.");
			}
			
			template<>
			bool get<bool>() const
			{
				if (type != Type::BOOL)
					throw Error(ErrorCode::INCOMPATIBLE_SIGNAL_TYPE);
				return bool_value;
			}

			template<>
			float get<float>() const
			{
				if (type != Type::AXIS1D)
					throw Error(ErrorCode::INCOMPATIBLE_SIGNAL_TYPE);
				return axis_1d_value;
			}

			template<>
			glm::vec2 get<glm::vec2>() const
			{
				if (type != Type::AXIS2D)
					throw Error(ErrorCode::INCOMPATIBLE_SIGNAL_TYPE);
				return axis_2d_value;
			}

			bool from_controller() const { return source >= Source::JOYSTICK; }
			int controller_id() const
			{
				if (source < Source::JOYSTICK)
					throw Error(ErrorCode::INVALID_CONTROLLER_ID);
				return source;
			}
		};

		struct InputController
		{
			virtual ~InputController() = default;

			using Handler = bool(InputController::*)(Signal);
		};

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

	class Platform;
	namespace input
	{
		class BindingContext : public EventHandler<KeyEventData>, public EventHandler<MouseButtonEventData>, public EventHandler<CursorPosEventData>, public EventHandler<ScrollEventData>
		{
			std::vector<std::pair<SignalID, KeyBinding>> key_bindings;
			std::vector<std::pair<SignalID, MouseButtonBinding>> mb_bindings;
			std::vector<std::pair<SignalID, GamepadButtonBinding>> gmpd_button_bindings;
			std::vector<std::pair<SignalID, GamepadAxis1DBinding>> gmpd_axis_1d_bindings;
			std::vector<std::pair<SignalID, GamepadAxis2DBinding>> gmpd_axis_2d_bindings;
			std::vector<std::pair<SignalID, CursorPosBinding>> cpos_bindings;
			std::vector<std::pair<SignalID, ScrollBinding>> scroll_bindings;

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
				std::array<ButtonPoll, GamepadButton::LAST + 1> button_polls;
				std::array<Axis1DPoll, GamepadAxis1D::LAST + 1> axis_1d_polls;
				std::array<Axis2DPoll, GamepadAxis2D::LAST + 1> axis_2d_polls;
			};
			FixedVector<GamepadPoll> gamepad_polls;

			struct HandlerRef
			{
				InputController::Handler handler;
				InputController* controller;
			};
			std::unordered_map<SignalID, HandlerRef> handler_map;

			friend class Platform;
			BindingContext(int num_gamepads);
			BindingContext(const BindingContext&) = delete;

			void attach_key(EventHandler<KeyEventData>* parent) { EventHandler<KeyEventData>::attach(parent); }
			void attach_mouse_button(EventHandler<MouseButtonEventData>* parent) { EventHandler<MouseButtonEventData>::attach(parent); }
			void attach_cursor_pos(EventHandler<CursorPosEventData>* parent) { EventHandler<CursorPosEventData>::attach(parent); }
			void attach_scroll(EventHandler<ScrollEventData>* parent) { EventHandler<ScrollEventData>::attach(parent); }
			void detach_key() { EventHandler<KeyEventData>::detach(); }
			void detach_mouse_button() { EventHandler<MouseButtonEventData>::detach(); }
			void detach_cusor_pos() { EventHandler<CursorPosEventData>::detach(); }
			void detach_scroll() { EventHandler<ScrollEventData>::detach(); }

#define REG_SIGNAL(Binding, vector) void register_signal(SignalID signal, Binding binding) { vector.push_back({ signal, binding }); }\
									void unregister_signal(SignalID signal, Binding binding)\
											{ vector.erase(std::find(vector.begin(), vector.end(), std::make_pair(signal, binding))); }

		public:
			REG_SIGNAL(KeyBinding, key_bindings)
			REG_SIGNAL(MouseButtonBinding, mb_bindings)
			REG_SIGNAL(GamepadButtonBinding, gmpd_button_bindings)
			REG_SIGNAL(GamepadAxis1DBinding, gmpd_axis_1d_bindings)
			REG_SIGNAL(GamepadAxis2DBinding, gmpd_axis_2d_bindings)
			REG_SIGNAL(CursorPosBinding, cpos_bindings)
			REG_SIGNAL(ScrollBinding, scroll_bindings)

#undef REG_SIGNAL

			void bind(SignalID signal, InputController::Handler handler, InputController* controller) { handler_map[signal] = { handler, controller }; }
			void unbind(SignalID signal, InputController::Handler handler, InputController* controller)
			{
				auto it = handler_map.find(signal);
				if (it != handler_map.end() && it->second.handler == handler && it->second.controller == controller)
					handler_map.erase(it);
			}

			// call poll() after glfwPollEvents() but before TIME.sync()
			void poll();

		private:
			void poll_cursor_pos();
			void poll_scroll();
			void poll_gamepad_button(int controller, int button);
			void poll_gamepad_axis_1d(int controller, int axis);
			void poll_gamepad_axis_2d(int controller, int axis);

			bool get_phase(int action, Phase& phase) const;
			bool dispatch(SignalID id, Signal signal) const;

		public:
			virtual bool consume(const KeyEventData& data) override;
			virtual bool consume(const MouseButtonEventData& data) override;
			virtual bool consume(const CursorPosEventData& data) override;
			virtual bool consume(const ScrollEventData& data) override;
		};
	}
}
