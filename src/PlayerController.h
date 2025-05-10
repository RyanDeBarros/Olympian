#pragma once

#include "Olympian.h"

struct PlayerController : public oly::InputController
{
	bool dragging = false;
	glm::vec2 ref_cursor_pos = {};
	glm::vec2 ref_text_pos = {};
	oly::rendering::Paragraph* test_text = nullptr;

	bool jump(oly::input::Signal signal);
	bool click(oly::input::Signal signal);
	bool drag(oly::input::Signal signal);
	bool drag(glm::vec2 cursor_pos);
	glm::vec2 screen_to_world_coords(glm::vec2 coords);
	bool zoom_camera(oly::input::Signal signal);
};
