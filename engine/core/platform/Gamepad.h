#pragma once

#include "core/platform/EventHandler.h"
#include "external/GL.h"
#include "external/GLM.h"

namespace oly
{
	namespace input
	{
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
	}

	namespace platform
	{
		class Gamepad
		{
			GLFWgamepadstate g;
			int c;

		public:
			EventHandler<input::GamepadEventData> handler;

			Gamepad(int controller) : c(controller) { poll(); }

			void poll() { glfwGetGamepadState(c, &g); glfwSetJoystickUserPointer(c, this); }
			int controller() const { return c; }
			bool connected() const { return glfwJoystickPresent(c); }
			bool has_mapping() const { return glfwJoystickIsGamepad(c); }
			int button_state(input::GamepadButton button) const { return g.buttons[button]; }
			float axis_1d_state(input::GamepadAxis1D axis) const { return g.axes[axis]; }
			glm::vec2 axis_2d_state(input::GamepadAxis2D axis) const
			{
				if (axis == input::GamepadAxis2D::LEFT_XY)
					return { g.axes[input::GamepadAxis1D::LEFT_X], g.axes[input::GamepadAxis1D::LEFT_Y] };
				else if (axis == input::GamepadAxis2D::RIGHT_XY)
					return { g.axes[input::GamepadAxis1D::RIGHT_X], g.axes[input::GamepadAxis1D::RIGHT_Y] };
				else
					return {};
			}
			const char* name() const { return glfwGetJoystickName(c); }
			const char* identifier() const { return glfwGetJoystickGUID(c); }
		};
	}

	namespace input
	{
		extern void init_joystick_handler();
	}
}
