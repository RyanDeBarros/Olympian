#pragma once

// TODO v9.3 move InputListener to imtk: return GLFW values for input enums, and leave conversions here

#include "definitions/enums/GamepadAxis2D.h"
#include "definitions/enums/KeyInput.h"
#include "definitions/enums/MouseButton.h"

#include <optional>

#include <imtk.hpp>

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

		static imtk::item_result DrawKeyListener(ListenMode& mode, std::optional<detail::KeyInput>& input);
		static imtk::item_result DrawMouseButtonListener(ListenMode& mode, std::optional<detail::MouseButton>& input);
		static imtk::item_result DrawGamepadButtonListener(ListenMode& mode, std::optional<GLenum>& input);
		static imtk::item_result DrawGamepadAxis1DListener(ListenMode& mode, std::optional<GLenum>& input);
		static imtk::item_result DrawGamepadAxis2DListener(ListenMode& mode, std::optional<detail::GamepadAxis2D>& input);
	};
}
