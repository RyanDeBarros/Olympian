#pragma once

#include "core/platform/Window.h"
#include "core/types/Functor.h"

namespace oly::platform
{
	// TODO v5 rename or move to different file - could just be the root window resize handler in Window
	struct WRDrawer : public EventHandler<input::WindowResizeEventData>
	{
		float resizing_frame_length = 1.0f / 60.0f;

	private:
		float last_update = 0.0f;

	public:
		bool block(const input::WindowResizeEventData& data) override;
		bool consume(const input::WindowResizeEventData& data) override;
	};
}
