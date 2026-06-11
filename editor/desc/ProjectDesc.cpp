#include "ProjectDesc.h"

#include "core/editor/ProjectInfo.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	ViewportDesc::ViewportDesc() :
		boxed(true, detail::Key::Boxed, "Boxed"),
		stretch(true, detail::Key::Stretch, "Stretch")
	{
	}

	WindowHintsDesc::WindowHintsDesc() :
		context_clear_color({ 0.f, 0.f, 0.f, 1.f }, detail::Key::ClearColor, "Clear color"),
		context_swap_interval(1, detail::Key::SwapInterval, "Swap interval"),
		window_resizable(true, detail::Key::Resizable, "Resizable"),
		window_visible(true, detail::Key::Visible, "Visible"),
		window_decorated(true, detail::Key::Decorated, "Decorated"),
		window_focused(true, detail::Key::Focused, "Focused"),
		window_auto_iconify(true, detail::Key::AutoIconify, "Auto-iconify"),
		window_floating(false, detail::Key::Floating, "Floating"),
		window_maximized(false, detail::Key::Maximized, "Maximized"),
		window_center_cursor(true, detail::Key::CenterCursor, "Center cursor"),
		window_transparent_framebuffer(false, detail::Key::TransparentFramebuffer, "Transparent framebuffer"),
		window_focus_on_show(true, detail::Key::FocusOnShow, "Focus on show"),
		window_scale_to_monitor(false, detail::Key::ScaleToMonitor, "Scale to monitor"),
		window_scale_framebuffer(true, detail::Key::ScaleFramebuffer, "Scale framebuffer"),
		window_mouse_passthrough(false, detail::Key::MousePassthrough, "Mouse passthrough"),
		window_position_x(MakeOpt<int>(), GLFW_ANY_POSITION, detail::Key::PositionX, "Position (X)"),
		window_position_y(MakeOpt<int>(), GLFW_ANY_POSITION, detail::Key::PositionY, "Position (Y)"),
		window_refresh_rate(MakeOpt<int>(), GLFW_DONT_CARE, detail::Key::RefreshRate, "Refresh rate"),
		window_stereo(false, detail::Key::Stereo, "Stereo"),
		window_srgb_capable(false, detail::Key::SrgbCapable, "sRGB Capable"),
		window_opengl_forward_compat(false, detail::Key::OpenglForwardCompat, "OpenGL forward compatible"),
		window_context_debug(false, detail::Key::ContextDebug, "Context debug")
	{
	}

	const detail::Key WindowDesc::viewport_key = detail::Key::Viewport;
	const detail::Key WindowDesc::window_hints_key = detail::Key::WindowHint;

	WindowDesc::WindowDesc() :
		width(1440, detail::Key::Width, "Width"),
		height(1080, detail::Key::Height, "Height"),
		title(ProjectInfo::Instance().ProjectName(), detail::Key::Title, "Title")
	{
	}

	const detail::Key PlatformDesc::window_key = detail::Key::Window;

	PlatformDesc::PlatformDesc() :
		window(),
		gamepads(1, detail::Key::Gamepads, "# Gamepads")
	{
	}

	CollisionDesc::CollisionDesc() :
		masks({}, detail::Key::Masks, "Masks"),
		layers({}, detail::Key::Layers, "Layers")
	{
	}

	LoggerEnableDesc::LoggerEnableDesc() :
		debug(false, detail::Key::Debug, "Debug"),
		info(true, detail::Key::Info, "Info"),
		warning(true, detail::Key::Warning, "Warning"),
		error(true, detail::Key::Error, "Error"),
		fatal(true, detail::Key::Fatal, "Fatal")
	{
	}

	const detail::Key LoggerDesc::enable_key = detail::Key::Enable;

	LoggerDesc::LoggerDesc() :
		use_logfile(true, detail::Key::UseLogfile, "Use Logfile"),
		use_console(true, detail::Key::UseConsole, "Use Console"),
		max_prior_log_files(MakeOpt<int>(), detail::Key::MaxPriorLogFiles, detail::Key::EnableMaxPriorLogFiles, "Max Prior Log Files"),
		max_prior_log_bytes(MakeOpt<int>(), detail::Key::MaxPriorLogBytes, detail::Key::EnableMaxPriorLogBytes, "Max Prior Log Bytes"),
		enable()
	{
	}

	FrameRateDesc::FrameRateDesc() :
		frame_length_clip(0.2, detail::Key::FrameLengthClip, "Frame length clip"),
		time_scale(1.0, detail::Key::TimeScale, "Time scale")
	{
	}

	const detail::Key ContextDesc::platform_key = detail::Key::Platform;
	const detail::Key ContextDesc::collision_key = detail::Key::Collision;
	const detail::Key ContextDesc::logger_key = detail::Key::Logger;
	const detail::Key ContextDesc::frame_rate_key = detail::Key::FrameRate;

	const detail::Key ProjectDesc::context_key = detail::Key::Context;
}
