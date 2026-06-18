#include "InputListener.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"

namespace oly::editor
{
	std::optional<detail::KeyInput> InputListener::ConvertKey(ImGuiKey key)
	{
		switch (key)
		{
		case ImGuiKey_Tab:
			return GLFW_KEY_TAB;
		case ImGuiKey_LeftArrow:
			return GLFW_KEY_LEFT;
		case ImGuiKey_RightArrow:
			return GLFW_KEY_RIGHT;
		case ImGuiKey_UpArrow:
			return GLFW_KEY_UP;
		case ImGuiKey_DownArrow:
			return GLFW_KEY_DOWN;
		case ImGuiKey_PageUp:
			return GLFW_KEY_PAGE_UP;
		case ImGuiKey_PageDown:
			return GLFW_KEY_PAGE_DOWN;
		case ImGuiKey_Home:
			return GLFW_KEY_HOME;
		case ImGuiKey_End:
			return GLFW_KEY_END;
		case ImGuiKey_Insert:
			return GLFW_KEY_INSERT;
		case ImGuiKey_Delete:
			return GLFW_KEY_DELETE;
		case ImGuiKey_Backspace:
			return GLFW_KEY_BACKSPACE;
		case ImGuiKey_Space:
			return GLFW_KEY_SPACE;
		case ImGuiKey_Enter:
			return GLFW_KEY_ENTER;
		case ImGuiKey_Escape:
			return GLFW_KEY_ESCAPE;
		case ImGuiKey_LeftCtrl:
			return GLFW_KEY_LEFT_CONTROL;
		case ImGuiKey_LeftShift:
			return GLFW_KEY_LEFT_SHIFT;
		case ImGuiKey_LeftAlt:
			return GLFW_KEY_LEFT_ALT;
		case ImGuiKey_LeftSuper:
			return GLFW_KEY_LEFT_SUPER;
		case ImGuiKey_RightCtrl:
			return GLFW_KEY_RIGHT_CONTROL;
		case ImGuiKey_RightShift:
			return GLFW_KEY_RIGHT_SHIFT;
		case ImGuiKey_RightAlt:
			return GLFW_KEY_RIGHT_ALT;
		case ImGuiKey_RightSuper:
			return GLFW_KEY_RIGHT_SUPER;
		case ImGuiKey_Menu:
			return GLFW_KEY_MENU;
		case ImGuiKey_0:
			return GLFW_KEY_0;
		case ImGuiKey_1:
			return GLFW_KEY_1;
		case ImGuiKey_2:
			return GLFW_KEY_2;
		case ImGuiKey_3:
			return GLFW_KEY_3;
		case ImGuiKey_4:
			return GLFW_KEY_4;
		case ImGuiKey_5:
			return GLFW_KEY_5;
		case ImGuiKey_6:
			return GLFW_KEY_6;
		case ImGuiKey_7:
			return GLFW_KEY_7;
		case ImGuiKey_8:
			return GLFW_KEY_8;
		case ImGuiKey_9:
			return GLFW_KEY_9;
		case ImGuiKey_A:
			return GLFW_KEY_A;
		case ImGuiKey_B:
			return GLFW_KEY_B;
		case ImGuiKey_C:
			return GLFW_KEY_C;
		case ImGuiKey_D:
			return GLFW_KEY_D;
		case ImGuiKey_E:
			return GLFW_KEY_E;
		case ImGuiKey_F:
			return GLFW_KEY_F;
		case ImGuiKey_G:
			return GLFW_KEY_G;
		case ImGuiKey_H:
			return GLFW_KEY_H;
		case ImGuiKey_I:
			return GLFW_KEY_I;
		case ImGuiKey_J:
			return GLFW_KEY_J;
		case ImGuiKey_K:
			return GLFW_KEY_K;
		case ImGuiKey_L:
			return GLFW_KEY_L;
		case ImGuiKey_M:
			return GLFW_KEY_M;
		case ImGuiKey_N:
			return GLFW_KEY_N;
		case ImGuiKey_O:
			return GLFW_KEY_O;
		case ImGuiKey_P:
			return GLFW_KEY_P;
		case ImGuiKey_Q:
			return GLFW_KEY_Q;
		case ImGuiKey_R:
			return GLFW_KEY_R;
		case ImGuiKey_S:
			return GLFW_KEY_S;
		case ImGuiKey_T:
			return GLFW_KEY_T;
		case ImGuiKey_U:
			return GLFW_KEY_U;
		case ImGuiKey_V:
			return GLFW_KEY_V;
		case ImGuiKey_W:
			return GLFW_KEY_W;
		case ImGuiKey_X:
			return GLFW_KEY_X;
		case ImGuiKey_Y:
			return GLFW_KEY_Y;
		case ImGuiKey_Z:
			return GLFW_KEY_Z;
		case ImGuiKey_F1:
			return GLFW_KEY_F1;
		case ImGuiKey_F2:
			return GLFW_KEY_F2;
		case ImGuiKey_F3:
			return GLFW_KEY_F3;
		case ImGuiKey_F4:
			return GLFW_KEY_F4;
		case ImGuiKey_F5:
			return GLFW_KEY_F5;
		case ImGuiKey_F6:
			return GLFW_KEY_F6;
		case ImGuiKey_F7:
			return GLFW_KEY_F7;
		case ImGuiKey_F8:
			return GLFW_KEY_F8;
		case ImGuiKey_F9:
			return GLFW_KEY_F9;
		case ImGuiKey_F10:
			return GLFW_KEY_F10;
		case ImGuiKey_F11:
			return GLFW_KEY_F11;
		case ImGuiKey_F12:
			return GLFW_KEY_F12;
		case ImGuiKey_F13:
			return GLFW_KEY_F13;
		case ImGuiKey_F14:
			return GLFW_KEY_F14;
		case ImGuiKey_F15:
			return GLFW_KEY_F15;
		case ImGuiKey_F16:
			return GLFW_KEY_F16;
		case ImGuiKey_F17:
			return GLFW_KEY_F17;
		case ImGuiKey_F18:
			return GLFW_KEY_F18;
		case ImGuiKey_F19:
			return GLFW_KEY_F19;
		case ImGuiKey_F20:
			return GLFW_KEY_F20;
		case ImGuiKey_F21:
			return GLFW_KEY_F21;
		case ImGuiKey_F22:
			return GLFW_KEY_F22;
		case ImGuiKey_F23:
			return GLFW_KEY_F23;
		case ImGuiKey_F24:
			return GLFW_KEY_F24;
		case ImGuiKey_Apostrophe:
			return GLFW_KEY_APOSTROPHE;
		case ImGuiKey_Comma:
			return GLFW_KEY_COMMA;
		case ImGuiKey_Minus:
			return GLFW_KEY_MINUS;
		case ImGuiKey_Period:
			return GLFW_KEY_PERIOD;
		case ImGuiKey_Slash:
			return GLFW_KEY_SLASH;
		case ImGuiKey_Semicolon:
			return GLFW_KEY_SEMICOLON;
		case ImGuiKey_Equal:
			return GLFW_KEY_EQUAL;
		case ImGuiKey_LeftBracket:
			return GLFW_KEY_LEFT_BRACKET;
		case ImGuiKey_Backslash:
			return GLFW_KEY_BACKSLASH;
		case ImGuiKey_RightBracket:
			return GLFW_KEY_RIGHT_BRACKET;
		case ImGuiKey_GraveAccent:
			return GLFW_KEY_GRAVE_ACCENT;
		case ImGuiKey_CapsLock:
			return GLFW_KEY_CAPS_LOCK;
		case ImGuiKey_ScrollLock:
			return GLFW_KEY_SCROLL_LOCK;
		case ImGuiKey_NumLock:
			return GLFW_KEY_NUM_LOCK;
		case ImGuiKey_PrintScreen:
			return GLFW_KEY_PRINT_SCREEN;
		case ImGuiKey_Pause:
			return GLFW_KEY_PAUSE;
		case ImGuiKey_Keypad0:
			return GLFW_KEY_KP_0;
		case ImGuiKey_Keypad1:
			return GLFW_KEY_KP_1;
		case ImGuiKey_Keypad2:
			return GLFW_KEY_KP_2;
		case ImGuiKey_Keypad3:
			return GLFW_KEY_KP_3;
		case ImGuiKey_Keypad4:
			return GLFW_KEY_KP_4;
		case ImGuiKey_Keypad5:
			return GLFW_KEY_KP_5;
		case ImGuiKey_Keypad6:
			return GLFW_KEY_KP_6;
		case ImGuiKey_Keypad7:
			return GLFW_KEY_KP_7;
		case ImGuiKey_Keypad8:
			return GLFW_KEY_KP_8;
		case ImGuiKey_Keypad9:
			return GLFW_KEY_KP_9;
		case ImGuiKey_KeypadDecimal:
			return GLFW_KEY_KP_DECIMAL;
		case ImGuiKey_KeypadDivide:
			return GLFW_KEY_KP_DIVIDE;
		case ImGuiKey_KeypadMultiply:
			return GLFW_KEY_KP_MULTIPLY;
		case ImGuiKey_KeypadSubtract:
			return GLFW_KEY_KP_SUBTRACT;
		case ImGuiKey_KeypadAdd:
			return GLFW_KEY_KP_ADD;
		case ImGuiKey_KeypadEnter:
			return GLFW_KEY_KP_ENTER;
		case ImGuiKey_KeypadEqual:
			return GLFW_KEY_KP_EQUAL;
		case ImGuiKey_Oem102:
			return GLFW_KEY_BACKSLASH;
		default:
			return std::nullopt;
		}
	}

