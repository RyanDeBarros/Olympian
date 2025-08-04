#pragma once

#include <memory>

#include "external/TOML.h"
#include "core/platform/Window.h"
#include "core/platform/Gamepad.h"

namespace oly::platform
{
	struct PlatformSetup
	{
		WindowHint window_hint;
		int window_width = 0, window_height = 0;
		std::string window_title;

		unsigned int num_gamepads = 0;

		glm::ivec2 window_size() const { return { window_width, window_height }; }
	};

	class Platform;

	namespace internal
	{
		extern std::unique_ptr<Platform> create_platform(const PlatformSetup&);
	}

	class Platform
	{
		Window _window;
		FixedVector<Gamepad> _gamepads;
		int _num_gamepads = 0;

		friend std::unique_ptr<Platform> internal::create_platform(const PlatformSetup&);
		Platform(const PlatformSetup& setup);

	public:
		Platform(const Platform&) = delete;
		Platform(Platform&&) noexcept = delete;

		GLenum per_frame_clear_mask = GL_COLOR_BUFFER_BIT;
		bool frame();

		const Window& window() const { return _window; }
		Window& window() { return _window; }
		const Gamepad& gamepad(int i = 0) const { return _gamepads[i]; }
		Gamepad& gamepad(int i = 0) { return _gamepads[i]; }
	};
}
