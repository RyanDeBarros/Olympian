#pragma once

#include "gui/DrawResult.h"

#include "external/GL.h"

#include "definitions/enums/GamepadAxis2D.h"
#include "definitions/enums/KeyInput.h"
#include "definitions/enums/MouseButton.h"

#include <optional>

namespace oly::editor
{
	enum class ListenMode
	{
		None,
		Key,
		MouseButton,
		GamepadButton,
		GamepadAxis1D,
		GamepadAxis2D
	};

	struct InputListener
	{
		static std::optional<detail::KeyInput> ConvertKey(ImGuiKey key);
		static std::optional<detail::MouseButton> ConvertMouseButton(ImGuiMouseButton mb);
		static std::optional<GLenum> ConvertGamepadButton(ImGuiKey key);
		static std::optional<GLenum> ConvertGamepadAxis1D(ImGuiKey key);
		static std::optional<detail::GamepadAxis2D> ConvertGamepadAxis2D(ImGuiKey key);

		static std::optional<detail::KeyInput> ListenForKey();
		static std::optional<detail::MouseButton> ListenForMouseButton();
		static std::optional<GLenum> ListenForGamepadButton();
		static std::optional<GLenum> ListenForGamepadAxis1D();
		static std::optional<detail::GamepadAxis2D> ListenForGamepadAxis2D();

		static DrawResult DrawKeyListener(ListenMode& mode, std::optional<detail::KeyInput>& input);
		static DrawResult DrawMouseButtonListener(ListenMode& mode, std::optional<detail::MouseButton>& input);
		static DrawResult DrawGamepadButtonListener(ListenMode& mode, std::optional<GLenum>& input);
		static DrawResult DrawGamepadAxis1DListener(ListenMode& mode, std::optional<GLenum>& input);
		static DrawResult DrawGamepadAxis2DListener(ListenMode& mode, std::optional<detail::GamepadAxis2D>& input);
	};
}
