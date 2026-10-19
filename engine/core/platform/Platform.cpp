#include "Platform.h"

#include "core/base/Assert.h"

#include "core/context/Platform.h"

namespace oly::platform
{
    glm::ivec2 PlatformSetup::window_size() const
    {
        return { window_width, window_height };
    }

    std::unique_ptr<Platform> internal::create_platform(const PlatformSetup& setup)
    {
        return std::unique_ptr<Platform>(new Platform(setup));
    }

    Platform::Platform(const PlatformSetup& setup)
        : window_(setup.window_width, setup.window_height, setup.window_title.c_str(), setup.window_hint)
        , num_gamepads_(setup.num_gamepads)
        , gamepads_(setup.num_gamepads, 0)
    {
        for (int i = 0; i < num_gamepads_; ++i)
            gamepads_[i] = Gamepad(GLFW_JOYSTICK_1 + i);

        context::input_binding_context().attach_key(&window_.handlers.key);
        context::input_binding_context().attach_mouse_button(&window_.handlers.mouse_button);
        context::input_binding_context().attach_cursor_pos(&window_.handlers.cursor_pos);
        context::input_binding_context().attach_scroll(&window_.handlers.scroll);
    }

    bool Platform::frame()
    {
        window_.swap_buffers();
        glfwPollEvents();
        context::input_binding_context().poll();
        glClear(per_frame_clear_mask);
        return !window_.should_close();
    }

    const Window& Platform::window() const
    {
        return window_;
    }

    Window& Platform::window()
    {
        return window_;
    }

    const Gamepad& Platform::gamepad(int i) const
    {
        return gamepads_[i];
    }

    Gamepad& Platform::gamepad(int i)
    {
        return gamepads_[i];
    }
}
