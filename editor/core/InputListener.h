#pragma once

#include "definitions/enums/GamepadAxis2D.h"
#include "definitions/enums/KeyInput.h"
#include "definitions/enums/MouseButton.h"

#include "external/GL.h"

#include <imgui.h>

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

		static std::optional<detail::KeyInput> DrawKeyListener(ListenMode& mode);
		static std::optional<detail::MouseButton> DrawMouseButtonListener(ListenMode& mode);
		static std::optional<GLenum> DrawGamepadButtonListener(ListenMode& mode);
		static std::optional<GLenum> DrawGamepadAxis1DListener(ListenMode& mode);
		static std::optional<detail::GamepadAxis2D> DrawGamepadAxis2DListener(ListenMode& mode);
	};
}
