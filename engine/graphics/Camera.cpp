#include "Camera.h"

#include "core/context/Platform.h"

namespace oly::rendering
{
	Camera2D::WindowResizeHandler::WindowResizeHandler()
	{
		attach(&context::get_wr_drawer());
		set_projection();
	}

	bool Camera2D::WindowResizeHandler::block(const input::WindowResizeEventData& data)
	{
		if (!context::get_wr_viewport().stretch)
			set_projection();
		return false;
	}

	void Camera2D::WindowResizeHandler::set_projection()
	{
		projection = context::get_wr_viewport().get_projection(); // TODO v5 camera should replace wr_viewport
	}
}
