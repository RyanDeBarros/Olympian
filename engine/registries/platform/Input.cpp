#include "Input.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

// TODO v3 log warnings in all registries when format is incompatible (currently, it's quietly ignored).

// TODO v3 OREPL commands for signal assets
// TODO v3 modifiers for signals, like swizzle/negate/etc.
// TODO v3 allow for key signals to generate non-bool values, like float/glm::vec2

namespace oly::reg
{
	static void load_key_binding(const TOMLNode& node, const std::string& id)
	{
		auto key = node["key"].value<int64_t>();
		if (!key)
			return;
		input::KeyBinding b{ .key = (int)key.value() };
		b.required_mods = (int)node["required mods"].value<int64_t>().value_or(0);
		b.forbidden_mods = (int)node["forbidden mods"].value<int64_t>().value_or(0);

		context::get_platform().binding_context().register_signal(context::get_platform().signal_table().get(id), b);
	}

	static void load_mouse_button_binding(const TOMLNode& node, const std::string& id)
	{
		auto button = node["button"].value<int64_t>();
		if (!button)
			return;
		input::MouseButtonBinding b{ .button = (int)button.value() };
		b.required_mods = (int)node["required mods"].value<int64_t>().value_or(0);
		b.forbidden_mods = (int)node["forbidden mods"].value<int64_t>().value_or(0);

		context::get_platform().binding_context().register_signal(context::get_platform().signal_table().get(id), b);
	}

	static void load_gamepad_button_binding(const TOMLNode& node, const std::string& id)
	{
		auto button = node["button"].value<int64_t>();
		if (!button)
			return;
		input::GamepadButtonBinding b{ .button = (input::GamepadButton)(int)button.value() };

		context::get_platform().binding_context().register_signal(context::get_platform().signal_table().get(id), b);
	}

	static void load_gamepad_axis_1d_binding(const TOMLNode& node, const std::string& id)
	{
		auto axis1d = node["axis1d"].value<int64_t>();
		if (!axis1d)
			return;
		input::GamepadAxis1DBinding b{ .axis = (input::GamepadAxis1D)(int)axis1d.value() };
		b.deadzone = (float)node["deadzone"].value<double>().value_or(0.0);

		context::get_platform().binding_context().register_signal(context::get_platform().signal_table().get(id), b);
	}

	static void load_gamepad_axis_2d_binding(const TOMLNode& node, const std::string& id)
	{
		auto axis2d = node["axis2d"].value<int64_t>();
		if (!axis2d)
			return;
		input::GamepadAxis2DBinding b{ .axis = (input::GamepadAxis2D)(int)axis2d.value() };
		b.deadzone = (float)node["deadzone"].value<double>().value_or(0.0);

		context::get_platform().binding_context().register_signal(context::get_platform().signal_table().get(id), b);
	}

	static void load_cursor_pos_binding(const TOMLNode& node, const std::string& id)
	{
		input::CursorPosBinding b{};

		context::get_platform().binding_context().register_signal(context::get_platform().signal_table().get(id), b);
	}

	static void load_scroll_binding(const TOMLNode& node, const std::string& id)
	{
		input::ScrollBinding b{};
			
		context::get_platform().binding_context().register_signal(context::get_platform().signal_table().get(id), b);
	}

	void load_signal(const TOMLNode& node)
	{
		auto toml_id = node["id"].value<std::string>();
		if (!toml_id)
			return;
		auto toml_binding = node["binding"].value<std::string>();
		if (!toml_binding)
			return;
		const std::string& binding = toml_binding.value();
		if (binding == "key")
			load_key_binding(node, toml_id.value());
		else if (binding == "mouse button")
			load_mouse_button_binding(node, toml_id.value());
		else if (binding == "gamepad button")
			load_gamepad_button_binding(node, toml_id.value());
		else if (binding == "gamepad axis 1d")
			load_gamepad_axis_1d_binding(node, toml_id.value());
		else if (binding == "gamepad axis 2d")
			load_gamepad_axis_2d_binding(node, toml_id.value());
		else if (binding == "cursor pos")
			load_cursor_pos_binding(node, toml_id.value());
		else if (binding == "scroll")
			load_scroll_binding(node, toml_id.value());
	}

	void load_signals(const char* signal_registry_filepath)
	{
		auto toml = load_toml(signal_registry_filepath);
		auto signals = toml["signal"].as_array();
		if (!signals)
			return;
		signals->for_each([](auto&& node) {
			if constexpr (toml::is_table<decltype(node)>)
				load_signal((TOMLNode)node);
			});
	}
}