	std::optional<detail::MouseButton> InputListener::ConvertMouseButton(ImGuiMouseButton mb)
	{
		switch (mb)
		{
		case ImGuiMouseButton_Left:
			return GLFW_MOUSE_BUTTON_LEFT;

		case ImGuiMouseButton_Right:
			return GLFW_MOUSE_BUTTON_RIGHT;

		case ImGuiMouseButton_Middle:
			return GLFW_MOUSE_BUTTON_MIDDLE;

		default:
			return std::nullopt;
		}
	}

	std::optional<GLenum> InputListener::ConvertGamepadButton(ImGuiKey key)
	{
		switch (key)
		{
		case ImGuiKey_GamepadStart:
			return GLFW_GAMEPAD_BUTTON_START;
		case ImGuiKey_GamepadBack:
			return GLFW_GAMEPAD_BUTTON_BACK;
		case ImGuiKey_GamepadFaceLeft:
			return GLFW_GAMEPAD_BUTTON_SQUARE;
		case ImGuiKey_GamepadFaceRight:
			return GLFW_GAMEPAD_BUTTON_CIRCLE;
		case ImGuiKey_GamepadFaceUp:
			return GLFW_GAMEPAD_BUTTON_TRIANGLE;
		case ImGuiKey_GamepadFaceDown:
			return GLFW_GAMEPAD_BUTTON_CROSS;
		case ImGuiKey_GamepadDpadLeft:
			return GLFW_GAMEPAD_BUTTON_DPAD_LEFT;
		case ImGuiKey_GamepadDpadRight:
			return GLFW_GAMEPAD_BUTTON_DPAD_RIGHT;
		case ImGuiKey_GamepadDpadUp:
			return GLFW_GAMEPAD_BUTTON_DPAD_UP;
		case ImGuiKey_GamepadDpadDown:
			return GLFW_GAMEPAD_BUTTON_DPAD_DOWN;
		case ImGuiKey_GamepadL1:
			return GLFW_GAMEPAD_BUTTON_LEFT_BUMPER;
		case ImGuiKey_GamepadR1:
			return GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER;
		case ImGuiKey_GamepadL3:
			return GLFW_GAMEPAD_BUTTON_LEFT_THUMB;
		case ImGuiKey_GamepadR3:
			return GLFW_GAMEPAD_BUTTON_RIGHT_THUMB;
		default:
			return std::nullopt;
		}
	}

