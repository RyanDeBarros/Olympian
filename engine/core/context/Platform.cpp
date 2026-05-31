#include "Platform.h"

#include "core/context/Context.h"
#include "core/base/Errors.h"
#include "core/base/Assert.h"
#include "core/util/Logger.h"
#include "core/util/Loader.h"
#include "core/util/Parser.h"
#include "core/algorithms/STLUtils.h"
#include "graphics/Camera.h"
#include "core/platform/Definitions.h"

#include "detail/assets/MetaSplitter.h"

#include "detail/definitions/Keys.h"

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
		assets::Parser top_parser(node, { "(platform init)" }, ErrorCode::PlatformInit, true);

		platform::PlatformSetup platform_setup;

		auto toml_window = top_parser.required<TOMLNode>(detail::Key::Window)();
		assets::Parser window_parser(toml_window);

		window_parser.required(detail::Key::Width)(platform_setup.window_width);
		window_parser.required(detail::Key::Height)(platform_setup.window_height);
		window_parser.required(detail::Key::Title)(platform_setup.window_title);

		if (auto toml_window_hint = window_parser.required<TOMLNode>(detail::Key::WindowHint)())
		{
			assets::Parser parser(toml_window_hint, { "(platform init)" });
			parser.optional(detail::Key::ClearColor)(platform_setup.window_hint.context.clear_color);
			parser.optional(detail::Key::SwapInterval)(platform_setup.window_hint.context.swap_interval);
			parser.optional(detail::Key::Resizable)(platform_setup.window_hint.window.resizable);
			parser.optional(detail::Key::Visible)(platform_setup.window_hint.window.visible);
			parser.optional(detail::Key::Decorated)(platform_setup.window_hint.window.decorated);
			parser.optional(detail::Key::Focused)(platform_setup.window_hint.window.focused);
			parser.optional(detail::Key::AutoIconify)(platform_setup.window_hint.window.auto_iconify);
			parser.optional(detail::Key::Floating)(platform_setup.window_hint.window.floating);
			parser.optional(detail::Key::Maximized)(platform_setup.window_hint.window.maximized);
			parser.optional(detail::Key::CenterCursor)(platform_setup.window_hint.window.center_cursor);
			parser.optional(detail::Key::TransparentFramebuffer)(platform_setup.window_hint.window.transparent_framebuffer);
			parser.optional(detail::Key::FocusOnShow)(platform_setup.window_hint.window.focus_on_show);
			parser.optional(detail::Key::ScaleToMonitor)(platform_setup.window_hint.window.scale_to_monitor);
			parser.optional(detail::Key::ScaleFramebuffer)(platform_setup.window_hint.window.scale_framebuffer);
			parser.optional(detail::Key::MousePassthrough)(platform_setup.window_hint.window.mouse_passthrough);
			parser.optional(detail::Key::PositionX)(platform_setup.window_hint.window.position_x);
			parser.optional(detail::Key::PositionY)(platform_setup.window_hint.window.position_y);
			parser.optional(detail::Key::RefreshRate)(platform_setup.window_hint.window.refresh_rate);
			parser.optional(detail::Key::Stereo)(platform_setup.window_hint.window.stereo);
			parser.optional(detail::Key::SrgbCapable)(platform_setup.window_hint.window.srgb_capable);
			parser.optional(detail::Key::DoubleBuffer)(platform_setup.window_hint.window.double_buffer);
			parser.optional(detail::Key::OpenglForwardCompat)(platform_setup.window_hint.window.opengl_forward_compat);
			parser.optional(detail::Key::ContextDebug)(platform_setup.window_hint.window.context_debug);
		}

		platform_setup.num_gamepads = glm::clamp(top_parser.defaulted(detail::Key::Gamepads)(0u), 0u, (unsigned int)GLFW_JOYSTICK_LAST);
		internal::input_binding_context = std::make_unique<input::internal::InputBindingContext>(platform_setup.num_gamepads);

		internal::platform = platform::internal::create_platform(platform_setup);

		SingletonTickService<TickPhase::None, void, TerminatePhase::Platform, PlatformOnTerminate>::instance();
	}

	void internal::init_viewport(TOMLNode node)
	{
		bool camera_boxed = true;
		bool camera_stretch = true;

		if (auto window_parser = assets::Parser(node).optional(detail::Key::Window).subparser())
		{
			if (auto viewport_parser = window_parser->optional(detail::Key::Viewport).subparser())
			{
				viewport_parser->optional(detail::Key::Boxed)(camera_boxed);
				viewport_parser->optional(detail::Key::Stretch)(camera_stretch);
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

	static void load_modifier_base(input::ModifierBase& modifier, const assets::Parser& parser)
	{
		parser.optional(detail::Key::Swizzle)(modifier.swizzle);
		parser.optional(detail::Key::Multiplier)(assets::PartialView(modifier.multiplier));
		parser.optional(detail::Key::Invert)(assets::PartialView(modifier.invert));
	}

	static input::Axis0DModifier load_modifier_0d(const assets::Parser& parser)
	{
		input::Axis0DModifier modifier;
		if (auto mod_parser = parser.optional(detail::Key::Modifier).subparser())
		{
			mod_parser->optional(detail::Key::Conversion)(modifier.conversion);
			load_modifier_base(modifier, *mod_parser);
		}
		return modifier;
	}

	static input::Axis1DModifier load_modifier_1d(const assets::Parser& parser)
	{
		input::Axis1DModifier modifier;
		if (auto mod_parser = parser.optional(detail::Key::Modifier).subparser())
		{
			mod_parser->optional(detail::Key::Conversion)(modifier.conversion);
			load_modifier_base(modifier, *mod_parser);
		}
		return modifier;
	}

	static input::Axis2DModifier load_modifier_2d(const assets::Parser& parser)
	{
		input::Axis2DModifier modifier;
		if (auto mod_parser = parser.optional(detail::Key::Modifier).subparser())
		{
			mod_parser->optional(detail::Key::Conversion)(modifier.conversion);
			load_modifier_base(modifier, *mod_parser);
		}
		return modifier;
	}

	static void load_key_binding(const assets::Parser& parser, const std::string& id)
	{
		input::KeyBinding b;
		if (!parser.optional(detail::Key::Key)(b.key))
			return;
		parser.optional(detail::Key::RequiredMods)(b.required_key_mods);
		parser.optional(detail::Key::ForbiddenMods)(b.forbidden_key_mods);
		b.modifier = load_modifier_0d(parser);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_mouse_button_binding(const assets::Parser& parser, const std::string& id)
	{
		input::MouseButtonBinding b;
		if (!parser.optional(detail::Key::Button)(b.button))
			return;
		parser.optional(detail::Key::RequiredMods)(b.required_button_mods);
		parser.optional(detail::Key::ForbiddenMods)(b.forbidden_button_mods);
		b.modifier = load_modifier_0d(parser);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_button_binding(const assets::Parser& parser, const std::string& id)
	{
		int button;
		if (!parser.optional(detail::Key::Button)(button))
			return;
		input::GamepadButtonBinding b{ .button = (input::GamepadButton)button };
		b.modifier = load_modifier_0d(parser);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_axis_1d_binding(const assets::Parser& parser, const std::string& id)
	{
		int axis1d;
		if (!parser.optional(detail::Key::Axis1D)(axis1d))
			return;
		input::GamepadAxis1DBinding b{ .axis = (input::GamepadAxis1D)axis1d };
		parser.optional(detail::Key::Deadzone)(b.deadzone);
		b.modifier = load_modifier_1d(parser);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_axis_2d_binding(const assets::Parser& parser, const std::string& id)
	{
		int axis2d;
		if (!parser.optional(detail::Key::Axis2D)(axis2d))
			return;
		input::GamepadAxis2DBinding b{ .axis = (input::GamepadAxis2D)axis2d };
		parser.optional(detail::Key::Deadzone)(b.deadzone);
		b.modifier = load_modifier_2d(parser);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_cursor_pos_binding(const assets::Parser& parser, const std::string& id)
	{
		input::CursorPosBinding b{};
		b.modifier = load_modifier_2d(parser);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_scroll_binding(const assets::Parser& parser, const std::string& id)
	{
		input::ScrollBinding b{};
		b.modifier = load_modifier_2d(parser);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	void load_signal(TOMLNode node)
	{
		assets::Parser parser(node);
		const auto id = parser.required<std::string>(detail::Key::ID)();
		switch (parser.required<input::SignalBindingType>(detail::Key::Binding)())
		{
		case input::SignalBindingType::Key:
			load_key_binding(parser, id);
			break;
		case input::SignalBindingType::MouseButton:
			load_mouse_button_binding(parser, id);
			break;
		case input::SignalBindingType::GamepadButton:
			load_gamepad_button_binding(parser, id);
			break;
		case input::SignalBindingType::GamepadAxis1D:
			load_gamepad_axis_1d_binding(parser, id);
			break;
		case input::SignalBindingType::GamepadAxis2D:
			load_gamepad_axis_2d_binding(parser, id);
			break;
		case input::SignalBindingType::CursorPos:
			load_cursor_pos_binding(parser, id);
			break;
		case input::SignalBindingType::Scroll:
			load_scroll_binding(parser, id);
			break;
		}
	}

	void load_signal(TOMLNode node, const DebugTrace& trace)
	{
		auto scope = trace.scope("CONTEXT", "oly::context::load_signal()");
		load_signal(node);
	}

	void load_signal_mapping(TOMLNode node)
	{
		assets::Parser parser(node);
		const auto id = parser.required<std::string>(detail::Key::ID)();
		auto toml_signals = parser.required<TOMLArray>(detail::Key::MappedSignalList)();
		std::vector<std::string> signals;
		for (size_t i = 0; i < toml_signals->size(); ++i)
		{
			if (auto signal = toml_signals->get_as<std::string>(i))
				signals.push_back(signal->get());
			else
				_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Input signal #" << i << " cannot be parsed as a string" << LOG.nl;
		}
		context::assign_signal_mapping(id, std::move(signals));
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
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		if (!detail::MetaSplitter::meta(file.get_absolute()).has_type("signal"))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain signal type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto toml = io::load_toml(file);
		assets::Parser parser(toml);

		if (LOG.enable.debug)
		{
			std::string source = file.get_absolute().generic_string();
			DebugTrace trace(source.c_str());

			if (auto signals = parser.optional<TOMLArray>(detail::Key::SignalArray)())
				signals->for_each([&trace](auto&& node) { load_signal((TOMLNode)node, trace); });

			if (auto mappings = parser.optional<TOMLArray>(detail::Key::MappingArray)())
				mappings->for_each([&trace](auto&& node) { load_signal_mapping((TOMLNode)node, trace); });
		}
		else
		{
			if (auto signals = parser.optional<TOMLArray>(detail::Key::SignalArray)())
				signals->for_each([](auto&& node) { load_signal((TOMLNode)node); });

			if (auto mappings = parser.optional<TOMLArray>(detail::Key::MappingArray)())
				mappings->for_each([](auto&& node) { load_signal_mapping((TOMLNode)node); });
		}
	}
}
