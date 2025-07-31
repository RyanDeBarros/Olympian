#include "Gamepad.h"

namespace oly
{
	static void joystick(int jid, int jevent)
	{
		static_cast<platform::Gamepad*>(glfwGetJoystickUserPointer(jid))->handler.handle({ jevent });
	}

	void init_joystick_handler()
	{
		glfwSetJoystickCallback(&joystick);
	}
}