	std::optional<GLenum> InputListener::ConvertGamepadAxis1D(ImGuiKey key)
	{
		switch (key)
		{
		case ImGuiKey_GamepadL2:
			return GLFW_GAMEPAD_AXIS_LEFT_TRIGGER;
		case ImGuiKey_GamepadR2:
			return GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
		case ImGuiKey_GamepadLStickLeft:
		case ImGuiKey_GamepadLStickRight:
			return GLFW_GAMEPAD_AXIS_LEFT_X;
		case ImGuiKey_GamepadLStickUp:
		case ImGuiKey_GamepadLStickDown:
			return GLFW_GAMEPAD_AXIS_LEFT_Y;
		case ImGuiKey_GamepadRStickLeft:
		case ImGuiKey_GamepadRStickRight:
			return GLFW_GAMEPAD_AXIS_RIGHT_X;
		case ImGuiKey_GamepadRStickUp:
		case ImGuiKey_GamepadRStickDown:
			return GLFW_GAMEPAD_AXIS_RIGHT_Y;
		default:
			return std::nullopt;
		}
	}

	std::optional<detail::GamepadAxis2D> InputListener::ConvertGamepadAxis2D(ImGuiKey key)
	{
		switch (key)
		{
		case ImGuiKey_GamepadLStickLeft:
		case ImGuiKey_GamepadLStickRight:
		case ImGuiKey_GamepadLStickUp:
		case ImGuiKey_GamepadLStickDown:
			return detail::GamepadAxis2D::LeftXY;
		case ImGuiKey_GamepadRStickLeft:
		case ImGuiKey_GamepadRStickRight:
		case ImGuiKey_GamepadRStickUp:
		case ImGuiKey_GamepadRStickDown:
			return detail::GamepadAxis2D::RightXY;
		default:
			return std::nullopt;
		}
	}

