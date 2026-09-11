#pragma once

#include "bindings/Fields.h"

namespace oly::editor
{
#define VIEWPORT_GENERATOR(M) \
	M((imtk::field::bool_fld), boxed) \
	M((imtk::field::bool_fld), stretch)

	struct ViewportDesc
	{
		IMTK_DESCRIPTOR_BODY(ViewportDesc, VIEWPORT_GENERATOR);

		ViewportDesc(imtk::datapath_link link = {});
	};

#define WINDOW_HINTS_GENERATOR(M) \
	M((imtk::field::color4_fld), context_clear_color) \
	M((imtk::field::int_fld<0, imp::nullpotential>), context_swap_interval) \
	M((imtk::field::bool_fld), window_resizable) \
	M((imtk::field::bool_fld), window_visible) \
	M((imtk::field::bool_fld), window_decorated) \
	M((imtk::field::bool_fld), window_focused) \
	M((imtk::field::bool_fld), window_auto_iconify) \
	M((imtk::field::bool_fld), window_floating) \
	M((imtk::field::bool_fld), window_maximized) \
	M((imtk::field::bool_fld), window_center_cursor) \
	M((imtk::field::bool_fld), window_transparent_framebuffer) \
	M((imtk::field::bool_fld), window_focus_on_show) \
	M((imtk::field::bool_fld), window_scale_to_monitor) \
	M((imtk::field::bool_fld), window_scale_framebuffer) \
	M((imtk::field::bool_fld), window_mouse_passthrough) \
	M((imtk::field::compact_optional_int_fld<0, imp::nullpotential>), window_position_x) \
	M((imtk::field::compact_optional_int_fld<0, imp::nullpotential>), window_position_y) \
	M((imtk::field::compact_optional_int_fld<0, imp::nullpotential>), window_refresh_rate) \
	M((imtk::field::bool_fld), window_stereo) \
	M((imtk::field::bool_fld), window_srgb_capable) \
	M((imtk::field::bool_fld), window_opengl_forward_compat) \
	M((imtk::field::bool_fld), window_context_debug)

	struct WindowHintsDesc
	{
		IMTK_DESCRIPTOR_BODY(WindowHintsDesc, WINDOW_HINTS_GENERATOR);

		WindowHintsDesc(imtk::datapath_link link = {});
	};

#define WINDOW_PARTIAL_GENERATOR(M) \
	M((imtk::field::int_fld<1, imp::nullpotential>), width) \
	M((imtk::field::int_fld<1, imp::nullpotential>), height) \
	M((imtk::field::string_fld), title)

#define WINDOW_GENERATOR(M) \
	WINDOW_PARTIAL_GENERATOR(M) \
	M((imtk::desc::sub<ViewportDesc>), viewport) \
	M((imtk::desc::sub<WindowHintsDesc>), window_hints)

	struct WindowDesc
	{
		IMTK_DESCRIPTOR_BODY(WindowDesc, WINDOW_GENERATOR);

		WindowDesc(imtk::datapath_link link = {});
	};

#define PLATFORM_PARTIAL_GENERATOR(M) \
	M((imtk::field::int_fld<0, GLFW_JOYSTICK_LAST>), gamepads)

#define PLATFORM_GENERATOR(M) \
	M((imtk::desc::sub<WindowDesc>), window) \
	PLATFORM_PARTIAL_GENERATOR(M)

	struct PlatformDesc
	{
		IMTK_DESCRIPTOR_BODY(PlatformDesc, PLATFORM_GENERATOR);

		PlatformDesc(imtk::datapath_link link = {});
	};

#define COLLISION_GENERATOR(M) \
	M((imtk::field::array_fld<imtk::field::string_fld, 32>), masks) \
	M((imtk::field::array_fld<imtk::field::string_fld, 32>), layers)

	struct CollisionDesc
	{
		IMTK_DESCRIPTOR_BODY(CollisionDesc, COLLISION_GENERATOR);

		CollisionDesc(imtk::datapath_link link = {});
	};

#define LOGGER_ENABLE_GENERATOR(M) \
	M((imtk::field::bool_fld), debug) \
	M((imtk::field::bool_fld), info) \
	M((imtk::field::bool_fld), warning) \
	M((imtk::field::bool_fld), error) \
	M((imtk::field::bool_fld), fatal)

	struct LoggerEnableDesc
	{
		IMTK_DESCRIPTOR_BODY(LoggerEnableDesc, LOGGER_ENABLE_GENERATOR);

		LoggerEnableDesc(imtk::datapath_link link = {});
	};

#define LOGGER_PARTIAL_GENERATOR(M) \
	M((imtk::field::bool_fld), use_logfile) \
	M((imtk::field::bool_fld), use_console) \
	M((imtk::field::optional_int_fld<0, imp::nullpotential>), max_prior_log_files) \
	M((imtk::field::optional_int_fld<0, imp::nullpotential>), max_prior_log_bytes)

#define LOGGER_GENERATOR(M) \
	LOGGER_PARTIAL_GENERATOR(M) \
	M((imtk::desc::sub<LoggerEnableDesc>), enable)

	struct LoggerDesc
	{
		IMTK_DESCRIPTOR_BODY(LoggerDesc, LOGGER_GENERATOR);

		LoggerDesc(imtk::datapath_link link = {});
	};

#define FRAME_RATE_GENERATOR(M) \
	M((imtk::field::double_fld<0.0, imp::nullpotential>), frame_length_clip) \
	M((imtk::field::double_fld<0.0, imp::nullpotential>), time_scale)

	struct FrameRateDesc
	{
		IMTK_DESCRIPTOR_BODY(FrameRateDesc, FRAME_RATE_GENERATOR);
		
		FrameRateDesc(imtk::datapath_link link = {});
	};

#define CONTEXT_GENERATOR(M) \
	M((imtk::desc::sub<PlatformDesc>), platform) \
	M((imtk::desc::sub<CollisionDesc>), collision) \
	M((imtk::desc::sub<LoggerDesc>), logger) \
	M((imtk::desc::sub<FrameRateDesc>), frame_rate)

	struct ContextDesc
	{
		IMTK_DESCRIPTOR_BODY(ContextDesc, CONTEXT_GENERATOR);

		ContextDesc(imtk::datapath_link link = {});
	};

#define PROJECT_GENERATOR(M) \
	M((imtk::desc::sub<ContextDesc>), context)

	struct ProjectDesc
	{
		IMTK_DESCRIPTOR_BODY(ProjectDesc, PROJECT_GENERATOR);

		ProjectDesc(imtk::datapath_link link = {});
	};
}
