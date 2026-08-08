#include "ProjectDesc.h"

#include "core/editor/ProjectInfo.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	ViewportDesc::ViewportDesc(imtk::datapath_link link) :
		link(std::move(link)),
		boxed(IMTK_DATAPATH_SUBLINK(subpaths.boxed), true, detail::Key::Boxed, "Boxed"),
		stretch(IMTK_DATAPATH_SUBLINK(subpaths.stretch), true, detail::Key::Stretch, "Stretch")
	{
	}

	WindowHintsDesc::WindowHintsDesc(imtk::datapath_link link) :
		link(std::move(link)),
		context_clear_color(IMTK_DATAPATH_SUBLINK(subpaths.context_clear_color), { 0.f, 0.f, 0.f, 1.f }, detail::Key::ClearColor, "Clear color"),
		context_swap_interval(IMTK_DATAPATH_SUBLINK(subpaths.context_swap_interval), 1, detail::Key::SwapInterval, "Swap interval"),
		window_resizable(IMTK_DATAPATH_SUBLINK(subpaths.window_resizable), true, detail::Key::Resizable, "Resizable"),
		window_visible(IMTK_DATAPATH_SUBLINK(subpaths.window_visible), true, detail::Key::Visible, "Visible"),
		window_decorated(IMTK_DATAPATH_SUBLINK(subpaths.window_decorated), true, detail::Key::Decorated, "Decorated"),
		window_focused(IMTK_DATAPATH_SUBLINK(subpaths.window_focused), true, detail::Key::Focused, "Focused"),
		window_auto_iconify(IMTK_DATAPATH_SUBLINK(subpaths.window_auto_iconify), true, detail::Key::AutoIconify, "Auto-iconify"),
		window_floating(IMTK_DATAPATH_SUBLINK(subpaths.window_floating), false, detail::Key::Floating, "Floating"),
		window_maximized(IMTK_DATAPATH_SUBLINK(subpaths.window_maximized), false, detail::Key::Maximized, "Maximized"),
		window_center_cursor(IMTK_DATAPATH_SUBLINK(subpaths.window_center_cursor), true, detail::Key::CenterCursor, "Center cursor"),
		window_transparent_framebuffer(IMTK_DATAPATH_SUBLINK(subpaths.window_transparent_framebuffer), false, detail::Key::TransparentFramebuffer, "Transparent framebuffer"),
		window_focus_on_show(IMTK_DATAPATH_SUBLINK(subpaths.window_focus_on_show), true, detail::Key::FocusOnShow, "Focus on show"),
		window_scale_to_monitor(IMTK_DATAPATH_SUBLINK(subpaths.window_scale_to_monitor), false, detail::Key::ScaleToMonitor, "Scale to monitor"),
		window_scale_framebuffer(IMTK_DATAPATH_SUBLINK(subpaths.window_scale_framebuffer), true, detail::Key::ScaleFramebuffer, "Scale framebuffer"),
		window_mouse_passthrough(IMTK_DATAPATH_SUBLINK(subpaths.window_mouse_passthrough), false, detail::Key::MousePassthrough, "Mouse passthrough"),
		window_position_x(IMTK_DATAPATH_SUBLINK(subpaths.window_position_x), OptionalPrimitive<int>(false, 0), GLFW_ANY_POSITION, detail::Key::PositionX, "Position (X)"),
		window_position_y(IMTK_DATAPATH_SUBLINK(subpaths.window_position_y), OptionalPrimitive<int>(false, 0), GLFW_ANY_POSITION, detail::Key::PositionY, "Position (Y)"),
		window_refresh_rate(IMTK_DATAPATH_SUBLINK(subpaths.window_refresh_rate), OptionalPrimitive<int>(false, 0), GLFW_DONT_CARE, detail::Key::RefreshRate, "Refresh rate"),
		window_stereo(IMTK_DATAPATH_SUBLINK(subpaths.window_stereo), false, detail::Key::Stereo, "Stereo"),
		window_srgb_capable(IMTK_DATAPATH_SUBLINK(subpaths.window_srgb_capable), false, detail::Key::SrgbCapable, "sRGB Capable"),
		window_opengl_forward_compat(IMTK_DATAPATH_SUBLINK(subpaths.window_opengl_forward_compat), false, detail::Key::OpenglForwardCompat, "OpenGL forward compatible"),
		window_context_debug(IMTK_DATAPATH_SUBLINK(subpaths.window_context_debug), false, detail::Key::ContextDebug, "Context debug")
	{
	}

	const detail::Key WindowDesc::viewport_key = detail::Key::Viewport;
	const detail::Key WindowDesc::window_hints_key = detail::Key::WindowHint;

	WindowDesc::WindowDesc(imtk::datapath_link link) :
		link(std::move(link)),
		width(IMTK_DATAPATH_SUBLINK(subpaths.width), 1440, detail::Key::Width, "Width"),
		height(IMTK_DATAPATH_SUBLINK(subpaths.height), 1080, detail::Key::Height, "Height"),
		title(IMTK_DATAPATH_SUBLINK(subpaths.title), ProjectInfo::Instance().ProjectName(), detail::Key::Title, "Title"),
		viewport(IMTK_DATAPATH_SUBLINK(subpaths.viewport)),
		window_hints(IMTK_DATAPATH_SUBLINK(subpaths.window_hints))
	{
	}

	const detail::Key PlatformDesc::window_key = detail::Key::Window;

	PlatformDesc::PlatformDesc(imtk::datapath_link link) :
		link(std::move(link)),
		window(IMTK_DATAPATH_SUBLINK(subpaths.window)),
		gamepads(IMTK_DATAPATH_SUBLINK(subpaths.gamepads), 1, detail::Key::Gamepads, "# Gamepads")
	{
	}

	CollisionDesc::CollisionDesc(imtk::datapath_link link) :
		link(std::move(link)),
		masks(IMTK_DATAPATH_SUBLINK(subpaths.masks), {}, detail::Key::Masks, "Masks"),
		layers(IMTK_DATAPATH_SUBLINK(subpaths.layers), {}, detail::Key::Layers, "Layers")
	{
	}

	LoggerEnableDesc::LoggerEnableDesc(imtk::datapath_link link) :
		link(std::move(link)),
		debug(IMTK_DATAPATH_SUBLINK(subpaths.debug), false, detail::Key::Debug, "Debug"),
		info(IMTK_DATAPATH_SUBLINK(subpaths.info), true, detail::Key::Info, "Info"),
		warning(IMTK_DATAPATH_SUBLINK(subpaths.warning), true, detail::Key::Warning, "Warning"),
		error(IMTK_DATAPATH_SUBLINK(subpaths.error), true, detail::Key::Error, "Error"),
		fatal(IMTK_DATAPATH_SUBLINK(subpaths.fatal), true, detail::Key::Fatal, "Fatal")
	{
	}

	const detail::Key LoggerDesc::enable_key = detail::Key::Enable;

	LoggerDesc::LoggerDesc(imtk::datapath_link link) :
		link(std::move(link)),
		use_logfile(IMTK_DATAPATH_SUBLINK(subpaths.use_logfile), true, detail::Key::UseLogfile, "Use Logfile"),
		use_console(IMTK_DATAPATH_SUBLINK(subpaths.use_console), true, detail::Key::UseConsole, "Use Console"),
		max_prior_log_files(IMTK_DATAPATH_SUBLINK(subpaths.max_prior_log_files), MakeOpt<int>(), detail::Key::MaxPriorLogFiles, detail::Key::EnableMaxPriorLogFiles, "Max Prior Log Files"),
		max_prior_log_bytes(IMTK_DATAPATH_SUBLINK(subpaths.max_prior_log_bytes), MakeOpt<int>(), detail::Key::MaxPriorLogBytes, detail::Key::EnableMaxPriorLogBytes, "Max Prior Log Bytes"),
		enable(IMTK_DATAPATH_SUBLINK(subpaths.enable))
	{
	}

	FrameRateDesc::FrameRateDesc(imtk::datapath_link link) :
		link(std::move(link)),
		frame_length_clip(IMTK_DATAPATH_SUBLINK(subpaths.frame_length_clip), 0.2, detail::Key::FrameLengthClip, "Frame length clip"),
		time_scale(IMTK_DATAPATH_SUBLINK(subpaths.time_scale), 1.0, detail::Key::TimeScale, "Time scale")
	{
	}

	const detail::Key ContextDesc::platform_key = detail::Key::Platform;
	const detail::Key ContextDesc::collision_key = detail::Key::Collision;
	const detail::Key ContextDesc::logger_key = detail::Key::Logger;
	const detail::Key ContextDesc::frame_rate_key = detail::Key::FrameRate;

	ContextDesc::ContextDesc(imtk::datapath_link link) :
		link(std::move(link)),
		platform(IMTK_DATAPATH_SUBLINK(subpaths.platform)),
		collision(IMTK_DATAPATH_SUBLINK(subpaths.collision)),
		logger(IMTK_DATAPATH_SUBLINK(subpaths.logger)),
		frame_rate(IMTK_DATAPATH_SUBLINK(subpaths.frame_rate))
	{
	}

	const detail::Key ProjectDesc::context_key = detail::Key::Context;

	ProjectDesc::ProjectDesc(imtk::datapath_link link) :
		link(std::move(link)),
		context(IMTK_DATAPATH_SUBLINK(subpaths.context))
	{
	}
}
