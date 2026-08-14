#pragma once

#include "desc/Fields.h"

namespace oly::editor
{
#define VIEWPORT_GENERATOR(M) \
	M((BoolField), boxed) \
	M((BoolField), stretch)

	struct ViewportDesc
	{
		IMTK_DESCRIPTOR_BODY(ViewportDesc, VIEWPORT_GENERATOR);

		ViewportDesc(imtk::datapath_link link = {});
	};

#define WINDOW_HINTS_GENERATOR(M) \
	M((Color4Field), context_clear_color) \
	M((IntField<MakeOpt(0), MakeOpt<int>()>), context_swap_interval) \
	M((BoolField), window_resizable) \
	M((BoolField), window_visible) \
	M((BoolField), window_decorated) \
	M((BoolField), window_focused) \
	M((BoolField), window_auto_iconify) \
	M((BoolField), window_floating) \
	M((BoolField), window_maximized) \
	M((BoolField), window_center_cursor) \
	M((BoolField), window_transparent_framebuffer) \
	M((BoolField), window_focus_on_show) \
	M((BoolField), window_scale_to_monitor) \
	M((BoolField), window_scale_framebuffer) \
	M((BoolField), window_mouse_passthrough) \
	M((CompactOptionalIntField<MakeOpt(0), MakeOpt<int>()>), window_position_x) \
	M((CompactOptionalIntField<MakeOpt(0), MakeOpt<int>()>), window_position_y) \
	M((CompactOptionalIntField<MakeOpt(0), MakeOpt<int>()>), window_refresh_rate) \
	M((BoolField), window_stereo) \
	M((BoolField), window_srgb_capable) \
	M((BoolField), window_opengl_forward_compat) \
	M((BoolField), window_context_debug)

	struct WindowHintsDesc
	{
		IMTK_DESCRIPTOR_BODY(WindowHintsDesc, WINDOW_HINTS_GENERATOR);

		WindowHintsDesc(imtk::datapath_link link = {});
	};

#define WINDOW_PARTIAL_GENERATOR(M) \
	M((IntField<MakeOpt(1), MakeOpt<int>()>), width) \
	M((IntField<MakeOpt(1), MakeOpt<int>()>), height) \
	M((StringField), title)

#define WINDOW_GENERATOR(M) \
	WINDOW_PARTIAL_GENERATOR(M) \
	M((ViewportDesc), viewport) \
	M((WindowHintsDesc), window_hints)

	struct WindowDesc
	{
		IMTK_DESCRIPTOR_BODY(WindowDesc, WINDOW_GENERATOR);

		static const detail::Key viewport_key;
		static const detail::Key window_hints_key;

		WindowDesc(imtk::datapath_link link = {});
	};

#define PLATFORM_PARTIAL_GENERATOR(M) \
	M((IntField<MakeOpt(0), MakeOpt<int>(GLFW_JOYSTICK_LAST)>), gamepads)

#define PLATFORM_GENERATOR(M) \
	M((WindowDesc), window) \
	PLATFORM_PARTIAL_GENERATOR(M)

	struct PlatformDesc
	{
		IMTK_DESCRIPTOR_BODY(PlatformDesc, PLATFORM_GENERATOR);

		static const detail::Key window_key;

		PlatformDesc(imtk::datapath_link link = {});
	};

#define COLLISION_GENERATOR(M) \
	M((StringArrayField<32>), masks) \
	M((StringArrayField<32>), layers)

	struct CollisionDesc
	{
		IMTK_DESCRIPTOR_BODY(CollisionDesc, COLLISION_GENERATOR);

		CollisionDesc(imtk::datapath_link link = {});
	};

#define LOGGER_ENABLE_GENERATOR(M) \
	M((BoolField), debug) \
	M((BoolField), info) \
	M((BoolField), warning) \
	M((BoolField), error) \
	M((BoolField), fatal)

	struct LoggerEnableDesc
	{
		IMTK_DESCRIPTOR_BODY(LoggerEnableDesc, LOGGER_ENABLE_GENERATOR);

		LoggerEnableDesc(imtk::datapath_link link = {});
	};

#define LOGGER_PARTIAL_GENERATOR(M) \
	M((BoolField), use_logfile) \
	M((BoolField), use_console) \
	M((OptionalIntField<MakeOpt(0), MakeOpt<int>()>), max_prior_log_files) \
	M((OptionalIntField<MakeOpt(0), MakeOpt<int>()>), max_prior_log_bytes)

#define LOGGER_GENERATOR(M) \
	LOGGER_PARTIAL_GENERATOR(M) \
	M((LoggerEnableDesc), enable)

	struct LoggerDesc
	{
		IMTK_DESCRIPTOR_BODY(LoggerDesc, LOGGER_GENERATOR);

		static const detail::Key enable_key;

		LoggerDesc(imtk::datapath_link link = {});
	};

#define FRAME_RATE_GENERATOR(M) \
	M((DoubleField<MakeOpt(0.0), MakeOpt<double>()>), frame_length_clip) \
	M((DoubleField<MakeOpt(0.0), MakeOpt<double>()>), time_scale)

	struct FrameRateDesc
	{
		IMTK_DESCRIPTOR_BODY(FrameRateDesc, FRAME_RATE_GENERATOR);
		
		FrameRateDesc(imtk::datapath_link link = {});
	};

#define CONTEXT_GENERATOR(M) \
	M((PlatformDesc), platform) \
	M((CollisionDesc), collision) \
	M((LoggerDesc), logger) \
	M((FrameRateDesc), frame_rate)

	struct ContextDesc
	{
		IMTK_DESCRIPTOR_BODY(ContextDesc, CONTEXT_GENERATOR);

		static const detail::Key platform_key;
		static const detail::Key collision_key;
		static const detail::Key logger_key;
		static const detail::Key frame_rate_key;

		ContextDesc(imtk::datapath_link link = {});
	};

#define PROJECT_GENERATOR(M) \
	M((ContextDesc), context)

	struct ProjectDesc
	{
		IMTK_DESCRIPTOR_BODY(ProjectDesc, PROJECT_GENERATOR);

		static const detail::Key context_key;

		ProjectDesc(imtk::datapath_link link = {});
	};
}
