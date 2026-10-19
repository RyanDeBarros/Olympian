#pragma once

#include "core/platform/EventHandler.h"

#include "external/GL.h"
#include "external/GLM.h"

#include "definitions/enums/GamepadAxis2D.h"

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
            static constexpr int kLast = GLFW_GAMEPAD_BUTTON_LAST;

        private:
            int v_;

        public:
            explicit constexpr GamepadButton(int v)
                : v_(glm::clamp(v, 0, kLast))
            {
            }

            constexpr operator int() const
            {
                return v_;
            }

            static const GamepadButton kA;
            static const GamepadButton kB;
            static const GamepadButton kX;
            static const GamepadButton kY;
            static const GamepadButton kCross;
            static const GamepadButton kCircle;
            static const GamepadButton kSquare;
            static const GamepadButton kTriangle;
            static const GamepadButton kLeftBumper;
            static const GamepadButton kRightBumper;
            static const GamepadButton kBack;
            static const GamepadButton kStart;
            static const GamepadButton kGuide;
            static const GamepadButton kLeftThumb;
            static const GamepadButton kRightThumb;
            static const GamepadButton kDpadUp;
            static const GamepadButton kDpadRight;
            static const GamepadButton kDpadDown;
            static const GamepadButton kDpadLeft;
        };

        inline constexpr GamepadButton GamepadButton::kA = GamepadButton(GLFW_GAMEPAD_BUTTON_A);
        inline constexpr GamepadButton GamepadButton::kB = GamepadButton(GLFW_GAMEPAD_BUTTON_B);
        inline constexpr GamepadButton GamepadButton::kX = GamepadButton(GLFW_GAMEPAD_BUTTON_X);
        inline constexpr GamepadButton GamepadButton::kY = GamepadButton(GLFW_GAMEPAD_BUTTON_Y);
        inline constexpr GamepadButton GamepadButton::kCross = GamepadButton(GLFW_GAMEPAD_BUTTON_CROSS);
        inline constexpr GamepadButton GamepadButton::kCircle = GamepadButton(GLFW_GAMEPAD_BUTTON_CIRCLE);
        inline constexpr GamepadButton GamepadButton::kSquare = GamepadButton(GLFW_GAMEPAD_BUTTON_SQUARE);
        inline constexpr GamepadButton GamepadButton::kTriangle = GamepadButton(GLFW_GAMEPAD_BUTTON_TRIANGLE);
        inline constexpr GamepadButton GamepadButton::kLeftBumper = GamepadButton(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER);
        inline constexpr GamepadButton GamepadButton::kRightBumper = GamepadButton(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER);
        inline constexpr GamepadButton GamepadButton::kBack = GamepadButton(GLFW_GAMEPAD_BUTTON_BACK);
        inline constexpr GamepadButton GamepadButton::kStart = GamepadButton(GLFW_GAMEPAD_BUTTON_START);
        inline constexpr GamepadButton GamepadButton::kGuide = GamepadButton(GLFW_GAMEPAD_BUTTON_GUIDE);
        inline constexpr GamepadButton GamepadButton::kLeftThumb = GamepadButton(GLFW_GAMEPAD_BUTTON_LEFT_THUMB);
        inline constexpr GamepadButton GamepadButton::kRightThumb = GamepadButton(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB);
        inline constexpr GamepadButton GamepadButton::kDpadUp = GamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_UP);
        inline constexpr GamepadButton GamepadButton::kDpadRight = GamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT);
        inline constexpr GamepadButton GamepadButton::kDpadDown = GamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_DOWN);
        inline constexpr GamepadButton GamepadButton::kDpadLeft = GamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_LEFT);

        struct GamepadAxis1D
        {
            static constexpr int kLast = GLFW_GAMEPAD_AXIS_LAST;

        private:
            int v_;

        public:
            explicit constexpr GamepadAxis1D(int v)
                : v_(glm::clamp(v, 0, kLast))
            {
            }

            constexpr operator int() const
            {
                return v_;
            }

            static const GamepadAxis1D kLeftX;
            static const GamepadAxis1D kLeftY;
            static const GamepadAxis1D kRightX;
            static const GamepadAxis1D kRightY;
            static const GamepadAxis1D kLeftTrigger;
            static const GamepadAxis1D kRightTrigger;
        };

        inline constexpr GamepadAxis1D GamepadAxis1D::kLeftX = GamepadAxis1D(GLFW_GAMEPAD_AXIS_LEFT_X);
        inline constexpr GamepadAxis1D GamepadAxis1D::kLeftY = GamepadAxis1D(GLFW_GAMEPAD_AXIS_LEFT_Y);
        inline constexpr GamepadAxis1D GamepadAxis1D::kRightX = GamepadAxis1D(GLFW_GAMEPAD_AXIS_RIGHT_X);
        inline constexpr GamepadAxis1D GamepadAxis1D::kRightY = GamepadAxis1D(GLFW_GAMEPAD_AXIS_RIGHT_Y);
        inline constexpr GamepadAxis1D GamepadAxis1D::kLeftTrigger = GamepadAxis1D(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER);
        inline constexpr GamepadAxis1D GamepadAxis1D::kRightTrigger = GamepadAxis1D(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER);
    }

    namespace platform
    {
        class Gamepad
        {
            GLFWgamepadstate state_;
            int controller_;

        public:
            EventHandler<input::GamepadEventData> handler;

            Gamepad(int controller);

            void poll();
            int controller() const;
            bool connected() const;
            bool has_mapping() const;

            int button_state(input::GamepadButton button) const;
            float axis_1d_state(input::GamepadAxis1D axis) const;
            glm::vec2 axis_2d_state(detail::GamepadAxis2D axis) const;

            const char* name() const;
            const char* identifier() const;
        };
    }

    namespace input
    {
        extern void init_joystick_handler();
    }
}
