#include "SignalDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	const char* MODIFIER_INVERT_SUBLABELS[] = {
		"X",
		"Y",
		"Z"
	};

	ModifierBaseDesc::ModifierBaseDesc(imtk::datapath_link link) :
		link(std::move(link)),
		swizzle(IMTK_DATAPATH_SUBLINK(subpaths.swizzle), detail::Swizzle::None, detail::Key::Swizzle, "Swizzle"),
		multiplier(IMTK_DATAPATH_SUBLINK(subpaths.multiplier), glm::vec3(1.f, 1.f, 1.f), detail::Key::Multiplier, "Multiplier"),
		invert(IMTK_DATAPATH_SUBLINK(subpaths.invert), { false, false, false }, detail::Key::Invert, "Invert", MODIFIER_INVERT_SUBLABELS, true)
	{
	}

	Modifier0dDesc::Modifier0dDesc(imtk::datapath_link link) :
		link(std::move(link)),
		base(IMTK_DATAPATH_SUBLINK(subpaths.base)),
		conversion(IMTK_DATAPATH_SUBLINK(subpaths.conversion), detail::Axis0dConversion::None, detail::Key::Conversion, "Conversion")
	{
	}

	Modifier1dDesc::Modifier1dDesc(imtk::datapath_link link) :
		link(std::move(link)),
		base(IMTK_DATAPATH_SUBLINK(subpaths.base)),
		conversion(IMTK_DATAPATH_SUBLINK(subpaths.conversion), detail::Axis1dConversion::None, detail::Key::Conversion, "Conversion")
	{
	}

	Modifier2dDesc::Modifier2dDesc(imtk::datapath_link link) :
		link(std::move(link)),
		base(IMTK_DATAPATH_SUBLINK(subpaths.base)),
		conversion(IMTK_DATAPATH_SUBLINK(subpaths.conversion), detail::Axis2dConversion::None, detail::Key::Conversion, "Conversion")
	{
	}

	KeyDesc::KeyDesc(imtk::datapath_link link) :
		link(std::move(link)),
		key(IMTK_DATAPATH_SUBLINK(subpaths.key), detail::KEY_INPUT_DEFAULT, detail::Key::Key, "Key button", detail::KEY_INPUT_VALUES, detail::KEY_INPUT_NAMES),
		required_mods(IMTK_DATAPATH_SUBLINK(subpaths.required_mods), detail::INPUT_MOD_DEFAULT, detail::Key::RequiredMods, "Required mods", detail::INPUT_MOD_VALUES, detail::INPUT_MOD_NAMES, false),
		forbidden_mods(IMTK_DATAPATH_SUBLINK(subpaths.forbidden_mods), detail::INPUT_MOD_DEFAULT, detail::Key::ForbiddenMods, "Forbidden mods", detail::INPUT_MOD_VALUES, detail::INPUT_MOD_NAMES, false),
		modifier(detail::Key::Modifier, IMTK_DATAPATH_SUBLINK(subpaths.modifier))
	{
	}

	MouseButtonDesc::MouseButtonDesc(imtk::datapath_link link) :
		link(std::move(link)),
		button(IMTK_DATAPATH_SUBLINK(subpaths.button), detail::MOUSE_BUTTON_DEFAULT, detail::Key::Button, "Mouse button", detail::MOUSE_BUTTON_VALUES, detail::MOUSE_BUTTON_NAMES),
		required_mods(IMTK_DATAPATH_SUBLINK(subpaths.required_mods), detail::INPUT_MOD_DEFAULT, detail::Key::RequiredMods, "Required mods", detail::INPUT_MOD_VALUES, detail::INPUT_MOD_NAMES, false),
		forbidden_mods(IMTK_DATAPATH_SUBLINK(subpaths.forbidden_mods), detail::INPUT_MOD_DEFAULT, detail::Key::ForbiddenMods, "Forbidden mods", detail::INPUT_MOD_VALUES, detail::INPUT_MOD_NAMES, false),
		modifier(detail::Key::Modifier, IMTK_DATAPATH_SUBLINK(subpaths.modifier))
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

	GamepadButtonDesc::GamepadButtonDesc(imtk::datapath_link link) :
		link(std::move(link)),
		button(IMTK_DATAPATH_SUBLINK(subpaths.button), GLFW_GAMEPAD_BUTTON_A, detail::Key::Button, "Button", GAMEPAD_BUTTON_VALUES, GAMEPAD_BUTTON_NAMES),
		modifier(detail::Key::Modifier, IMTK_DATAPATH_SUBLINK(subpaths.modifier))
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

	GamepadAxis1DDesc::GamepadAxis1DDesc(imtk::datapath_link link) :
		link(std::move(link)),
		axis(IMTK_DATAPATH_SUBLINK(subpaths.axis), GLFW_GAMEPAD_AXIS_LEFT_X, detail::Key::Axis1D, "Axis", GAMEPAD_AXIS_1D_VALUES, GAMEPAD_AXIS_1D_NAMES),
		modifier(detail::Key::Modifier, IMTK_DATAPATH_SUBLINK(subpaths.modifier)),
		deadzone(IMTK_DATAPATH_SUBLINK(subpaths.deadzone), 0.f, detail::Key::Deadzone, "Deadzone")
	{
	}

	GamepadAxis2DDesc::GamepadAxis2DDesc(imtk::datapath_link link) :
		link(std::move(link)),
		axis(IMTK_DATAPATH_SUBLINK(subpaths.axis), detail::GamepadAxis2D::LeftXY, detail::Key::Axis2D, "Axis"),
		modifier(detail::Key::Modifier, IMTK_DATAPATH_SUBLINK(subpaths.modifier)),
		deadzone(IMTK_DATAPATH_SUBLINK(subpaths.deadzone), 0.f, detail::Key::Deadzone, "Deadzone")
	{
	}

	CursorPosDesc::CursorPosDesc(imtk::datapath_link link) :
		link(std::move(link)),
		modifier(detail::Key::Modifier, IMTK_DATAPATH_SUBLINK(subpaths.modifier))
	{
	}

	ScrollDesc::ScrollDesc(imtk::datapath_link link) :
		link(std::move(link)),
		modifier(detail::Key::Modifier, IMTK_DATAPATH_SUBLINK(subpaths.modifier))
	{
	}

	SignalDesc::SignalDesc(imtk::datapath_link link) :
		link(std::move(link)),
		id(IMTK_DATAPATH_SUBLINK(subpaths.id), "", detail::Key::ID, "ID"),
		binding(IMTK_DATAPATH_SUBLINK(subpaths.binding), detail::SignalBindingType::Key, detail::Key::Binding, "Binding"),
		variant(IMTK_DATAPATH_SUBLINK(subpaths.variant), KeyDesc())
	{
	}

	RouteDesc::RouteDesc(imtk::datapath_link link) :
		link(std::move(link)),
		id(IMTK_DATAPATH_SUBLINK(subpaths.id), "", detail::Key::ID, "ID"),
		signals(IMTK_DATAPATH_SUBLINK(subpaths.signals), {}, detail::Key::Signals, "Signals")
	{
	}

	SignalFullDesc::SignalFullDesc(imtk::datapath_link link) :
		link(std::move(link)),
		signals(detail::Key::SignalArray, IMTK_DATAPATH_SUBLINK(subpaths.signals)),
		routes(detail::Key::RoutingArray, IMTK_DATAPATH_SUBLINK(subpaths.routes))
	{
	}
}
