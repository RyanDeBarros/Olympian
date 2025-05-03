#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Events.h"

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

		class Gamepad
		{
			GLFWgamepadstate g;
			int c;

		public:
			EventHandler<GamepadEventData> handler;

			Gamepad(int controller) : c(controller) { glfwGetGamepadState(controller, &g); }

			int controller() const { return c; }
			bool connected() const { return glfwJoystickPresent(c); }
			bool has_mapping() const { return glfwJoystickIsGamepad(c); }
			int button_state(int button) const { return g.buttons[button]; }
			float axis_state(int axis) const { return g.axes[axis]; }
			int dpad_state(unsigned int dpad = 0) const { int count; const unsigned char* hats = glfwGetJoystickHats(c, &count); return dpad < (unsigned int)count ? hats[dpad] : 0; }
			const char* name() const { return glfwGetJoystickName(c); }
			const char* identifier() const { return glfwGetJoystickGUID(c); }
			void set_handler() { glfwSetJoystickUserPointer(c, this); }
		};

		extern void init_joystick_handler();
	}
}
