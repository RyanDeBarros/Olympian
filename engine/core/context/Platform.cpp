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

		glm::ivec2 initial_window_size;

		std::unique_ptr<input::internal::InputBindingContext> input_binding_context;
		input::SignalTable signal_table;
		input::SignalMappingTable signal_mapping_table;
	}

	void internal::init_platform(const TOMLNode& node)
	{
		platform::PlatformSetup platform_setup;

		auto toml_window = node["window"];
		if (!toml_window.as_table())
		{
			OLY_LOG_FATAL(true, "CONTEXT") << LOG.source_info.full_source() << "Cannot initialize platform: missing \"window\" table." << LOG.nl;
			throw Error(ErrorCode::PLATFORM_INIT);
		}
		auto width = toml_window["width"].value<int64_t>();
		auto height = toml_window["height"].value<int64_t>();
		auto title = toml_window["title"].value<std::string>();
		if (!width || !height || !title)
		{
			OLY_LOG_FATAL(true, "CONTEXT") << LOG.source_info.full_source() << "Cannot initialize platform: missing or invalid \"width\", \"height\", and/or \"title\" fields." << LOG.nl;
			throw Error(ErrorCode::PLATFORM_INIT);
		}

		platform_setup.window_width = (int)width.value();
		platform_setup.window_height = (int)height.value();
		platform_setup.window_title = title.value();

		if (auto toml_window_hint = toml_window["window_hint"])
		{
			reg::parse_vec(toml_window_hint["clear_color"].as_array(), platform_setup.window_hint.context.clear_color);
			platform_setup.window_hint.context.swap_interval = (int)toml_window_hint["swap_interval"].value<int64_t>().value_or(1);
			platform_setup.window_hint.window.resizable = toml_window_hint["resizable"].value<bool>().value_or(true);
			platform_setup.window_hint.window.visible = toml_window_hint["visible"].value<bool>().value_or(true);
			platform_setup.window_hint.window.decorated = toml_window_hint["decorated"].value<bool>().value_or(true);
			platform_setup.window_hint.window.focused = toml_window_hint["focused"].value<bool>().value_or(true);
			platform_setup.window_hint.window.auto_iconify = toml_window_hint["auto_iconify"].value<bool>().value_or(true);
			platform_setup.window_hint.window.floating = toml_window_hint["floating"].value<bool>().value_or(false);
			platform_setup.window_hint.window.maximized = toml_window_hint["maximized"].value<bool>().value_or(false);
			platform_setup.window_hint.window.center_cursor = toml_window_hint["center_cursor"].value<bool>().value_or(true);
			platform_setup.window_hint.window.transparent_framebuffer = toml_window_hint["transparent_framebuffer"].value<bool>().value_or(false);
			platform_setup.window_hint.window.focus_on_show = toml_window_hint["focus_on_show"].value<bool>().value_or(true);
			platform_setup.window_hint.window.scale_to_monitor = toml_window_hint["scale_to_monitor"].value<bool>().value_or(false);
			platform_setup.window_hint.window.scale_framebuffer = toml_window_hint["scale_framebuffer"].value<bool>().value_or(true);
			platform_setup.window_hint.window.mouse_passthrough = toml_window_hint["mouse_passthrough"].value<bool>().value_or(false);
			platform_setup.window_hint.window.position_x = (unsigned int)toml_window_hint["position_x"].value<int64_t>().value_or(GLFW_ANY_POSITION);
			platform_setup.window_hint.window.position_y = (unsigned int)toml_window_hint["position_y"].value<int64_t>().value_or(GLFW_ANY_POSITION);
			platform_setup.window_hint.window.refresh_rate = (int)toml_window_hint["refresh_rate"].value<int64_t>().value_or(GLFW_DONT_CARE);
			platform_setup.window_hint.window.stereo = toml_window_hint["stereo"].value<bool>().value_or(false);
			platform_setup.window_hint.window.srgb_capable = toml_window_hint["srgb_capable"].value<bool>().value_or(false);
			platform_setup.window_hint.window.double_buffer = toml_window_hint["double_buffer"].value<bool>().value_or(true);
			platform_setup.window_hint.window.opengl_forward_compat = toml_window_hint["opengl_forward_compat"].value<bool>().value_or(false);
			platform_setup.window_hint.window.context_debug = toml_window_hint["context_debug"].value<bool>().value_or(false);
		}

		platform_setup.num_gamepads = glm::clamp((int)node["gamepads"].value<int64_t>().value_or(0), 0, GLFW_JOYSTICK_LAST);
		internal::input_binding_context = std::make_unique<input::internal::InputBindingContext>(platform_setup.num_gamepads);

		internal::initial_window_size = platform_setup.window_size();
		internal::platform = platform::internal::create_platform(platform_setup);
	}

	void internal::init_viewport(const TOMLNode& node)
	{
		rendering::Camera2DRef default_camera = REF_DEFAULT;

		bool camera_boxed = true;
		bool camera_stretch = true;

		if (auto window = node["window"])
		{
			// TODO v5 use "camera" or "default_camera" instead of "viewport"
			if (auto viewport = window["viewport"])
			{
				camera_boxed = viewport["boxed"].value_or<bool>(true);
				camera_stretch = viewport["stretch"].value_or<bool>(true);
			}
		}
		
		rendering::internal::initialize(*default_camera, camera_boxed, camera_stretch);
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

	glm::vec2 get_cursor_screen_pos()
	{
		double x, y;
		glfwGetCursorPos(internal::platform->window(), &x, &y);
		return { (float)x - 0.5f * internal::platform->window().get_width(), 0.5f * internal::platform->window().get_height() - (float)y };
	}

	glm::vec2 get_initial_window_size()
	{
		return internal::initial_window_size;
	}

	glm::vec2 get_view_stretch()
	{
		rendering::Camera2DRef default_camera = REF_DEFAULT;
		if (default_camera->is_stretch())
		{
			auto v = default_camera->get_viewport();
			return glm::vec2(v.w, v.h) / glm::vec2(internal::initial_window_size);
		}
		else
			return { 1.0f, 1.0f };
	}

	glm::vec2 get_cursor_view_pos()
	{
		return get_cursor_screen_pos() / get_view_stretch();
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
