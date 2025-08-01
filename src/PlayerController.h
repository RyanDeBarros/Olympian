#pragma once

#include "Olympian.h"

#include "physics/dynamics/bodies/KinematicBody.h"

struct PlayerController : public oly::InputController
{
	OLY_INPUT_CONTROLLER_HEADER(PlayerController);

public:
	bool dragging = false;
	glm::vec2 ref_cursor_pos = {};
	glm::vec2 ref_text_pos = {};
	oly::rendering::ParagraphRef test_text = nullptr;
	oly::physics::KinematicBodyRef rigid_body = nullptr;

	bool jump(oly::input::Signal signal);
	bool click(oly::input::Signal signal);
	bool drag(oly::input::Signal signal);
	bool drag(glm::vec2 cursor_pos);
	glm::vec2 screen_to_world_coords(glm::vec2 coords);
	bool zoom_camera(oly::input::Signal signal);
	bool move(oly::input::Signal signal);
};
