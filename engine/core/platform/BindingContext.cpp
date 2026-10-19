#include "BindingContext.h"

#include "core/context/Platform.h"
#include "core/util/Time.h"

namespace oly::input
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

        if (swizzle == detail::Swizzle::YX)
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
        case detail::Swizzle::XZY:
            std::swap(value.y, value.z);
            break;
        case detail::Swizzle::YXZ:
            std::swap(value.x, value.y);
            break;
        case detail::Swizzle::YZX:
            std::swap(value.x, value.y);
            std::swap(value.y, value.z);
            break;
        case detail::Swizzle::ZXY:
            std::swap(value.y, value.z);
            std::swap(value.x, value.y);
            break;
        case detail::Swizzle::ZYX:
            std::swap(value.x, value.z);
            break;
        }

        return value;
    }

    Signal Axis0DModifier::signal(Phase phase, bool value, Signal::Source source) const
    {
        switch (conversion)
        {
        case detail::Axis0dConversion::None:
            return Signal(phase, modify(value), source);
        case detail::Axis0dConversion::To1D:
            return Signal(phase, modify(float(value)), source);
        case detail::Axis0dConversion::To2D:
            return Signal(phase, modify(glm::vec2(float(value))), source);
        case detail::Axis0dConversion::To3D:
            return Signal(phase, modify(glm::vec3(float(value))), source);
        }
        throw Error(ErrorCode::UnsupportedSwitchCase);
    }

    Signal Axis1DModifier::signal(Phase phase, float value, Signal::Source source) const
    {
        switch (conversion)
        {
        case detail::Axis1dConversion::None:
            return Signal(phase, modify(value), source);
        case detail::Axis1dConversion::To0D:
            return Signal(phase, modify(value != 0.0f), source);
        case detail::Axis1dConversion::To2D:
            return Signal(phase, modify(glm::vec2(value)), source);
        case detail::Axis1dConversion::To3D:
            return Signal(phase, modify(glm::vec3(value)), source);
        }
        throw Error(ErrorCode::UnsupportedSwitchCase);
    }

    Signal Axis2DModifier::signal(Phase phase, glm::vec2 value, Signal::Source source) const
    {
        switch (conversion)
        {
        case detail::Axis2dConversion::None:
            return Signal(phase, modify(value), source);
        case detail::Axis2dConversion::To0D_X:
            return Signal(phase, modify(value.x != 0.0f), source);
        case detail::Axis2dConversion::To0D_Y:
            return Signal(phase, modify(value.y != 0.0f), source);
        case detail::Axis2dConversion::To0D_XY:
            return Signal(phase, modify(value.x != 0.0f || value.y != 0.0f), source);
        case detail::Axis2dConversion::To1D_X:
            return Signal(phase, modify(value.x), source);
        case detail::Axis2dConversion::To1D_Y:
            return Signal(phase, modify(value.y), source);
        case detail::Axis2dConversion::To1D_XY:
            return Signal(phase, modify(glm::length(value)), source);
        case detail::Axis2dConversion::To3D_0:
            return Signal(phase, modify(glm::vec3(value, 0.0f)), source);
        case detail::Axis2dConversion::To3D_1:
            return Signal(phase, modify(glm::vec3(value, 1.0f)), source);
        }
        throw Error(ErrorCode::UnsupportedSwitchCase);
    }

    std::optional<Signal> KeyBinding::signal(Phase phase, int key, int mods) const
    {
        if (key != this->key)
            return std::nullopt;
        if ((mods & required_key_mods) != required_key_mods)
            return std::nullopt;
        if ((mods & forbidden_key_mods) != 0)
            return std::nullopt;

        return modifier.signal(phase, phase != Phase::Completed, Signal::Signal::Keyboard);
    }

    std::optional<Signal> MouseButtonBinding::signal(Phase phase, int button, int mods) const
    {
        if (button != this->button)
            return std::nullopt;
        if ((mods & required_button_mods) != required_button_mods)
            return std::nullopt;
        if ((mods & forbidden_button_mods) != 0)
            return std::nullopt;

        return modifier.signal(phase, phase != Phase::Completed, Signal::Signal::Mouse);
    }

    std::optional<Signal> GamepadButtonBinding::signal(Phase phase, GamepadButton button, int controller) const
    {
        if (button != this->button)
            return std::nullopt;

        return modifier.signal(
            phase,
            phase != Phase::Completed,
            Signal::Source(Signal::Source::JoystickBase + controller));
    }

    std::optional<Signal> GamepadAxis1DBinding::signal(
        Phase phase,
        GamepadAxis1D axis,
        float value,
        int controller) const
    {
        if (axis != this->axis || glm::abs(value) < glm::abs(deadzone))
            return std::nullopt;

        return modifier.signal(phase, value, Signal::Source(Signal::Source::JoystickBase + controller));
    }

    std::optional<Signal> GamepadAxis2DBinding::signal(
        Phase phase,
        detail::GamepadAxis2D axis,
        glm::vec2 value,
        int controller) const
    {
        if (axis != this->axis || glm::dot(value, value) < deadzone * deadzone)
            return std::nullopt;

        return modifier.signal(phase, value, Signal::Source(Signal::Source::JoystickBase + controller));
    }

    Signal CursorPosBinding::signal(Phase phase, glm::vec2 pos) const
    {
        return modifier.signal(phase, pos, Signal::Signal::Mouse);
    }

    Signal ScrollBinding::signal(Phase phase, glm::vec2 scroll) const
    {
        return modifier.signal(phase, scroll, Signal::Signal::Mouse);
    }

    namespace internal
    {
        InputBindingContext::ControllerHandler::ControllerHandler(InputController* controller)
            : controller(controller)
        {
        }
        
        InputBindingContext::ControllerHandler::ControllerHandler(const InputController* controller)
            : controller(controller)
        {
        }

        InputBindingContext::HandlerRef::HandlerRef(InputController& controller, InputController::Handler handler)
            : ControllerHandler(&controller)
            , handler(handler)
        {
        }

        bool InputBindingContext::HandlerRef::invoke(Signal signal) const
        {
            return (const_cast<InputController*>(controller)->*handler)(signal);
        }

        InputBindingContext::ConstHandlerRef::ConstHandlerRef(
            const InputController& controller,
            InputController::ConstHandler handler)
            : ControllerHandler(&controller)
            , handler(handler)
        {
        }

        bool InputBindingContext::ConstHandlerRef::invoke(Signal signal) const
        {
            return (controller->*handler)(signal);
        }

        InputBindingContext::InputBindingContext(unsigned int num_gamepads)
            : gamepad_polls_(std::min(num_gamepads, (unsigned int)GLFW_JOYSTICK_LAST))
        {
        }

        bool InputBindingContext::consume(const KeyEventData& data)
        {
            Phase phase;
            if (!get_phase(data.action, phase))
                return false;

            bool consumed = false;
            for (const auto& [id, bindings] : key_bindings_)
            {
                for (const KeyBinding& binding : bindings)
                {
                    std::optional<Signal> signal = binding.signal(phase, data.key, data.mods);
                    if (signal && dispatch(id, *signal))
                        consumed = true;
                }
            }
            return consumed;
        }

        bool InputBindingContext::consume(const MouseButtonEventData& data)
        {
            Phase phase;
            if (!get_phase(data.action, phase))
                return false;

            bool consumed = false;
            for (const auto& [id, bindings] : mb_bindings_)
            {
                for (const MouseButtonBinding& binding : bindings)
                {
                    std::optional<Signal> signal = binding.signal(phase, data.button, data.mods);
                    if (signal && dispatch(id, *signal))
                        consumed = true;
                }
            }
            return consumed;
        }

        bool InputBindingContext::consume(const CursorPosEventData& data)
        {
            Phase phase;
            if (cpos_poll_.moving)
                phase = Phase::Ongoing;
            else
                phase = Phase::Started;
            cpos_poll_.moving = true;
            cpos_poll_.callback_time = kTime.now<double>();

            bool consumed = false;
            for (const auto& [id, bindings] : cpos_bindings_)
            {
                for (const CursorPosBinding& binding : bindings)
                {
                    if (dispatch(id, binding.signal(phase, { (float)data.x, (float)data.y })))
                        consumed = true;
                }
            }
            return consumed;
        }

        bool InputBindingContext::consume(const ScrollEventData& data)
        {
            Phase phase;
            if (scroll_poll_.moving)
                phase = Phase::Ongoing;
            else
                phase = Phase::Started;
            scroll_poll_.moving = true;
            scroll_poll_.callback_time = kTime.now<double>();

            bool consumed = false;
            for (const auto& [id, bindings] : scroll_bindings_)
            {
                for (const ScrollBinding& binding : bindings)
                {
                    if (dispatch(id, binding.signal(phase, { (float)data.xoff, (float)data.yoff })))
                        consumed = true;
                }
            }
            return consumed;
        }

        void InputBindingContext::poll()
        {
            poll_cursor_pos();
            poll_scroll();

            for (int g = 0; g < (int)gamepad_polls_.size(); ++g)
                context::get_platform().gamepad(g).poll();

            for (int g = 0; g < (int)gamepad_polls_.size(); ++g)
            {
                for (int i = 0; i <= GamepadButton::kLast; ++i)
                    poll_gamepad_button(g, i);
            }

            for (int g = 0; g < (int)gamepad_polls_.size(); ++g)
            {
                for (int i = 0; i <= GamepadAxis1D::kLast; ++i)
                    poll_gamepad_axis_1d(g, i);
            }

            for (int g = 0; g < (int)gamepad_polls_.size(); ++g)
            {
                static constexpr int first = static_cast<int>(detail::GamepadAxis2D::_FIRST);
                static constexpr int last = static_cast<int>(detail::GamepadAxis2D::_LAST);
                for (int i = first; i <= last; ++i)
                    poll_gamepad_axis_2d(g, i);
            }
        }

        void InputBindingContext::register_signal_binding(SignalID signal, const KeyBinding& binding)
        {
            key_bindings_[signal].push_back(binding);
        }
        
        void InputBindingContext::unregister_signal_binding(SignalID signal, const KeyBinding& binding)
        {
            auto& vector = key_bindings_[signal];
            vector.erase(std::find(vector.begin(), vector.end(), binding));
        }

        void InputBindingContext::register_signal_binding(SignalID signal, const MouseButtonBinding& binding)
        {
            mb_bindings_[signal].push_back(binding);
        }

        void InputBindingContext::unregister_signal_binding(SignalID signal, const MouseButtonBinding& binding)
        {
            auto& vector = mb_bindings_[signal];
            vector.erase(std::find(vector.begin(), vector.end(), binding));
        }

        void InputBindingContext::register_signal_binding(SignalID signal, const GamepadButtonBinding& binding)
        {
            gmpd_button_bindings_[signal].push_back(binding);
        }

        void InputBindingContext::unregister_signal_binding(SignalID signal, const GamepadButtonBinding& binding)
        {
            auto& vector = gmpd_button_bindings_[signal];
            vector.erase(std::find(vector.begin(), vector.end(), binding));
        }

        void InputBindingContext::register_signal_binding(SignalID signal, const GamepadAxis1DBinding& binding)
        {
            gmpd_axis_1d_bindings_[signal].push_back(binding);
        }

        void InputBindingContext::unregister_signal_binding(SignalID signal, const GamepadAxis1DBinding& binding)
        {
            auto& vector = gmpd_axis_1d_bindings_[signal];
            vector.erase(std::find(vector.begin(), vector.end(), binding));
        }

        void InputBindingContext::register_signal_binding(SignalID signal, const GamepadAxis2DBinding& binding)
        {
            gmpd_axis_2d_bindings_[signal].push_back(binding);
        }

        void InputBindingContext::unregister_signal_binding(SignalID signal, const GamepadAxis2DBinding& binding)
        {
            auto& vector = gmpd_axis_2d_bindings_[signal];
            vector.erase(std::find(vector.begin(), vector.end(), binding));
        }

        void InputBindingContext::register_signal_binding(SignalID signal, const CursorPosBinding& binding)
        {
            cpos_bindings_[signal].push_back(binding);
        }

        void InputBindingContext::unregister_signal_binding(SignalID signal, const CursorPosBinding& binding)
        {
            auto& vector = cpos_bindings_[signal];
            vector.erase(std::find(vector.begin(), vector.end(), binding));
        }

        void InputBindingContext::register_signal_binding(SignalID signal, const ScrollBinding& binding)
        {
            scroll_bindings_[signal].push_back(binding);
        }

        void InputBindingContext::unregister_signal_binding(SignalID signal, const ScrollBinding& binding)
        {
            auto& vector = scroll_bindings_[signal];
            vector.erase(std::find(vector.begin(), vector.end(), binding));
        }

        void InputBindingContext::attach_key(EventHandler<KeyEventData>* parent)
        {
            EventHandler<KeyEventData>::attach(parent);
        }
        
        void InputBindingContext::attach_mouse_button(EventHandler<MouseButtonEventData>* parent)
        {
            EventHandler<MouseButtonEventData>::attach(parent);
        }

        void InputBindingContext::attach_cursor_pos(EventHandler<CursorPosEventData>* parent)
        {
            EventHandler<CursorPosEventData>::attach(parent);
        }

        void InputBindingContext::attach_scroll(EventHandler<ScrollEventData>* parent)
        {
            EventHandler<ScrollEventData>::attach(parent);
        }

        void InputBindingContext::detach_key()
        {
            EventHandler<KeyEventData>::detach();
        }

        void InputBindingContext::detach_mouse_button()
        {
            EventHandler<MouseButtonEventData>::detach();
        }

        void InputBindingContext::detach_cusor_pos()
        {
            EventHandler<CursorPosEventData>::detach();
        }

        void InputBindingContext::detach_scroll()
        {
            EventHandler<ScrollEventData>::detach();
        }

        void InputBindingContext::poll_cursor_pos()
        {
            if (cpos_poll_.moving && (kTime.now<double>() > cpos_poll_.callback_time))
            {
                cpos_poll_.moving = false;
                double x, y;
                glfwGetCursorPos(context::get_platform().window(), &x, &y);
                Signal signal(Phase::Completed, { (float)x, (float)y }, Signal::Source::Mouse);
                for (const auto& binding : cpos_bindings_)
                    if (dispatch(binding.first, signal))
                        break;
            }
        }

        void InputBindingContext::poll_scroll()
        {
            if (scroll_poll_.moving && (kTime.now<double>() > scroll_poll_.callback_time))
            {
                scroll_poll_.moving = false;
                Signal signal(Phase::Completed, glm::vec2{}, Signal::Source::Mouse);
                for (const auto& binding : scroll_bindings_)
                    if (dispatch(binding.first, signal))
                        break;
            }
        }

        void InputBindingContext::poll_gamepad_button(int controller, int button)
        {
            int state = context::get_platform().gamepad(controller).button_state(GamepadButton(button));
            ButtonPoll& bpoll = gamepad_polls_[controller].button_polls[button];
            int prev_action = bpoll.action;
            bpoll.action = state;

            Phase phase;
            if (prev_action == GLFW_PRESS)
            {
                if (state == GLFW_PRESS)
                    phase = Phase::Ongoing;
                else
                    phase = Phase::Completed;
            }
            else
            {
                if (state == GLFW_PRESS)
                    phase = Phase::Started;
                else
                    return;
            }

            for (const auto& [id, bindings] : gmpd_button_bindings_)
            {
                for (const GamepadButtonBinding& binding : bindings)
                {
                    std::optional<Signal> signal = binding.signal(phase, GamepadButton(button), controller);
                    if (signal)
                        dispatch(id, *signal);
                }
            }
        }

        void InputBindingContext::poll_gamepad_axis_1d(int controller, int axis)
        {
            float state = context::get_platform().gamepad(controller).axis_1d_state(GamepadAxis1D(axis));
            Axis1DPoll& apoll = gamepad_polls_[controller].axis_1d_polls[axis];
            float prev_axis = apoll.axis;
            apoll.axis = state;

            Phase phase;
            if (prev_axis != state)
            {
                if (apoll.moving)
                    phase = Phase::Ongoing;
                else
                {
                    phase = Phase::Started;
                    apoll.moving = true;
                }
            }
            else
            {
                if (apoll.moving)
                {
                    phase = Phase::Completed;
                    apoll.moving = false;
                }
                else
                    return;
            }

            for (const auto& [id, bindings] : gmpd_axis_1d_bindings_)
            {
                for (const GamepadAxis1DBinding& binding : bindings)
                {
                    std::optional<Signal> signal = binding.signal(phase, GamepadAxis1D(axis), state, controller);
                    if (signal)
                        dispatch(id, *signal);
                }
            }
        }

        void InputBindingContext::poll_gamepad_axis_2d(int controller, int axis)
        {
            glm::vec2 state = context::get_platform().gamepad(controller)
                .axis_2d_state(static_cast<detail::GamepadAxis2D>(axis));

            Axis2DPoll& apoll = gamepad_polls_[controller].axis_2d_polls[axis];
            glm::vec2 prev_axis = apoll.axis;
            apoll.axis = state;

            Phase phase;
            if (prev_axis != state)
            {
                if (apoll.moving)
                    phase = Phase::Ongoing;
                else
                {
                    phase = Phase::Started;
                    apoll.moving = true;
                }
            }
            else
            {
                if (apoll.moving)
                {
                    phase = Phase::Completed;
                    apoll.moving = false;
                }
                else
                    return;
            }

            for (const auto& [id, bindings] : gmpd_axis_2d_bindings_)
            {
                for (const GamepadAxis2DBinding& binding : bindings)
                {
                    std::optional<Signal> signal = binding.signal(
                        phase,
                        static_cast<detail::GamepadAxis2D>(axis),
                        state,
                        controller);

                    if (signal)
                        dispatch(id, *signal);
                }
            }
        }

        bool InputBindingContext::get_phase(int action, Phase& phase) const
        {
            switch (action)
            {
            case GLFW_PRESS:
                phase = Phase::Started;
                return true;
            case GLFW_RELEASE:
                phase = Phase::Completed;
                return true;
            case GLFW_REPEAT:
                phase = Phase::Ongoing;
                return true;
            default:
                return false;
            }
        }

        bool InputBindingContext::dispatch(SignalID id, Signal signal)
        {
            auto it = handler_map_.find(id);
            if (it != handler_map_.end())
                return it->second->invoke(signal);
            else
                return false;
        }
    }
}
