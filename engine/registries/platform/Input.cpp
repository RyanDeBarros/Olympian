#include "Input.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

// TODO v3 log warnings in all registries when format is incompatible (currently, it's quietly ignored).

// TODO v3 OREPL commands for signal assets

// TODO v3 add modifier section to signal assets

// TODO v3 in assets, use shorthand and less verbose nomenclature. shift towards a conservative, minimal, custom file format. this is especially convenient for independent files like signals or tilesets or import files, not yet necessarily for compound files like archetypes.

namespace oly::reg
{
	static void load_modifier_base(input::ModifierBase& modifier, const TOMLNode& mnode)
	{
		if (auto swizzle = mnode["swizzle"].value<std::string>())
		{
			std::string swizz = swizzle.value();
			if (swizz == "YX")
				modifier.swizzle = input::ModifierBase::Swizzle::YX;
			else if (swizz == "XZY")
				modifier.swizzle = input::ModifierBase::Swizzle::XZY;
			else if (swizz == "YXZ")
				modifier.swizzle = input::ModifierBase::Swizzle::YXZ;
			else if (swizz == "YZX")
				modifier.swizzle = input::ModifierBase::Swizzle::YZX;
			else if (swizz == "ZXY")
				modifier.swizzle = input::ModifierBase::Swizzle::ZXY;
			else if (swizz == "ZYX")
				modifier.swizzle = input::ModifierBase::Swizzle::ZYX;
		}

		if (!parse_float(mnode, "multiplier", modifier.multiplier.x))
			if (!parse_vec(mnode["multiplier"].as_array(), reinterpret_cast<glm::vec2&>(modifier.multiplier)))
				parse_vec(mnode["multiplier"].as_array(), modifier.multiplier);

		if (auto invert = mnode["invert"].as_array())
		{
			for (size_t i = 0; i < 3; ++i)
			{
				if (invert->size() >= i + 1)
				{
					if (auto inv = invert->get_as<bool>(i))
						modifier.invert[i] = inv->get();
				}
			}
		}
	}
		
	static input::Axis0DModifier load_modifier_0d(const TOMLNode& node)
	{
		input::Axis0DModifier modifier;
		TOMLNode mnode = node["modifier"];

		if (auto conversion = mnode["conversion"].value<std::string>())
		{
			std::string conv = conversion.value();
			if (conv == "TO_1D")
				modifier.conversion = input::Axis0DModifier::Conversion::TO_1D;
			else if (conv == "TO_2D")
				modifier.conversion = input::Axis0DModifier::Conversion::TO_2D;
			else if (conv == "TO_3D")
				modifier.conversion = input::Axis0DModifier::Conversion::TO_3D;
		}

		load_modifier_base(modifier, mnode);

		return modifier;
	}

	static input::Axis1DModifier load_modifier_1d(const TOMLNode& node)
	{
		input::Axis1DModifier modifier;
		TOMLNode mnode = node["modifier"];

		if (auto conversion = mnode["conversion"].value<std::string>())
		{
			std::string conv = conversion.value();
			if (conv == "TO_0D")
				modifier.conversion = input::Axis1DModifier::Conversion::TO_0D;
			else if (conv == "TO_2D")
				modifier.conversion = input::Axis1DModifier::Conversion::TO_2D;
			else if (conv == "TO_3D")
				modifier.conversion = input::Axis1DModifier::Conversion::TO_3D;
		}

		load_modifier_base(modifier, mnode);

		return modifier;
	}

