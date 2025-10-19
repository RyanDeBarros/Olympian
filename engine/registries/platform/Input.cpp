#include "Input.h"

#include "core/context/Platform.h"
#include "core/util/Logger.h"
#include "registries/Loader.h"

namespace oly::reg
{
	static void load_modifier_base(input::ModifierBase& modifier, TOMLNode mnode)
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
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized swizzle value \"" << swizz << "\"." << LOG.nl;
		}

		if (!parse_float(mnode["multiplier"], modifier.multiplier.x))
			if (!parse_vec(mnode["multiplier"], reinterpret_cast<glm::vec2&>(modifier.multiplier)))
				parse_vec(mnode["multiplier"], modifier.multiplier);

		if (auto invert = mnode["invert"].as_array())
		{
			for (size_t i = 0; i < 3; ++i)
			{
				if (invert->size() >= i + 1)
				{
					if (auto inv = invert->get_as<bool>(i))
						modifier.invert[i] = inv->get();
					else if (auto inv = invert->get_as<int64_t>(i))
						modifier.invert[i] = (bool)inv->get();
				}
			}
		}
	}
		
	static input::Axis0DModifier load_modifier_0d(TOMLNode node)
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
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized conversion value \"" << conv << "\"." << LOG.nl;
		}

		load_modifier_base(modifier, mnode);

		return modifier;
	}

	static input::Axis1DModifier load_modifier_1d(TOMLNode node)
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
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized conversion value \"" << conv << "\"." << LOG.nl;
		}

		load_modifier_base(modifier, mnode);

		return modifier;
	}

	static input::Axis2DModifier load_modifier_2d(TOMLNode node)
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
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized conversion value \"" << conv << "\"." << LOG.nl;
		}

		load_modifier_base(modifier, mnode);

		return modifier;
	}

	static void load_key_binding(TOMLNode node, const std::string& id)
	{
		input::KeyBinding b;
		if (!parse_int(node["key"], b.key))
			return;
		parse_int(node["req_mods"], b.required_key_mods);
		parse_int(node["ban_mods"], b.forbidden_key_mods);
		b.modifier = load_modifier_0d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_mouse_button_binding(TOMLNode node, const std::string& id)
	{
		input::MouseButtonBinding b;
		if (!parse_int(node["button"], b.button))
			return;
		parse_int(node["req_mods"], b.required_button_mods);
		parse_int(node["ban_mods"], b.forbidden_button_mods);
		b.modifier = load_modifier_0d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_button_binding(TOMLNode node, const std::string& id)
	{
		int button;
		if (!parse_int(node["button"], button))
			return;
		input::GamepadButtonBinding b{ .button = (input::GamepadButton)button };
		b.modifier = load_modifier_0d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_axis_1d_binding(TOMLNode node, const std::string& id)
	{
		int axis1d;
		if (!parse_int(node["axis1d"], axis1d))
			return;
		input::GamepadAxis1DBinding b{ .axis = (input::GamepadAxis1D)axis1d };
		parse_float(node["deadzone"], b.deadzone);
		b.modifier = load_modifier_1d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_axis_2d_binding(TOMLNode node, const std::string& id)
	{
		int axis2d;
		if (!parse_int(node["axis2d"], axis2d))
			return;
		input::GamepadAxis2DBinding b{ .axis = (input::GamepadAxis2D)axis2d };
		parse_float(node["deadzone"], b.deadzone);
		b.modifier = load_modifier_2d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_cursor_pos_binding(TOMLNode node, const std::string& id)
	{
		input::CursorPosBinding b{};
		b.modifier = load_modifier_2d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_scroll_binding(TOMLNode node, const std::string& id)
	{
		input::ScrollBinding b{};
		b.modifier = load_modifier_2d(node);
			
		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	void load_signal(TOMLNode node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing input signal [" << (src ? *src : "") << "]." << LOG.nl;
		}

		auto toml_id = node["id"].value<std::string>();
		if (!toml_id)
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Missing \"id\" field." << LOG.endl;
			return;
		}
		auto toml_binding = node["binding"].value<std::string>();
		if (!toml_binding)
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Missing \"binding\" field." << LOG.endl;
			return;
		}

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
		else
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Unrecognized binding value \"" << binding << "\"." << LOG.nl;
			return;
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Input signal [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}
	}

	void load_signal_mapping(TOMLNode node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing input signal mapping [" << (src ? *src : "") << "]." << LOG.nl;
		}

		auto toml_id = node["id"].value<std::string>();
		if (!toml_id)
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Missing \"id\" field." << LOG.endl;
			return;
		}

		if (auto toml_signals = node["signals"].as_array())
		{
			std::vector<std::string> signals;
			for (size_t i = 0; i < toml_signals->size(); ++i)
			{
				if (auto signal = toml_signals->get_as<std::string>(i))
					signals.push_back(signal->get());
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Input signal #" << i << " cannot be parsed as a string." << LOG.nl;
			}
			context::assign_signal_mapping(toml_id.value(), std::move(signals));
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Input signal mapping [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}
	}

	void load_signals(const ResourcePath& file)
	{
		auto toml = load_toml(file);

		auto signals = toml["signal"].as_array();
		if (signals)
			signals->for_each([](auto&& node) { load_signal((TOMLNode)node); });

		auto mappings = toml["mapping"].as_array();
		if (mappings)
			mappings->for_each([](auto&& node) { load_signal_mapping((TOMLNode)node); });
	}
}
