#pragma once

#include "core/platform/EventHandler.h"
#include "core/platform/InputController.h"
#include "core/platform/Gamepad.h"
#include "core/platform/WindowEvents.h"

#include "definitions/enums/AxisConversions.h"
#include "definitions/enums/GamepadAxis2D.h"

#include <imp/fixed_vector.hpp>

#include <array>
#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>

namespace oly
{
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
            detail::Swizzle swizzle = detail::Swizzle::None;

            bool operator==(const ModifierBase&) const = default;

            bool modify(bool value) const;
            float modify(float value) const;
            glm::vec2 modify(glm::vec2 value) const;
            glm::vec3 modify(glm::vec3 value) const;
        };

        struct Axis0DModifier
            : public ModifierBase
        {
            detail::Axis0dConversion conversion = detail::Axis0dConversion::None;

            bool operator==(const Axis0DModifier&) const = default;

            Signal signal(Phase phase, bool value, Signal::Source source) const;
        };

        struct Axis1DModifier
            : public ModifierBase
        {
            detail::Axis1dConversion conversion = detail::Axis1dConversion::None;

            bool operator==(const Axis1DModifier&) const = default;

            Signal signal(Phase phase, float value, Signal::Source source) const;
        };

        struct Axis2DModifier
            : public ModifierBase
        {
            detail::Axis2dConversion conversion = detail::Axis2dConversion::None;

            bool operator==(const Axis2DModifier&) const = default;

            Signal signal(Phase phase, glm::vec2 value, Signal::Source source) const;
        };

        struct KeyBinding
        {
            int key;
            int required_key_mods = 0;
            int forbidden_key_mods = 0;
            Axis0DModifier modifier;

            bool operator==(const KeyBinding&) const = default;

            std::optional<Signal> signal(Phase phase, int key, int mods) const;
        };

        struct MouseButtonBinding
        {
            int button;
            int required_button_mods = 0;
            int forbidden_button_mods = 0;
            Axis0DModifier modifier;

            bool operator==(const MouseButtonBinding&) const = default;

            std::optional<Signal> signal(Phase phase, int button, int mods) const;
        };

        struct GamepadButtonBinding
        {
            GamepadButton button;
            Axis0DModifier modifier;

            bool operator==(const GamepadButtonBinding&) const = default;

            std::optional<Signal> signal(Phase phase, GamepadButton button, int controller) const;
        };

        struct GamepadAxis1DBinding
        {
            GamepadAxis1D axis;
            float deadzone = 0.f;
            Axis1DModifier modifier;

            bool operator==(const GamepadAxis1DBinding&) const = default;

            std::optional<Signal> signal(Phase phase, GamepadAxis1D axis, float value, int controller) const;
        };

        struct GamepadAxis2DBinding
        {
            detail::GamepadAxis2D axis;
            float deadzone = 0.0f;
            Axis2DModifier modifier;

            bool operator==(const GamepadAxis2DBinding&) const = default;

            std::optional<Signal> signal(
                Phase phase,
                detail::GamepadAxis2D axis,
                glm::vec2 value,
                int controller) const;
        };

        struct CursorPosBinding
        {
            Axis2DModifier modifier;

            bool operator==(const CursorPosBinding&) const = default;

            Signal signal(Phase phase, glm::vec2 pos) const;
        };

        struct ScrollBinding
        {
            Axis2DModifier modifier;

            bool operator==(const ScrollBinding&) const = default;

            Signal signal(Phase phase, glm::vec2 scroll) const;
        };

        namespace internal
        {
            class InputBindingContext
                : public EventHandler<KeyEventData>
                , public EventHandler<MouseButtonEventData>
                , public EventHandler<CursorPosEventData>
                , public EventHandler<ScrollEventData>
            {
                friend struct InputController;
                friend class platform::Platform;

            private:
                struct CallbackPoll
                {
                    double callback_time = 0.0;
                    bool moving = false;
                };

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
                    std::array<ButtonPoll, GamepadButton::kLast + 1> button_polls;
                    std::array<Axis1DPoll, GamepadAxis1D::kLast + 1> axis_1d_polls;
                    std::array<Axis2DPoll, static_cast<size_t>(detail::GamepadAxis2D::_last) + 1> axis_2d_polls;
                };

                struct ControllerHandler
                {
                    const InputController* controller = nullptr;

                    ControllerHandler(InputController* controller);
                    ControllerHandler(const InputController* controller);
                    virtual ~ControllerHandler() = default;

                    virtual bool invoke(Signal) const = 0;
                };

                struct HandlerRef
                    : public ControllerHandler
                {
                    InputController::Handler handler = nullptr;

                    HandlerRef(InputController& controller, InputController::Handler handler);

                    bool invoke(Signal signal) const override;
                };

                struct ConstHandlerRef
                    : public ControllerHandler
                {
                    InputController::ConstHandler handler = nullptr;

                    ConstHandlerRef(const InputController& controller, InputController::ConstHandler handler);

                    bool invoke(Signal signal) const override;
                };

                std::unordered_map<SignalID, std::vector<KeyBinding>> key_bindings_;
                std::unordered_map<SignalID, std::vector<MouseButtonBinding>> mb_bindings_;
                std::unordered_map<SignalID, std::vector<GamepadButtonBinding>> gmpd_button_bindings_;
                std::unordered_map<SignalID, std::vector<GamepadAxis1DBinding>> gmpd_axis_1d_bindings_;
                std::unordered_map<SignalID, std::vector<GamepadAxis2DBinding>> gmpd_axis_2d_bindings_;
                std::unordered_map<SignalID, std::vector<CursorPosBinding>> cpos_bindings_;
                std::unordered_map<SignalID, std::vector<ScrollBinding>> scroll_bindings_;

                CallbackPoll cpos_poll_;
                CallbackPoll scroll_poll_;
                imp::fixed_vector<GamepadPoll> gamepad_polls_;

                std::unordered_map<SignalID, std::unique_ptr<ControllerHandler>> handler_map_;
                std::unordered_map<const InputController*, std::unordered_set<SignalID>> controller_lut_;

            public:
                InputBindingContext(unsigned int num_gamepads);
                InputBindingContext(const InputBindingContext&) = delete;
                InputBindingContext(InputBindingContext&&) = delete;

            public:
                bool consume(const KeyEventData& data) override;
                bool consume(const MouseButtonEventData& data) override;
                bool consume(const CursorPosEventData& data) override;
                bool consume(const ScrollEventData& data) override;

                // call poll() after glfwPollEvents() but before TIME.sync()
                void poll();

                void register_signal_binding(SignalID signal, const KeyBinding& binding);
                void unregister_signal_binding(SignalID signal, const KeyBinding& binding);

                void register_signal_binding(SignalID signal, const MouseButtonBinding& binding);
                void unregister_signal_binding(SignalID signal, const MouseButtonBinding& binding);
                
                void register_signal_binding(SignalID signal, const GamepadButtonBinding& binding);
                void unregister_signal_binding(SignalID signal, const GamepadButtonBinding& binding);
                
                void register_signal_binding(SignalID signal, const GamepadAxis1DBinding& binding);
                void unregister_signal_binding(SignalID signal, const GamepadAxis1DBinding& binding);
                
                void register_signal_binding(SignalID signal, const GamepadAxis2DBinding& binding);
                void unregister_signal_binding(SignalID signal, const GamepadAxis2DBinding& binding);
                
                void register_signal_binding(SignalID signal, const CursorPosBinding& binding);
                void unregister_signal_binding(SignalID signal, const CursorPosBinding& binding);
                
                void register_signal_binding(SignalID signal, const ScrollBinding& binding);
                void unregister_signal_binding(SignalID signal, const ScrollBinding& binding);

            private:
                void attach_key(EventHandler<KeyEventData>* parent);
                void attach_mouse_button(EventHandler<MouseButtonEventData>* parent);
                void attach_cursor_pos(EventHandler<CursorPosEventData>* parent);
                void attach_scroll(EventHandler<ScrollEventData>* parent);
                void detach_key();
                void detach_mouse_button();
                void detach_cusor_pos();
                void detach_scroll();

                void poll_cursor_pos();
                void poll_scroll();
                void poll_gamepad_button(int controller, int button);
                void poll_gamepad_axis_1d(int controller, int axis);
                void poll_gamepad_axis_2d(int controller, int axis);

                bool get_phase(int action, Phase& phase) const;
                bool dispatch(SignalID id, Signal signal);
            };
        }
    }
}
