#include "PlayerController.h"

bool PlayerController::jump(oly::input::Signal signal)
{
	if (signal.phase == oly::input::Phase::STARTED)
	{
		OLY_LOG(true) << "Jump!" << oly::LOG.endl;
		return true;
	}
	return false;
}

bool PlayerController::click(oly::input::Signal signal)
{
	if (signal.phase == oly::input::Phase::STARTED)
	{
		dragging = true;
		ref_cursor_pos = oly::default_camera().get_cursor_world_position();
		if (test_text)
			ref_text_pos = test_text->get_local().position;
		drag(ref_cursor_pos);
		return true;
	}
	else if (signal.phase == oly::input::Phase::COMPLETED)
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
	case oly::input::Phase::STARTED:
		OLY_LOG_INFO() << "STARTED:   " << signal.get<glm::vec2>() << oly::LOG.endl;
		return true;
	case oly::input::Phase::ONGOING:
		OLY_LOG_INFO() << "ONGOING:   " << signal.get<glm::vec2>() << oly::LOG.endl;
		return true;
	case oly::input::Phase::COMPLETED:
		OLY_LOG_INFO() << "COMPLETED: " << signal.get<glm::vec2>() << oly::LOG.endl;
		return true;
	}
	return false;
}

bool PlayerController::move(oly::input::Signal signal)
{
	if (signal.phase == oly::input::Phase::STARTED)
	{
		rigid_body->properties().net_linear_impulse += rigid_body->properties().mass() * 50.0f * signal.get<glm::vec2>();
		return true;
	}
	return false;
}
