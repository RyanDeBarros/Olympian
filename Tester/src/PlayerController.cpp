#include "PlayerController.h"

PlayerController::PlayerController()
{
	load_signals("@/signals/Camera.oly");
	load_signals("@/signals/Click.oly");
	load_signals("@/signals/Drag.oly");
	load_signals("@/signals/Jump.oly");
	load_signals("@/signals/Move.oly");

	bind("jump", &PlayerController::jump);
	bind("click", &PlayerController::click);
	bind("drag", &PlayerController::drag);
	bind("zoom camera", &PlayerController::zoom_camera);
	bind("move", &PlayerController::move);
}

bool PlayerController::jump(oly::input::Signal signal)
{
	if (signal.phase == oly::input::Phase::Started)
	{
		OLY_LOG(true) << "Jump!" << oly::LOG.endl;
		return true;
	}
	return false;
}

bool PlayerController::click(oly::input::Signal signal)
{
	if (signal.phase == oly::input::Phase::Started)
	{
		dragging = true;
		ref_cursor_pos = oly::default_camera().get_cursor_world_position();
		if (test_text)
			ref_text_pos = test_text->get_local().position;
		drag(ref_cursor_pos);
		return true;
	}
	else if (signal.phase == oly::input::Phase::Completed)
	{
		dragging = false;
		return true;
	}
	return false;
}

bool PlayerController::drag(oly::input::Signal signal)
{
	if (dragging)
		return drag(oly::default_camera().screen_to_world_coordinates(signal.get<glm::vec2>()));
	return false;
}

bool PlayerController::drag(glm::vec2 view_pos)
{
	if (test_text)
	{
		test_text->set_local().position = ref_text_pos + view_pos - ref_cursor_pos;
		return true;
	}
	return false;
}

bool PlayerController::zoom_camera(oly::input::Signal signal)
{
	switch (signal.phase)
	{
	case oly::input::Phase::Started:
		OLY_LOG_INFO() << "Started:   " << signal.get<glm::vec2>() << oly::LOG.endl;
		return true;
	case oly::input::Phase::Ongoing:
		OLY_LOG_INFO() << "Ongoing:   " << signal.get<glm::vec2>() << oly::LOG.endl;
		return true;
	case oly::input::Phase::Completed:
		OLY_LOG_INFO() << "Completed: " << signal.get<glm::vec2>() << oly::LOG.endl;
		return true;
	}
	return false;
}

bool PlayerController::move(oly::input::Signal signal)
{
	if (signal.phase == oly::input::Phase::Started)
	{
		rigid_body->properties().net_linear_impulse += rigid_body->properties().mass() * 50.0f * signal.get<glm::vec2>();
		return true;
	}
	return false;
}
