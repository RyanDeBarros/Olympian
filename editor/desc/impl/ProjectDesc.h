#pragma once

#include "desc/Fields.h"

namespace oly::editor
{
#define VIEWPORT_GENERATOR(M) \
	M(boxed) \
	M(stretch)

	struct ViewportDesc
	{
		BoolField boxed;
		BoolField stretch;

		DESCRIPTOR_BODY(ViewportDesc, VIEWPORT_GENERATOR);

		ViewportDesc();
	};

#define WINDOW_HINTS_GENERATOR(M) \
	M(context_clear_color) \
	M(context_swap_interval) \
	M(window_resizable) \
	M(window_visible) \
	M(window_decorated) \
	M(window_focused) \
	M(window_auto_iconify) \
	M(window_floating) \
	M(window_maximized) \
	M(window_center_cursor) \
	M(window_transparent_framebuffer) \
	M(window_focus_on_show) \
	M(window_scale_to_monitor) \
	M(window_scale_framebuffer) \
	M(window_mouse_passthrough) \
	M(window_position_x) \
	M(window_position_y) \
	M(window_refresh_rate) \
	M(window_stereo) \
	M(window_srgb_capable) \
	M(window_opengl_forward_compat) \
	M(window_context_debug)

	struct WindowHintsDesc
	{
		Color4Field context_clear_color;
		IntField<MakeOpt(0), MakeOpt<int>()> context_swap_interval;
		BoolField window_resizable;
		BoolField window_visible;
		BoolField window_decorated;
		BoolField window_focused;
		BoolField window_auto_iconify;
		BoolField window_floating;
		BoolField window_maximized;
		BoolField window_center_cursor;
		BoolField window_transparent_framebuffer;
		BoolField window_focus_on_show;
		BoolField window_scale_to_monitor;
		BoolField window_scale_framebuffer;
		BoolField window_mouse_passthrough;
		CompactOptionalIntField<MakeOpt(0), MakeOpt<int>()> window_position_x;
		CompactOptionalIntField<MakeOpt(0), MakeOpt<int>()> window_position_y;
		CompactOptionalIntField<MakeOpt(0), MakeOpt<int>()> window_refresh_rate;
		BoolField window_stereo;
		BoolField window_srgb_capable;
		BoolField window_opengl_forward_compat;
		BoolField window_context_debug;

		DESCRIPTOR_BODY(WindowHintsDesc, WINDOW_HINTS_GENERATOR);

		WindowHintsDesc();
	};

#define WINDOW_PARTIAL_GENERATOR(M) \
	M(width) \
	M(height) \
	M(title)

#define WINDOW_GENERATOR(M) \
	WINDOW_PARTIAL_GENERATOR(M) \
	M(viewport) \
	M(window_hints)

	struct WindowDesc
	{
		IntField<MakeOpt(1), MakeOpt<int>()> width;
		IntField<MakeOpt(1), MakeOpt<int>()> height;
		StringField title;
		ViewportDesc viewport;
		static const detail::Key viewport_key;
		WindowHintsDesc window_hints;
		static const detail::Key window_hints_key;

		DESCRIPTOR_BODY(WindowDesc, WINDOW_GENERATOR);

		WindowDesc();
	};

#define PLATFORM_PARTIAL_GENERATOR(M) \
	M(gamepads)

#define PLATFORM_GENERATOR(M) \
	M(window) \
	PLATFORM_PARTIAL_GENERATOR(M)

	struct PlatformDesc
	{
		WindowDesc window;
		static const detail::Key window_key;
		IntField<MakeOpt(0), MakeOpt<int>(GLFW_JOYSTICK_LAST)> gamepads;

		DESCRIPTOR_BODY(PlatformDesc, PLATFORM_GENERATOR);

		PlatformDesc();
	};

#define COLLISION_GENERATOR(M) \
	M(masks) \
	M(layers)

	struct CollisionDesc
	{
		StringArrayField<32> masks;
		StringArrayField<32> layers;

		DESCRIPTOR_BODY(CollisionDesc, COLLISION_GENERATOR);

		CollisionDesc();
	};

#define LOGGER_ENABLE_GENERATOR(M) \
	M(debug) \
	M(info) \
	M(warning) \
	M(error) \
	M(fatal)

	struct LoggerEnableDesc
	{
		BoolField debug;
		BoolField info;
		BoolField warning;
		BoolField error;
		BoolField fatal;

		DESCRIPTOR_BODY(LoggerEnableDesc, LOGGER_ENABLE_GENERATOR);

		LoggerEnableDesc();
	};

#define LOGGER_PARTIAL_GENERATOR(M) \
	M(use_logfile) \
	M(use_console) \
	M(max_prior_log_files) \
	M(max_prior_log_bytes)

#define LOGGER_GENERATOR(M) \
	LOGGER_PARTIAL_GENERATOR(M) \
	M(enable)

	struct LoggerDesc
	{
		BoolField use_logfile;
		BoolField use_console;
		OptionalIntField<MakeOpt(0), MakeOpt<int>()> max_prior_log_files;
		OptionalIntField<MakeOpt(0), MakeOpt<int>()> max_prior_log_bytes;

		LoggerEnableDesc enable;
		static const detail::Key enable_key;

		DESCRIPTOR_BODY(LoggerDesc, LOGGER_GENERATOR);

		LoggerDesc();
	};

#define FRAME_RATE_GENERATOR(M) \
	M(frame_length_clip) \
	M(time_scale)

	struct FrameRateDesc
	{
		DoubleField<MakeOpt(0.0), MakeOpt<double>()> frame_length_clip;
		DoubleField<MakeOpt(0.0), MakeOpt<double>()> time_scale;

		DESCRIPTOR_BODY(FrameRateDesc, FRAME_RATE_GENERATOR);

		FrameRateDesc();
	};

#define CONTEXT_GENERATOR(M) \
	M(platform) \
	M(collision) \
	M(logger) \
	M(frame_rate)

	struct ContextDesc
	{
		PlatformDesc platform;
		static const detail::Key platform_key;
		CollisionDesc collision;
		static const detail::Key collision_key;
		LoggerDesc logger;
		static const detail::Key logger_key;
		FrameRateDesc frame_rate;
		static const detail::Key frame_rate_key;

		DESCRIPTOR_BODY(ContextDesc, CONTEXT_GENERATOR);
	};

#define PROJECT_GENERATOR(M) \
	M(context)

	struct ProjectDesc
	{
		ContextDesc context;
		static const detail::Key context_key;

		DESCRIPTOR_BODY(ProjectDesc, PROJECT_GENERATOR);
	};
}
