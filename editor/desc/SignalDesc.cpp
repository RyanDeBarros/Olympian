#include "SignalDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	const char* MODIFIER_INVERT_SUBLABELS[] = {
		"X",
		"Y",
		"Z"
	};

	ModifierBaseDesc::ModifierBaseDesc() :
		swizzle(detail::Swizzle::None, detail::Key::Swizzle, "Swizzle"),
		multiplier(glm::vec3(1.f, 1.f, 1.f), detail::Key::Multiplier, "Multiplier"),
		invert({ false, false, false }, detail::Key::Invert, "Invert", MODIFIER_INVERT_SUBLABELS)
	{
	}

	Modifier0dDesc::Modifier0dDesc() :
		base(),
		conversion(detail::Axis0dConversion::None, detail::Key::Conversion, "Conversion")
	{
	}

	Modifier1dDesc::Modifier1dDesc() :
		base(),
		conversion(detail::Axis1dConversion::None, detail::Key::Conversion, "Conversion")
	{
	}

	Modifier2dDesc::Modifier2dDesc() :
		base(),
		conversion(detail::Axis2dConversion::None, detail::Key::Conversion, "Conversion")
	{
	}

	KeyDesc::KeyDesc() :
		modifier()
	{
	}

	MouseButtonDesc::MouseButtonDesc() :
		modifier()
	{
	}

	static const GLenum GAMEPAD_BUTTON_VALUES[] = {
		GLFW_GAMEPAD_BUTTON_A,
		GLFW_GAMEPAD_BUTTON_B,
		GLFW_GAMEPAD_BUTTON_X,
		GLFW_GAMEPAD_BUTTON_Y,
		GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,
		GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
		GLFW_GAMEPAD_BUTTON_BACK,
		GLFW_GAMEPAD_BUTTON_START,
		GLFW_GAMEPAD_BUTTON_GUIDE,
		GLFW_GAMEPAD_BUTTON_LEFT_THUMB,
		GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,
		GLFW_GAMEPAD_BUTTON_DPAD_UP,
		GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
		GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
		GLFW_GAMEPAD_BUTTON_DPAD_LEFT
	};

	static const char* GAMEPAD_BUTTON_NAMES[] = {
		"A / Cross",
		"B / Circle",
		"X / Square",
		"Y / Triangle",
		"Left bumper (L1)",
		"Right bumper (R1)",
		"Back",
		"Start",
		"Select / Guide",
		"Left thumb (L3)",
		"Right thumb (R3)",
		"Dpad (up)",
		"Dpad (right)",
		"Dpad (down)",
		"Dpad (left)"
	};

	GamepadButtonDesc::GamepadButtonDesc() :
		button(GLFW_GAMEPAD_BUTTON_A, detail::Key::Button, "Button", GAMEPAD_BUTTON_VALUES, GAMEPAD_BUTTON_NAMES),
		modifier()
	{
	}

	static const GLenum GAMEPAD_AXIS_1D_VALUES[] = {
		GLFW_GAMEPAD_AXIS_LEFT_X,
		GLFW_GAMEPAD_AXIS_LEFT_Y,
		GLFW_GAMEPAD_AXIS_RIGHT_X,
		GLFW_GAMEPAD_AXIS_RIGHT_Y,
		GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,
		GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER,
	};

	static const char* GAMEPAD_AXIS_1D_NAMES[] = {
		"Left stick (X)",
		"Left stick (Y)",
		"Right stick (X)",
		"Right stick (Y)",
		"Left trigger (L2)",
		"Right trigger (R2)",
	};

	GamepadAxis1dDesc::GamepadAxis1dDesc() :
		axis(GLFW_GAMEPAD_AXIS_LEFT_X, detail::Key::Axis1D, "Axis", GAMEPAD_AXIS_1D_VALUES, GAMEPAD_AXIS_1D_NAMES),
		modifier(),
		deadzone(0.f, detail::Key::Deadzone, "Deadzone")
	{
	}

	GamepadAxis2dDesc::GamepadAxis2dDesc() :
		axis(detail::GamepadAxis2D::LeftXY, detail::Key::Axis2D, "Axis"),
		modifier(),
		deadzone(0.f, detail::Key::Deadzone, "Deadzone")
	{
	}

	const detail::Key SignalDesc::modifier_key = detail::Key::Modifier;

	SignalDesc::SignalDesc() :
		id("", detail::Key::ID, "ID"),
		binding(detail::SignalBindingType::Key, detail::Key::Binding, "Binding")//,
		//variant(KeyDesc())
	{
	}

	RouteDesc::RouteDesc() :
		id("", detail::Key::ID, "ID")//,
		//signals({}, detail::Key::Signals, "Signals")
	{
	}

	const detail::Key SignalFullDesc::signals_key = detail::Key::SignalArray;
	const detail::Key SignalFullDesc::routes_key = detail::Key::RoutingArray;
}
