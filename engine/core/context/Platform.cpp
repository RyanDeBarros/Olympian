#include "Platform.h"

#include "core/context/Context.h"
#include "core/base/Errors.h"
#include "core/base/Assert.h"
#include "core/util/Logger.h"
#include "core/util/Loader.h"
#include "core/util/MetaSplitter.h"
#include "core/algorithms/STLUtils.h"
#include "graphics/Camera.h"

namespace oly::context
{
	namespace internal
	{
		std::unique_ptr<platform::Platform> platform;
		std::unique_ptr<input::internal::InputBindingContext> input_binding_context;
		input::SignalTable signal_table;
		input::SignalMappingTable signal_mapping_table;
	}

	struct PlatformOnTerminate
	{
		void operator()() const
		{
			internal::platform.reset();
			internal::input_binding_context.reset();
		}
	};

	void internal::init_platform(TOMLNode node)
	{
		platform::PlatformSetup platform_setup;

		auto toml_window = node["window"];
		if (!toml_window)
		{
			_OLY_ENGINE_LOG_FATAL("CONTEXT") << "Cannot initialize platform: missing \"window\" table." << LOG.nl;
			throw Error(ErrorCode::PlatformInit);
		}

		if (!io::parse_int(toml_window["width"], platform_setup.window_width))
		{
			_OLY_ENGINE_LOG_FATAL("CONTEXT") << "Cannot initialize platform: missing or invalid \"width\" field." << LOG.nl;
			throw Error(ErrorCode::PlatformInit);
		}
		
		if (!io::parse_int(toml_window["height"], platform_setup.window_height))
		{
			_OLY_ENGINE_LOG_FATAL("CONTEXT") << "Cannot initialize platform: missing or invalid \"height\" field." << LOG.nl;
			throw Error(ErrorCode::PlatformInit);
		}

		if (auto title = toml_window["title"].value<std::string>())
			platform_setup.window_title = *title;
		else
		{
			_OLY_ENGINE_LOG_FATAL("CONTEXT") << "Cannot initialize platform: missing or invalid \"title\" fields." << LOG.nl;
			throw Error(ErrorCode::PlatformInit);
		}

		if (auto toml_window_hint = toml_window["window_hint"])
		{
			io::parse_vec(toml_window_hint["clear_color"], platform_setup.window_hint.context.clear_color);
			io::parse_int(toml_window_hint["swap_interval"], platform_setup.window_hint.context.swap_interval);
			io::parse_bool(toml_window_hint["resizable"], platform_setup.window_hint.window.resizable);
			io::parse_bool(toml_window_hint["visible"], platform_setup.window_hint.window.visible);
			io::parse_bool(toml_window_hint["decorated"], platform_setup.window_hint.window.decorated);
			io::parse_bool(toml_window_hint["focused"], platform_setup.window_hint.window.focused);
			io::parse_bool(toml_window_hint["auto_iconify"], platform_setup.window_hint.window.auto_iconify);
			io::parse_bool(toml_window_hint["floating"], platform_setup.window_hint.window.floating);
			io::parse_bool(toml_window_hint["maximized"], platform_setup.window_hint.window.maximized);
			io::parse_bool(toml_window_hint["center_cursor"], platform_setup.window_hint.window.center_cursor);
			io::parse_bool(toml_window_hint["transparent_framebuffer"], platform_setup.window_hint.window.transparent_framebuffer);
			io::parse_bool(toml_window_hint["focus_on_show"], platform_setup.window_hint.window.focus_on_show);
			io::parse_bool(toml_window_hint["scale_to_monitor"], platform_setup.window_hint.window.scale_to_monitor);
			io::parse_bool(toml_window_hint["scale_framebuffer"], platform_setup.window_hint.window.scale_framebuffer);
			io::parse_bool(toml_window_hint["mouse_passthrough"], platform_setup.window_hint.window.mouse_passthrough);
			io::parse_uint(toml_window_hint["position_x"], platform_setup.window_hint.window.position_x);
			io::parse_uint(toml_window_hint["position_y"], platform_setup.window_hint.window.position_y);
			io::parse_int(toml_window_hint["refresh_rate"], platform_setup.window_hint.window.refresh_rate);
			io::parse_bool(toml_window_hint["stereo"], platform_setup.window_hint.window.stereo);
			io::parse_bool(toml_window_hint["srgb_capable"], platform_setup.window_hint.window.srgb_capable);
			io::parse_bool(toml_window_hint["double_buffer"], platform_setup.window_hint.window.double_buffer);
			io::parse_bool(toml_window_hint["opengl_forward_compat"], platform_setup.window_hint.window.opengl_forward_compat);
			io::parse_bool(toml_window_hint["context_debug"], platform_setup.window_hint.window.context_debug);
		}

		platform_setup.num_gamepads = glm::clamp(io::parse_int_or(node["gamepads"], 0), 0, GLFW_JOYSTICK_LAST);
		internal::input_binding_context = std::make_unique<input::internal::InputBindingContext>(platform_setup.num_gamepads);

		internal::platform = platform::internal::create_platform(platform_setup);

		SingletonTickService<TickPhase::None, void, TerminatePhase::Platform, PlatformOnTerminate>::instance();
	}

