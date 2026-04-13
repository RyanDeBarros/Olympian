#include "Platform.h"

#include "core/context/Context.h"
#include "core/base/Errors.h"
#include "core/base/Assert.h"
#include "core/util/Logger.h"
#include "core/util/Loader.h"
#include "core/util/Parse.h"
#include "core/util/MetaSplitter.h"
#include "core/algorithms/STLUtils.h"
#include "graphics/Camera.h"
#include "core/platform/Definitions.h"

#include ".gen/keys/Context.inl"
#include ".gen/keys/Window.inl"
#include ".gen/keys/WindowHint.inl"
#include ".gen/keys/Signal.inl"

#include ".gen/enums/platform/Axis0DConversion.inl"
#include ".gen/enums/platform/Axis1DConversion.inl"
#include ".gen/enums/platform/Axis2DConversion.inl"
#include ".gen/enums/platform/Swizzle.inl"
#include ".gen/enums/platform/SignalBindingType.inl"

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

		auto toml_window = io::parse_key(node, _gen::keys::Context::Window);
		if (!toml_window)
		{
			_OLY_ENGINE_LOG_FATAL("CONTEXT") << "Cannot initialize platform: missing " << io::key_string(_gen::keys::Context::Window) << " table" << LOG.nl;
			throw Error(ErrorCode::PlatformInit);
		}

		if (!io::try_parse(io::parse_key(toml_window, _gen::keys::Window::Width), platform_setup.window_width))
		{
			_OLY_ENGINE_LOG_FATAL("CONTEXT") << "Cannot initialize platform: missing or invalid " << io::key_string(_gen::keys::Window::Width) << " field" << LOG.nl;
			throw Error(ErrorCode::PlatformInit);
		}

		if (!io::try_parse(io::parse_key(toml_window, _gen::keys::Window::Height), platform_setup.window_height))
		{
			_OLY_ENGINE_LOG_FATAL("CONTEXT") << "Cannot initialize platform: missing or invalid " << io::key_string(_gen::keys::Window::Height) << " field" << LOG.nl;
			throw Error(ErrorCode::PlatformInit);
		}

		if (auto title = io::parse_key(toml_window, _gen::keys::Window::Title).value<std::string>())
			platform_setup.window_title = *title;
		else
		{
			_OLY_ENGINE_LOG_FATAL("CONTEXT") << "Cannot initialize platform: missing or invalid " << io::key_string(_gen::keys::Window::Title) << " field" << LOG.nl;
			throw Error(ErrorCode::PlatformInit);
		}

		if (auto toml_window_hint = io::parse_key(toml_window, _gen::keys::Context::WindowHint))
		{
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::ClearColor), platform_setup.window_hint.context.clear_color);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::SwapInterval), platform_setup.window_hint.context.swap_interval);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::Resizable), platform_setup.window_hint.window.resizable);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::Visible), platform_setup.window_hint.window.visible);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::Decorated), platform_setup.window_hint.window.decorated);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::Focused), platform_setup.window_hint.window.focused);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::AutoIconify), platform_setup.window_hint.window.auto_iconify);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::Floating), platform_setup.window_hint.window.floating);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::Maximized), platform_setup.window_hint.window.maximized);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::CenterCursor), platform_setup.window_hint.window.center_cursor);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::TransparentFramebuffer), platform_setup.window_hint.window.transparent_framebuffer);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::FocusOnShow), platform_setup.window_hint.window.focus_on_show);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::ScaleToMonitor), platform_setup.window_hint.window.scale_to_monitor);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::ScaleFramebuffer), platform_setup.window_hint.window.scale_framebuffer);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::MousePassthrough), platform_setup.window_hint.window.mouse_passthrough);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::PositionX), platform_setup.window_hint.window.position_x);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::PositionY), platform_setup.window_hint.window.position_y);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::RefreshRate), platform_setup.window_hint.window.refresh_rate);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::Stereo), platform_setup.window_hint.window.stereo);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::SrgbCapable), platform_setup.window_hint.window.srgb_capable);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::DoubleBuffer), platform_setup.window_hint.window.double_buffer);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::OpenglForwardCompat), platform_setup.window_hint.window.opengl_forward_compat);
			io::try_parse(io::parse_key(toml_window_hint, _gen::keys::WindowHint::ContextDebug), platform_setup.window_hint.window.context_debug);
		}

		platform_setup.num_gamepads = glm::clamp(io::parse_or(io::parse_key(node, _gen::keys::Context::Gamepads), 0), 0, GLFW_JOYSTICK_LAST);
		internal::input_binding_context = std::make_unique<input::internal::InputBindingContext>(platform_setup.num_gamepads);

		internal::platform = platform::internal::create_platform(platform_setup);

		SingletonTickService<TickPhase::None, void, TerminatePhase::Platform, PlatformOnTerminate>::instance();
	}

	void internal::init_viewport(TOMLNode node)
	{
		bool camera_boxed = true;
		bool camera_stretch = true;

		if (auto window = io::parse_key(node, _gen::keys::Context::Window))
		{
			if (auto viewport = io::parse_key(window, _gen::keys::Window::Viewport))
			{
				io::try_parse(io::parse_key(viewport, _gen::keys::Window::Boxed), camera_boxed);
				io::try_parse(io::parse_key(viewport, _gen::keys::Window::Stretch), camera_stretch);
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
		io::try_parse_enum<_gen::platform::Swizzle>(mnode, _gen::keys::Signal::Swizzle, modifier.swizzle);
		io::try_parse_if_exists(mnode, _gen::keys::Signal::Multiplier, io::PartialView(modifier.multiplier));
		io::try_parse_if_exists(mnode, _gen::keys::Signal::Invert, io::PartialView(modifier.invert));
	}

	static input::Axis0DModifier load_modifier_0d(TOMLNode node)
	{
		input::Axis0DModifier modifier;
		TOMLNode mnode = io::parse_key(node, _gen::keys::Signal::Modifier);
		io::try_parse_enum<_gen::platform::Axis0DConversion>(mnode, _gen::keys::Signal::Conversion, modifier.conversion);
		load_modifier_base(modifier, mnode);
		return modifier;
	}

	static input::Axis1DModifier load_modifier_1d(TOMLNode node)
	{
		input::Axis1DModifier modifier;
		TOMLNode mnode = io::parse_key(node, _gen::keys::Signal::Modifier);
		io::try_parse_enum<_gen::platform::Axis1DConversion>(mnode, _gen::keys::Signal::Conversion, modifier.conversion);
		load_modifier_base(modifier, mnode);
		return modifier;
	}

	static input::Axis2DModifier load_modifier_2d(TOMLNode node)
	{
		input::Axis2DModifier modifier;
		TOMLNode mnode = io::parse_key(node, _gen::keys::Signal::Modifier);
		io::try_parse_enum<_gen::platform::Axis2DConversion>(mnode, _gen::keys::Signal::Conversion, modifier.conversion);
		load_modifier_base(modifier, mnode);
		return modifier;
	}

	static void load_key_binding(TOMLNode node, const std::string& id)
	{
		input::KeyBinding b;
		if (!io::try_parse(io::parse_key(node, _gen::keys::Signal::Key), b.key))
			return;
		io::try_parse(io::parse_key(node, _gen::keys::Signal::RequiredMods), b.required_key_mods);
		io::try_parse(io::parse_key(node, _gen::keys::Signal::ForbiddenMods), b.forbidden_key_mods);
		b.modifier = load_modifier_0d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_mouse_button_binding(TOMLNode node, const std::string& id)
	{
		input::MouseButtonBinding b;
		if (!io::try_parse(io::parse_key(node, _gen::keys::Signal::Button), b.button))
			return;
		io::try_parse(io::parse_key(node, _gen::keys::Signal::RequiredMods), b.required_button_mods);
		io::try_parse(io::parse_key(node, _gen::keys::Signal::ForbiddenMods), b.forbidden_button_mods);
		b.modifier = load_modifier_0d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_button_binding(TOMLNode node, const std::string& id)
	{
		int button;
		if (!io::try_parse(io::parse_key(node, _gen::keys::Signal::Button), button))
			return;
		input::GamepadButtonBinding b{ .button = (input::GamepadButton)button };
		b.modifier = load_modifier_0d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_axis_1d_binding(TOMLNode node, const std::string& id)
	{
		int axis1d;
		if (!io::try_parse(io::parse_key(node, _gen::keys::Signal::Axis1D), axis1d))
			return;
		input::GamepadAxis1DBinding b{ .axis = (input::GamepadAxis1D)axis1d };
		io::try_parse(io::parse_key(node, _gen::keys::Signal::Deadzone), b.deadzone);
		b.modifier = load_modifier_1d(node);

		context::input_binding_context().register_signal_binding(context::signal_table().get(id), b);
	}

	static void load_gamepad_axis_2d_binding(TOMLNode node, const std::string& id)
	{
		int axis2d;
		if (!io::try_parse(io::parse_key(node, _gen::keys::Signal::Axis2D), axis2d))
			return;
		input::GamepadAxis2DBinding b{ .axis = (input::GamepadAxis2D)axis2d };
		io::try_parse(io::parse_key(node, _gen::keys::Signal::Deadzone), b.deadzone);
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
		const auto id = io::parse_required<std::string>(node, _gen::keys::Signal::ID);
		switch (io::parse_required_enum<_gen::platform::SignalBindingType>(node, _gen::keys::Signal::Binding))
		{
		case input::SignalBindingType::Key:
			load_key_binding(node, id);
			break;
		case input::SignalBindingType::MouseButton:
			load_mouse_button_binding(node, id);
			break;
		case input::SignalBindingType::GamepadButton:
			load_gamepad_button_binding(node, id);
			break;
		case input::SignalBindingType::GamepadAxis1D:
			load_gamepad_axis_1d_binding(node, id);
			break;
		case input::SignalBindingType::GamepadAxis2D:
			load_gamepad_axis_2d_binding(node, id);
			break;
		case input::SignalBindingType::CursorPos:
			load_cursor_pos_binding(node, id);
			break;
		case input::SignalBindingType::Scroll:
			load_scroll_binding(node, id);
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
		const auto id = io::parse_required<std::string>(node, _gen::keys::Signal::ID);
		if (auto toml_signals = io::parse_key(node, _gen::keys::Signal::MappedSignalList).as_array())
		{
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

		if (!io::MetaSplitter::meta(file).has_type("signal"))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain signal type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto table = io::load_toml(file);
		TOMLNode toml = (TOMLNode)table;

		if (LOG.enable.debug)
		{
			std::string source = file.get_absolute().generic_string();
			DebugTrace trace(source.c_str());

			auto signals = io::parse_key(toml, _gen::keys::Signal::SignalArray).as_array();
			if (signals)
				signals->for_each([&trace](auto&& node) { load_signal((TOMLNode)node, trace); });

			auto mappings = io::parse_key(toml, _gen::keys::Signal::MappingArray).as_array();
			if (mappings)
				mappings->for_each([&trace](auto&& node) { load_signal_mapping((TOMLNode)node, trace); });
		}
		else
		{
			auto signals = io::parse_key(toml, _gen::keys::Signal::SignalArray).as_array();
			if (signals)
				signals->for_each([](auto&& node) { load_signal((TOMLNode)node); });

			auto mappings = io::parse_key(toml, _gen::keys::Signal::MappingArray).as_array();
			if (mappings)
				mappings->for_each([](auto&& node) { load_signal_mapping((TOMLNode)node); });
		}
	}
}
