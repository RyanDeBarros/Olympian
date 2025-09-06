#include "Arrow.h"

#include "core/cmath/Triangulation.h"
#include "core/types/Approximate.h"

namespace oly::rendering
{
	ArrowExtension::ArrowExtension()
	{
		body.transformer.attach_parent(&_transformer);
		body.polygon.points.resize(4);
		body.polygon.points[0] = { 0.0f, -0.5f };
		body.polygon.points[1] = { 1.0f, -0.5f };
		body.polygon.points[2] = { 1.0f,  0.5f };
		body.polygon.points[3] = { 0.0f,  0.5f };
		body.polygon.colors.resize(4);
		body.init();

		head.polygon.points.resize(3);
		head.polygon.points[0] = { 1.0f, -0.5f };
		head.polygon.points[1] = { 2.0f,  0.0f };
		head.polygon.points[2] = { 1.0f,  0.5f };
		head.polygon.colors.resize(3);
		head.transformer.attach_parent(&_transformer);
		head.init();
	}

	void ArrowExtension::draw(BatchBarrier barrier) const
	{
		if (dirty)
		{
			dirty = false;

			float length = glm::length(start - end);
			if (near_zero(length))
			{
				can_draw_body = false;
				can_draw_head = false;
			}
			else if (length <= head_height)
			{
				can_draw_body = false;
				can_draw_head = true;
				
				head.polygon.points[0] = {   0.0f, -0.5f * head_width };
				head.polygon.points[1] = { length,               0.0f };
				head.polygon.points[2] = {   0.0f,  0.5f * head_width };

				glm::mat2 rotation_matrix = UnitVector2D(end - start).rotation_matrix();
				for (glm::vec2& point : head.polygon.points)
					point = start + rotation_matrix * point;

				head.polygon.colors[0] = start_color;
				head.polygon.colors[1] = end_color;
				head.polygon.colors[2] = start_color;

				head.send_polygon();
			}
			else
			{
				can_draw_body = true;
				can_draw_head = true;

				body.polygon.points[0] = {                 0.0f, -0.5f * width };
				body.polygon.points[1] = { length - head_height, -0.5f * width };
				body.polygon.points[2] = { length - head_height,  0.5f * width };
				body.polygon.points[3] = {                 0.0f,  0.5f * width };

				head.polygon.points[0] = { length - head_height, -0.5f * head_width };
				head.polygon.points[1] = {               length,               0.0f };
				head.polygon.points[2] = { length - head_height,  0.5f * head_width };

				glm::mat2 rotation_matrix = UnitVector2D(end - start).rotation_matrix();
				for (glm::vec2& point : body.polygon.points)
					point = start + rotation_matrix * point;
				for (glm::vec2& point : head.polygon.points)
					point = start + rotation_matrix * point;

				body.polygon.colors[0] = start_color;
				body.polygon.colors[1] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				body.polygon.colors[2] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				body.polygon.colors[3] = start_color;

				head.polygon.colors[0] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				head.polygon.colors[1] = end_color;
				head.polygon.colors[2] = glm::mix(start_color, end_color, 1.0f - (head_height / length));

				body.send_polygon();
				head.send_polygon();
			}
		}
		if (can_draw_body)
			body.draw(barrier);
		if (can_draw_head)
			head.draw(barrier);
	}

	StaticArrowExtension::StaticArrowExtension()
	{
		body.polygon.points.resize(4);
		body.polygon.points[0] = { 0.0f, -0.5f };
		body.polygon.points[1] = { 1.0f, -0.5f };
		body.polygon.points[2] = { 1.0f,  0.5f };
		body.polygon.points[3] = { 0.0f,  0.5f };
		body.polygon.colors.resize(4);
		body.init();

		head.polygon.points.resize(3);
		head.polygon.points[0] = { 1.0f, -0.5f };
		head.polygon.points[1] = { 2.0f,  0.0f };
		head.polygon.points[2] = { 1.0f,  0.5f };
		head.polygon.colors.resize(3);
		head.init();
	}

	void StaticArrowExtension::draw(BatchBarrier barrier) const
	{
		if (dirty)
		{
			dirty = false;

			float length = glm::length(start - end);
			if (near_zero(length))
			{
				can_draw_body = false;
				can_draw_head = false;
			}
			else if (length <= head_height)
			{
				can_draw_body = false;
				can_draw_head = true;

				head.polygon.points[0] = { 0.0f, -0.5f * head_width };
				head.polygon.points[1] = { length,               0.0f };
				head.polygon.points[2] = { 0.0f,  0.5f * head_width };

				glm::mat2 rotation_matrix = UnitVector2D(end - start).rotation_matrix();
				for (glm::vec2& point : head.polygon.points)
					point = start + rotation_matrix * point;

				head.polygon.colors[0] = start_color;
				head.polygon.colors[1] = end_color;
				head.polygon.colors[2] = start_color;

				head.send_polygon();
			}
			else
			{
				can_draw_body = true;
				can_draw_head = true;

				body.polygon.points[0] = { 0.0f, -0.5f * width };
				body.polygon.points[1] = { length - head_height, -0.5f * width };
				body.polygon.points[2] = { length - head_height,  0.5f * width };
				body.polygon.points[3] = { 0.0f,  0.5f * width };

				head.polygon.points[0] = { length - head_height, -0.5f * head_width };
				head.polygon.points[1] = { length,               0.0f };
				head.polygon.points[2] = { length - head_height,  0.5f * head_width };

				glm::mat2 rotation_matrix = UnitVector2D(end - start).rotation_matrix();
				for (glm::vec2& point : body.polygon.points)
					point = start + rotation_matrix * point;
				for (glm::vec2& point : head.polygon.points)
					point = start + rotation_matrix * point;

				body.polygon.colors[0] = start_color;
				body.polygon.colors[1] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				body.polygon.colors[2] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				body.polygon.colors[3] = start_color;

				head.polygon.colors[0] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				head.polygon.colors[1] = end_color;
				head.polygon.colors[2] = glm::mix(start_color, end_color, 1.0f - (head_height / length));

				body.send_polygon();
				head.send_polygon();
			}
		}
		if (can_draw_body)
			body.draw(barrier);
		if (can_draw_head)
			head.draw(barrier);
	}
}
