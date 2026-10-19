#include "Gamepad.h"

namespace oly::platform
{
    Gamepad::Gamepad(int controller)
        : controller_(controller)
    {
        poll();
    }

    void Gamepad::poll()
    {
        glfwGetGamepadState(controller_, &state_);
        glfwSetJoystickUserPointer(controller_, this);
    }
    
    int Gamepad::controller() const
    {
        return controller_;
    }
    
    bool Gamepad::connected() const
    {
        return glfwJoystickPresent(controller_);
    }
    
    bool Gamepad::has_mapping() const
    {
        return glfwJoystickIsGamepad(controller_);
    }

    int Gamepad::button_state(input::GamepadButton button) const
    {
        return state_.buttons[button];
    }
    
    float Gamepad::axis_1d_state(input::GamepadAxis1D axis) const
    {
        return state_.axes[axis];
    }
    
    glm::vec2 Gamepad::axis_2d_state(detail::GamepadAxis2D axis) const
    {
        if (axis == detail::GamepadAxis2D::LeftXY)
            return { state_.axes[input::GamepadAxis1D::kLeftX], state_.axes[input::GamepadAxis1D::kLeftY] };
        else if (axis == detail::GamepadAxis2D::RightXY)
            return { state_.axes[input::GamepadAxis1D::kLeftX], state_.axes[input::GamepadAxis1D::kRightY] };
        else
            return {};
    }

    const char* Gamepad::name() const
    {
        return glfwGetJoystickName(controller_);
    }

    const char* Gamepad::identifier() const
    {
        return glfwGetJoystickGUID(controller_);
    }
}

namespace oly::input
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