	static input::Axis2DModifier load_modifier_2d(const TOMLNode& node)
	{
		input::Axis2DModifier modifier;
		TOMLNode mnode = node["modifier"];

		if (auto conversion = mnode["conversion"].value<std::string>())
		{
			std::string conv = conversion.value();
			if (conv == "TO_0D_X")
				modifier.conversion = input::Axis2DModifier::Conversion::TO_0D_X;
			else if (conv == "TO_0D_Y")
				modifier.conversion = input::Axis2DModifier::Conversion::TO_0D_Y;
			else if (conv == "TO_0D_XY")
				modifier.conversion = input::Axis2DModifier::Conversion::TO_0D_XY;
			else if (conv == "TO_1D_X")
				modifier.conversion = input::Axis2DModifier::Conversion::TO_1D_X;
			else if (conv == "TO_1D_Y")
				modifier.conversion = input::Axis2DModifier::Conversion::TO_1D_Y;
			else if (conv == "TO_1D_XY")
				modifier.conversion = input::Axis2DModifier::Conversion::TO_1D_XY;
			else if (conv == "TO_3D_0")
				modifier.conversion = input::Axis2DModifier::Conversion::TO_3D_0;
			else if (conv == "TO_3D_1")
				modifier.conversion = input::Axis2DModifier::Conversion::TO_3D_1;
		}

		load_modifier_base(modifier, mnode);

		return modifier;
	}

	static void load_key_binding(const TOMLNode& node, const std::string& id)
	{
		auto key = node["key"].value<int64_t>();
		if (!key)
			return;
		input::KeyBinding b{ .key = (int)key.value() };
		b.required_key_mods = (int)node["req key mods"].value<int64_t>().value_or(0);
		b.forbidden_key_mods = (int)node["ban key mods"].value<int64_t>().value_or(0);
		b.modifier = load_modifier_0d(node);

		context::get_platform().binding_context().register_signal_binding(context::get_platform().signal_table().get(id), b);
	}

	static void load_mouse_button_binding(const TOMLNode& node, const std::string& id)
	{
		auto button = node["button"].value<int64_t>();
		if (!button)
			return;
		input::MouseButtonBinding b{ .button = (int)button.value() };
		b.required_button_mods = (int)node["req btn mods"].value<int64_t>().value_or(0);
		b.forbidden_button_mods = (int)node["ban btn mods"].value<int64_t>().value_or(0);
		b.modifier = load_modifier_0d(node);

		context::get_platform().binding_context().register_signal_binding(context::get_platform().signal_table().get(id), b);
	}

	static void load_gamepad_button_binding(const TOMLNode& node, const std::string& id)
	{
		auto button = node["button"].value<int64_t>();
		if (!button)
			return;
		input::GamepadButtonBinding b{ .button = (input::GamepadButton)(int)button.value() };
		b.modifier = load_modifier_0d(node);

		context::get_platform().binding_context().register_signal_binding(context::get_platform().signal_table().get(id), b);
	}

	static void load_gamepad_axis_1d_binding(const TOMLNode& node, const std::string& id)
	{
		auto axis1d = node["axis1d"].value<int64_t>();
		if (!axis1d)
			return;
		input::GamepadAxis1DBinding b{ .axis = (input::GamepadAxis1D)(int)axis1d.value() };
		b.deadzone = (float)node["deadzone"].value<double>().value_or(0.0);
		b.modifier = load_modifier_1d(node);

		context::get_platform().binding_context().register_signal_binding(context::get_platform().signal_table().get(id), b);
	}

	static void load_gamepad_axis_2d_binding(const TOMLNode& node, const std::string& id)
	{
		auto axis2d = node["axis2d"].value<int64_t>();
		if (!axis2d)
			return;
		input::GamepadAxis2DBinding b{ .axis = (input::GamepadAxis2D)(int)axis2d.value() };
		b.deadzone = (float)node["deadzone"].value<double>().value_or(0.0);
		b.modifier = load_modifier_2d(node);

		context::get_platform().binding_context().register_signal_binding(context::get_platform().signal_table().get(id), b);
	}

	static void load_cursor_pos_binding(const TOMLNode& node, const std::string& id)
	{
		input::CursorPosBinding b{};
		b.modifier = load_modifier_2d(node);

		context::get_platform().binding_context().register_signal_binding(context::get_platform().signal_table().get(id), b);
	}

	static void load_scroll_binding(const TOMLNode& node, const std::string& id)
	{
		input::ScrollBinding b{};
		b.modifier = load_modifier_2d(node);
			
		context::get_platform().binding_context().register_signal_binding(context::get_platform().signal_table().get(id), b);
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