	std::optional<detail::KeyInput> InputListener::ListenForKey()
	{
		for (int key = ImGuiKey_Tab; key <= ImGuiKey_Oem102; ++key)
		{
			if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(key)))
			{
				if (auto k = ConvertKey(static_cast<ImGuiKey>(key)))
					return *k;
				else
					MainWindow::Instance().PushNotification(Notification(LogLevel::Error, "Unrecognized input"));

				break;
			}
		}

		return std::nullopt;
	}

	std::optional<detail::MouseButton> InputListener::ListenForMouseButton()
	{
		for (int mb = 0; mb < ImGuiMouseButton_COUNT; ++mb)
		{
			if (ImGui::IsMouseClicked(static_cast<ImGuiMouseButton>(mb)))
			{
				if (auto b = ConvertMouseButton(mb))
					return *b;
				else
					MainWindow::Instance().PushNotification(Notification(LogLevel::Error, "Unrecognized input"));

				break;
			}
		}

		return std::nullopt;
	}

	std::optional<GLenum> InputListener::ListenForGamepadButton()
	{
		for (int key = ImGuiKey_GamepadStart; key <= ImGuiKey_GamepadRStickDown; ++key)
		{
			if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(key)))
			{
				if (auto k = ConvertGamepadButton(static_cast<ImGuiKey>(key)))
					return *k;
				else
					MainWindow::Instance().PushNotification(Notification(LogLevel::Error, "Unrecognized input"));

				break;
			}
		}

		return std::nullopt;
	}

	std::optional<GLenum> InputListener::ListenForGamepadAxis1D()
	{
		for (int key = ImGuiKey_GamepadStart; key <= ImGuiKey_GamepadRStickDown; ++key)
		{
			if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(key)))
			{
				if (auto k = ConvertGamepadAxis1D(static_cast<ImGuiKey>(key)))
					return *k;
				else
					MainWindow::Instance().PushNotification(Notification(LogLevel::Error, "Unrecognized input"));

				break;
			}
		}

		return std::nullopt;
	}

	std::optional<detail::GamepadAxis2D> InputListener::ListenForGamepadAxis2D()
	{
		for (int key = ImGuiKey_GamepadStart; key <= ImGuiKey_GamepadRStickDown; ++key)
		{
			if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(key)))
			{
				if (auto k = ConvertGamepadAxis2D(static_cast<ImGuiKey>(key)))
					return *k;
				else
					MainWindow::Instance().PushNotification(Notification(LogLevel::Error, "Unrecognized input"));

				break;
			}
		}

		return std::nullopt;
	}

	static bool DrawActiveListenButton()
	{
		return ImGui::Button("Listening...");
	}

	static bool DrawInactiveListenButton()
	{
		bool pressed = ImGui::Button("...");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Listen for input");

		return pressed;
	}

	std::optional<detail::KeyInput> InputListener::DrawKeyListener(ListenMode& mode)
	{
		if (mode == ListenMode::Key)
		{
			if (DrawActiveListenButton())
				mode = ListenMode::None;
			else if (auto key = InputListener::ListenForKey())
			{
				mode = ListenMode::None;
				return key;
			}
		}
		else if (DrawInactiveListenButton())
			mode = ListenMode::Key;

		return std::nullopt;
	}

	std::optional<detail::MouseButton> InputListener::DrawMouseButtonListener(ListenMode& mode)
	{

		if (mode == ListenMode::MouseButton)
		{
			DrawActiveListenButton();

			if (auto mb = InputListener::ListenForMouseButton())
			{
				mode = ListenMode::None;
				return mb;
			}
		}
		else if (DrawInactiveListenButton())
			mode = ListenMode::MouseButton;

		return std::nullopt;
	}
	
	std::optional<GLenum> InputListener::DrawGamepadButtonListener(ListenMode& mode)
	{
		if (mode == ListenMode::GamepadButton)
		{
			if (DrawActiveListenButton())
				mode = ListenMode::None;
			else if (auto button = InputListener::ListenForGamepadButton())
			{
				mode = ListenMode::None;
				return button;
			}
		}
		else if (DrawInactiveListenButton())
			mode = ListenMode::GamepadButton;

		return std::nullopt;
	}
	
	std::optional<GLenum> InputListener::DrawGamepadAxis1DListener(ListenMode& mode)
	{
		if (mode == ListenMode::GamepadAxis1D)
		{
			if (DrawActiveListenButton())
				mode = ListenMode::None;
			else if (auto axis = InputListener::ListenForGamepadAxis1D())
			{
				mode = ListenMode::None;
				return axis;
			}
		}
		else if (DrawInactiveListenButton())
			mode = ListenMode::GamepadAxis1D;

		return std::nullopt;
	}
	
	std::optional<detail::GamepadAxis2D> InputListener::DrawGamepadAxis2DListener(ListenMode& mode)
	{
		if (mode == ListenMode::GamepadAxis2D)
		{
			if (DrawActiveListenButton())
				mode = ListenMode::None;
			else if (auto axis = InputListener::ListenForGamepadAxis2D())
			{
				mode = ListenMode::None;
				return axis;
			}
		}
		else if (DrawInactiveListenButton())
			mode = ListenMode::GamepadAxis2D;

		return std::nullopt;
	}
}
