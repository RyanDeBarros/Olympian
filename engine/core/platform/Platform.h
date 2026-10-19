#pragma once

#include "core/platform/Gamepad.h"
#include "core/platform/Window.h"

#include "external/TOML.h"

#include <imp/fixed_vector.hpp>

#include <memory>

namespace oly::platform
{
    struct PlatformSetup
    {
        WindowHint window_hint;
        int window_width = 0;
        int window_height = 0;
        std::string window_title;

        unsigned int num_gamepads = 0;

        glm::ivec2 window_size() const;
    };

    class Platform;

    namespace internal
    {
        extern std::unique_ptr<Platform> create_platform(const PlatformSetup&);
    }

    class Platform
    {
        friend std::unique_ptr<Platform> internal::create_platform(const PlatformSetup&);

    private:
        Window window_;
        imp::fixed_vector<Gamepad> gamepads_;
        int num_gamepads_ = 0;

    public:
        GLenum per_frame_clear_mask = GL_COLOR_BUFFER_BIT;

    private:
        Platform(const PlatformSetup& setup);

    public:
        Platform(const Platform&) = delete;
        Platform(Platform&&) noexcept = delete;

        bool frame();

        const Window& window() const;
        Window& window();
        const Gamepad& gamepad(int i = 0) const;
        Gamepad& gamepad(int i = 0);
    };
}
