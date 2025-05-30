#include "PlayerController.h"

bool PlayerController::jump(oly::input::Signal signal)
{
	if (signal.phase == oly::input::Phase::STARTED)
	{
		oly::LOG << "Jump!" << oly::LOG.endl;
		return true;
	}
	return false;
}

bool PlayerController::click(oly::input::Signal signal)
{
	if (signal.phase == oly::input::Phase::STARTED)
	{
		dragging = true;
		double x, y;
		glfwGetCursorPos(oly::get_platform().window(), &x, &y);
		ref_cursor_pos = { (float)x, (float)y };
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
		return drag(signal.get<glm::vec2>());
	return false;
}

bool PlayerController::drag(glm::vec2 cursor_pos)
{
	if (test_text)
	{
		test_text->set_local().position = ref_text_pos + screen_to_world_coords(cursor_pos) - screen_to_world_coords(ref_cursor_pos);
		return true;
	}
	return false;
}

glm::vec2 PlayerController::screen_to_world_coords(glm::vec2 coords)
{
	return { coords.x - 0.5f * oly::get_platform().window().get_width(), 0.5f * oly::get_platform().window().get_height() - coords.y };
}

bool PlayerController::zoom_camera(oly::input::Signal signal)
{
	switch (signal.phase)
	{
	case oly::input::Phase::STARTED:
		oly::LOG << "STARTED:   " << signal.get<glm::vec2>() << oly::LOG.endl;
		return true;
	case oly::input::Phase::ONGOING:
		oly::LOG << "ONGOING:   " << signal.get<glm::vec2>() << oly::LOG.endl;
		return true;
	case oly::input::Phase::COMPLETED:
		oly::LOG << "COMPLETED: " << signal.get<glm::vec2>() << oly::LOG.endl;
		return true;
	}
	return false;
}