	void internal::init_viewport(TOMLNode node)
	{
		bool camera_boxed = true;
		bool camera_stretch = true;

		if (auto window = node["window"])
		{
			if (auto viewport = window["viewport"])
			{
				io::parse_bool(viewport["boxed"], camera_boxed);
				io::parse_bool(viewport["stretch"], camera_stretch);
			}
		}
		
		rendering::internal::initialize_default_camera(camera_boxed, camera_stretch);
	}

	bool internal::platform_frame()
	{
		oly::internal::check_errors();
		LOG.flush();
		return internal::platform->frame();
	}

	platform::Platform& get_platform()
	{
		return *internal::platform;
	}

	input::internal::InputBindingContext& input_binding_context()
	{
		return *internal::input_binding_context;
	}

	input::SignalTable& signal_table()
	{
		return internal::signal_table;
	}

	input::SignalMappingTable& signal_mapping_table()
	{
		return internal::signal_mapping_table;
	}

	void assign_signal_mapping(const StringParam& mapping_name, std::vector<std::string>&& signal_names)
	{
		internal::signal_mapping_table[mapping_name.transfer()] = std::move(signal_names);
	}

	void unassign_signal_mapping(const StringParam& mapping_name)
	{
		internal::signal_mapping_table.erase(mapping_name.transfer());
	}

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
				_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Unrecognized swizzle value \"" << swizz << "\"." << LOG.nl;
		}

		if (!io::parse_float(mnode["multiplier"], modifier.multiplier.x))
			if (!io::parse_vec(mnode["multiplier"], reinterpret_cast<glm::vec2&>(modifier.multiplier)))
				io::parse_vec(mnode["multiplier"], modifier.multiplier);

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
				modifier.conversion = input::Axis0DModifier::Conversion::To1D;
			else if (conv == "TO_2D")
				modifier.conversion = input::Axis0DModifier::Conversion::To2D;
			else if (conv == "TO_3D")
				modifier.conversion = input::Axis0DModifier::Conversion::To3D;
			else
				_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Unrecognized conversion value \"" << conv << "\"." << LOG.nl;
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
				modifier.conversion = input::Axis1DModifier::Conversion::To0D;
			else if (conv == "TO_2D")
				modifier.conversion = input::Axis1DModifier::Conversion::To2D;
			else if (conv == "TO_3D")
				modifier.conversion = input::Axis1DModifier::Conversion::To3D;
			else
				_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Unrecognized conversion value \"" << conv << "\"." << LOG.nl;
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
				modifier.conversion = input::Axis2DModifier::Conversion::To0D_X;
			else if (conv == "TO_0D_Y")
				modifier.conversion = input::Axis2DModifier::Conversion::To0D_Y;
			else if (conv == "TO_0D_XY")
				modifier.conversion = input::Axis2DModifier::Conversion::To0D_XY;
			else if (conv == "TO_1D_X")
				modifier.conversion = input::Axis2DModifier::Conversion::To1D_X;
			else if (conv == "TO_1D_Y")
				modifier.conversion = input::Axis2DModifier::Conversion::To1D_Y;
			else if (conv == "TO_1D_XY")
				modifier.conversion = input::Axis2DModifier::Conversion::To1D_XY;
			else if (conv == "TO_3D_0")
				modifier.conversion = input::Axis2DModifier::Conversion::To3D_0;
			else if (conv == "TO_3D_1")
				modifier.conversion = input::Axis2DModifier::Conversion::To3D_1;
			else
				_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Unrecognized conversion value \"" << conv << "\"." << LOG.nl;
		}

		load_modifier_base(modifier, mnode);

		return modifier;
	}

	static void load_key_binding(TOMLNode node, const std::string& id)
	{
		input::KeyBinding b;
		if (!io::parse_int(node["key"], b.key))
			return;
		io::parse_int(node["req_mods"], b.required_key_mods);
		io::parse_int(node["ban_mods"], b.forbidden_key_mods);
		b.modifier = load_modifier_0d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_mouse_button_binding(TOMLNode node, const std::string& id)
	{
		input::MouseButtonBinding b;
		if (!io::parse_int(node["button"], b.button))
			return;
		io::parse_int(node["req_mods"], b.required_button_mods);
		io::parse_int(node["ban_mods"], b.forbidden_button_mods);
		b.modifier = load_modifier_0d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_button_binding(TOMLNode node, const std::string& id)
	{
		int button;
		if (!io::parse_int(node["button"], button))
			return;
		input::GamepadButtonBinding b{ .button = (input::GamepadButton)button };
		b.modifier = load_modifier_0d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_axis_1d_binding(TOMLNode node, const std::string& id)
	{
		int axis1d;
		if (!io::parse_int(node["axis1d"], axis1d))
			return;
		input::GamepadAxis1DBinding b{ .axis = (input::GamepadAxis1D)axis1d };
		io::parse_float(node["deadzone"], b.deadzone);
		b.modifier = load_modifier_1d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_axis_2d_binding(TOMLNode node, const std::string& id)
	{
		int axis2d;
		if (!io::parse_int(node["axis2d"], axis2d))
			return;
		input::GamepadAxis2DBinding b{ .axis = (input::GamepadAxis2D)axis2d };
		io::parse_float(node["deadzone"], b.deadzone);
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
		auto toml_id = node["id"].value<std::string>();
		if (!toml_id)
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Missing \"id\" field." << LOG.endl;
			return;
		}
		auto toml_binding = node["binding"].value<std::string>();
		if (!toml_binding)
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Missing \"binding\" field." << LOG.endl;
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
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Unrecognized binding value \"" << binding << "\"." << LOG.nl;
			return;
		}
	}

	void load_signal(TOMLNode node, const DebugTrace& trace)
	{
		auto scope = trace.scope("CONTEXT", "oly::context::load_signal()");
		load_signal(node);
	}

	void load_signal_mapping(TOMLNode node)
	{
		auto toml_id = node["id"].value<std::string>();
		if (!toml_id)
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Missing \"id\" field." << LOG.endl;
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
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Input signal #" << i << " cannot be parsed as a string." << LOG.nl;
			}
			context::assign_signal_mapping(toml_id.value(), std::move(signals));
		}
	}

	void load_signal_mapping(TOMLNode node, const DebugTrace& trace)
	{
		auto scope = trace.scope("CONTEXT", "oly::context::load_signal_mapping()");
		load_signal_mapping(node);
	}

	// TODO v7 revamp input signal system so that there's only one file, not multiple. Also, instead of a Tester.oly file, could do a whole .oly folder of files to keep things modular and simple for the editor - use meta fields and fixed filenames.

	void load_signals(const ResourcePath& file)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		if (!io::MetaSplitter::meta(file).has_type("signal"))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain signal type." << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto toml = io::load_toml(file);

		if (LOG.enable.debug)
		{
			std::string source = file.get_absolute().generic_string();
			DebugTrace trace(source.c_str());

			auto signals = toml["signal"].as_array();
			if (signals)
				signals->for_each([&trace](auto&& node) { load_signal((TOMLNode)node, trace); });

			auto mappings = toml["mapping"].as_array();
			if (mappings)
				mappings->for_each([&trace](auto&& node) { load_signal_mapping((TOMLNode)node, trace); });
		}
		else
		{
			auto signals = toml["signal"].as_array();
			if (signals)
				signals->for_each([](auto&& node) { load_signal((TOMLNode)node); });

			auto mappings = toml["mapping"].as_array();
			if (mappings)
				mappings->for_each([](auto&& node) { load_signal_mapping((TOMLNode)node); });
		}
	}
}
