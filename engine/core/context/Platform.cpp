#include "Platform.h"

#include "core/context/Context.h"
#include "core/base/Errors.h"
#include "core/util/Logger.h"
#include "core/algorithms/STLUtils.h"
#include "graphics/Camera.h"

#include "registries/Loader.h"

namespace oly::context
{
	namespace internal
	{
		std::unique_ptr<platform::Platform> platform;
		std::unique_ptr<input::internal::InputBindingContext> input_binding_context;
		input::SignalTable signal_table;
		input::SignalMappingTable signal_mapping_table;
	}

	void internal::init_platform(TOMLNode node)
	{
		platform::PlatformSetup platform_setup;

		auto toml_window = node["window"];
		if (!toml_window)
		{
			OLY_LOG_FATAL(true, "CONTEXT") << LOG.source_info.full_source() << "Cannot initialize platform: missing \"window\" table." << LOG.nl;
			throw Error(ErrorCode::PLATFORM_INIT);
		}

		if (!reg::parse_int(toml_window["width"], platform_setup.window_width))
		{
			OLY_LOG_FATAL(true, "CONTEXT") << LOG.source_info.full_source() << "Cannot initialize platform: missing or invalid \"width\" field." << LOG.nl;
			throw Error(ErrorCode::PLATFORM_INIT);
		}
		
		if (!reg::parse_int(toml_window["height"], platform_setup.window_height))
		{
			OLY_LOG_FATAL(true, "CONTEXT") << LOG.source_info.full_source() << "Cannot initialize platform: missing or invalid \"height\" field." << LOG.nl;
			throw Error(ErrorCode::PLATFORM_INIT);
		}

		if (auto title = toml_window["title"].value<std::string>())
			platform_setup.window_title = *title;
		else
		{
			OLY_LOG_FATAL(true, "CONTEXT") << LOG.source_info.full_source() << "Cannot initialize platform: missing or invalid \"title\" fields." << LOG.nl;
			throw Error(ErrorCode::PLATFORM_INIT);
		}

		if (auto toml_window_hint = toml_window["window_hint"])
		{
			reg::parse_vec(toml_window_hint["clear_color"], platform_setup.window_hint.context.clear_color);
			reg::parse_int(toml_window_hint["swap_interval"], platform_setup.window_hint.context.swap_interval);
			reg::parse_bool(toml_window_hint["resizable"], platform_setup.window_hint.window.resizable);
			reg::parse_bool(toml_window_hint["visible"], platform_setup.window_hint.window.visible);
			reg::parse_bool(toml_window_hint["decorated"], platform_setup.window_hint.window.decorated);
			reg::parse_bool(toml_window_hint["focused"], platform_setup.window_hint.window.focused);
			reg::parse_bool(toml_window_hint["auto_iconify"], platform_setup.window_hint.window.auto_iconify);
			reg::parse_bool(toml_window_hint["floating"], platform_setup.window_hint.window.floating);
			reg::parse_bool(toml_window_hint["maximized"], platform_setup.window_hint.window.maximized);
			reg::parse_bool(toml_window_hint["center_cursor"], platform_setup.window_hint.window.center_cursor);
			reg::parse_bool(toml_window_hint["transparent_framebuffer"], platform_setup.window_hint.window.transparent_framebuffer);
			reg::parse_bool(toml_window_hint["focus_on_show"], platform_setup.window_hint.window.focus_on_show);
			reg::parse_bool(toml_window_hint["scale_to_monitor"], platform_setup.window_hint.window.scale_to_monitor);
			reg::parse_bool(toml_window_hint["scale_framebuffer"], platform_setup.window_hint.window.scale_framebuffer);
			reg::parse_bool(toml_window_hint["mouse_passthrough"], platform_setup.window_hint.window.mouse_passthrough);
			reg::parse_uint(toml_window_hint["position_x"], platform_setup.window_hint.window.position_x);
			reg::parse_uint(toml_window_hint["position_y"], platform_setup.window_hint.window.position_y);
			reg::parse_int(toml_window_hint["refresh_rate"], platform_setup.window_hint.window.refresh_rate);
			reg::parse_bool(toml_window_hint["stereo"], platform_setup.window_hint.window.stereo);
			reg::parse_bool(toml_window_hint["srgb_capable"], platform_setup.window_hint.window.srgb_capable);
			reg::parse_bool(toml_window_hint["double_buffer"], platform_setup.window_hint.window.double_buffer);
			reg::parse_bool(toml_window_hint["opengl_forward_compat"], platform_setup.window_hint.window.opengl_forward_compat);
			reg::parse_bool(toml_window_hint["context_debug"], platform_setup.window_hint.window.context_debug);
		}

		platform_setup.num_gamepads = glm::clamp(reg::parse_int_or(node["gamepads"], 0), 0, GLFW_JOYSTICK_LAST);
		internal::input_binding_context = std::make_unique<input::internal::InputBindingContext>(platform_setup.num_gamepads);

		internal::platform = platform::internal::create_platform(platform_setup);
	}

	void internal::init_viewport(TOMLNode node)
	{
		bool camera_boxed = true;
		bool camera_stretch = true;

		if (auto window = node["window"])
		{
			if (auto viewport = window["viewport"])
			{
				reg::parse_bool(viewport["boxed"], camera_boxed);
				reg::parse_bool(viewport["stretch"], camera_stretch);
			}
		}
		
		rendering::internal::initialize_default_camera(camera_boxed, camera_stretch);
	}

	void internal::terminate_platform()
	{
		internal::platform.reset();
		internal::input_binding_context.reset();
	}

	bool internal::frame_platform()
	{
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

	void assign_signal_mapping(const std::string& mapping_name, std::vector<std::string>&& signal_names)
	{
		internal::signal_mapping_table[mapping_name] = std::move(signal_names);
	}

	void unassign_signal_mapping(const std::string& mapping_name)
	{
		internal::signal_mapping_table.erase(mapping_name);
	}
}
