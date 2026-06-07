#include "ProjectDesc.h"

#include "core/ProjectInfo.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	ViewportDesc::ViewportDesc() :
		boxed(true, detail::Key::Boxed, "Boxed"),
		stretch(true, detail::Key::Stretch, "Stretch")
	{
	}

	void ViewportDesc::Reset(ViewportDesc& source)
	{
		RESET_FIELDS(VIEWPORT_GENERATOR);
	}
	
	void ViewportDesc::Isolate()
	{
		ISOLATE_FIELDS(VIEWPORT_GENERATOR);
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

	void WindowHintsDesc::Reset(WindowHintsDesc& source)
	{
		RESET_FIELDS(WINDOW_HINTS_GENERATOR);
	}
	
	void WindowHintsDesc::Isolate()
	{
		ISOLATE_FIELDS(WINDOW_HINTS_GENERATOR);
	}

	const detail::Key WindowDesc::viewport_key = detail::Key::Viewport;
	const detail::Key WindowDesc::window_hints_key = detail::Key::WindowHint;

	WindowDesc::WindowDesc() :
		width(1440, detail::Key::Width, "Width"),
		height(1080, detail::Key::Height, "Height"),
		title(ProjectInfo::Instance().ProjectName(), detail::Key::Title, "Title")
	{
	}

	void WindowDesc::Reset(WindowDesc& source)
	{
		RESET_FIELDS(WINDOW_GENERATOR);
	}
	
	void WindowDesc::Isolate()
	{
		ISOLATE_FIELDS(WINDOW_GENERATOR);
	}

	const detail::Key PlatformDesc::window_key = detail::Key::Window;

	PlatformDesc::PlatformDesc() :
		window(),
		gamepads(1, detail::Key::Gamepads, "# Gamepads")
	{
	}

	void PlatformDesc::Reset(PlatformDesc& source)
	{
		RESET_FIELDS(PLATFORM_GENERATOR);
	}

	void PlatformDesc::Isolate()
	{
		ISOLATE_FIELDS(PLATFORM_GENERATOR);
	}

	LoggerEnableDesc::LoggerEnableDesc() :
		debug(false, detail::Key::Debug, "Debug"),
		info(true, detail::Key::Info, "Info"),
		warning(true, detail::Key::Warning, "Warning"),
		error(true, detail::Key::Error, "Error"),
		fatal(true, detail::Key::Fatal, "Fatal")
	{
	}

	void LoggerEnableDesc::Reset(LoggerEnableDesc& source)
	{
		RESET_FIELDS(LOGGER_ENABLE_GENERATOR);
	}

	void LoggerEnableDesc::Isolate()
	{
		ISOLATE_FIELDS(LOGGER_ENABLE_GENERATOR);
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

	void LoggerDesc::Reset(LoggerDesc& source)
	{
		RESET_FIELDS(LOGGER_GENERATOR);
	}

	void LoggerDesc::Isolate()
	{
		ISOLATE_FIELDS(LOGGER_GENERATOR);
	}

	const detail::Key ContextDesc::platform_key = detail::Key::Platform;
	const detail::Key ContextDesc::logger_key = detail::Key::Logger;

	void ContextDesc::Reset(ContextDesc& source)
	{
		RESET_FIELDS(CONTEXT_GENERATOR);
	}

	void ContextDesc::Isolate()
	{
		ISOLATE_FIELDS(CONTEXT_GENERATOR);
	}

	const detail::Key ProjectDesc::context_key = detail::Key::Context;

	void ProjectDesc::Reset(ProjectDesc& source)
	{
		RESET_FIELDS(PROJECT_GENERATOR);
	}

	void ProjectDesc::Isolate()
	{
		ISOLATE_FIELDS(PROJECT_GENERATOR);
	}
}
