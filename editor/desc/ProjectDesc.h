#pragma once

#include "desc/Fields.h"

namespace oly::editor
{
	struct ViewportDesc
	{
		BoolField boxed;
		BoolField stretch;

		ViewportDesc();

		void Reset(ViewportDesc& source);
		void Isolate();
	};

#define VIEWPORT_GENERATOR(M) \
	M(boxed) \
	M(stretch)

	struct WindowHintsDesc
	{
		ColorField context_clear_color;
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

		WindowHintsDesc();

		void Reset(WindowHintsDesc& source);
		void Isolate();
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

	struct WindowDesc
	{
		IntField<MakeOpt(1), MakeOpt<int>()> width;
		IntField<MakeOpt(1), MakeOpt<int>()> height;
		StringField title;
		ViewportDesc viewport;
		static const detail::Key viewport_key;
		WindowHintsDesc window_hints;
		static const detail::Key window_hints_key;

		WindowDesc();

		void Reset(WindowDesc& source);
		void Isolate();
	};

#define WINDOW_PARTIAL_GENERATOR(M) \
	M(width) \
	M(height) \
	M(title)

#define WINDOW_GENERATOR(M) \
	WINDOW_PARTIAL_GENERATOR(M) \
	M(viewport) \
	M(window_hints)

	struct PlatformDesc
	{
		WindowDesc window;
		static const detail::Key window_key;
		IntField<MakeOpt(0), MakeOpt<int>(GLFW_JOYSTICK_LAST)> gamepads;

		PlatformDesc();

		void Reset(PlatformDesc& source);
		void Isolate();
	};

#define PLATFORM_PARTIAL_GENERATOR(M) \
	M(gamepads)

#define PLATFORM_GENERATOR(M) \
	M(window) \
	PLATFORM_PARTIAL_GENERATOR(M)

	struct CollisionDesc
	{
		StringArrayField<32> masks;
		StringArrayField<32> layers;

		CollisionDesc();

		void Reset(CollisionDesc& source);
		void Isolate();
	};

#define COLLISION_GENERATOR(M) \
	M(masks) \
	M(layers)

	struct LoggerEnableDesc
	{
		BoolField debug;
		BoolField info;
		BoolField warning;
		BoolField error;
		BoolField fatal;

		LoggerEnableDesc();

		void Reset(LoggerEnableDesc& source);
		void Isolate();
	};

#define LOGGER_ENABLE_GENERATOR(M) \
	M(debug) \
	M(info) \
	M(warning) \
	M(error) \
	M(fatal)

	struct LoggerDesc
	{
		BoolField use_logfile;
		BoolField use_console;
		OptionalIntField<MakeOpt(0), MakeOpt<int>()> max_prior_log_files;
		OptionalIntField<MakeOpt(0), MakeOpt<int>()> max_prior_log_bytes;

		LoggerEnableDesc enable;
		static const detail::Key enable_key;

		LoggerDesc();

		void Reset(LoggerDesc& source);
		void Isolate();
	};

#define LOGGER_PARTIAL_GENERATOR(M) \
	M(use_logfile) \
	M(use_console) \
	M(max_prior_log_files) \
	M(max_prior_log_bytes)

#define LOGGER_GENERATOR(M) \
	LOGGER_PARTIAL_GENERATOR(M) \
	M(enable)

	struct ContextDesc
	{
		PlatformDesc platform;
		static const detail::Key platform_key;
		CollisionDesc collision;
		static const detail::Key collision_key;
		LoggerDesc logger;
		static const detail::Key logger_key;

		void Reset(ContextDesc& source);
		void Isolate();
	};

#define CONTEXT_GENERATOR(M) \
	M(platform) \
	M(collision) \
	M(logger)

	struct ProjectDesc
	{
		ContextDesc context;
		static const detail::Key context_key;

		void Reset(ProjectDesc& source);
		void Isolate();
	};

#define PROJECT_GENERATOR(M) \
	M(context)
}
