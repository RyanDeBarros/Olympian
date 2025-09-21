#include "Arrow.h"

#include "core/cmath/Triangulation.h"
#include "core/types/Approximate.h"

namespace oly::rendering
{
	ArrowExtension::ArrowExtension(PolygonBatch* batch)
		: body(batch), head(batch)
	{
		body.transformer.attach_parent(&_transformer);
		body.set_points().resize(4);
		body.set_points()[0] = { 0.0f, -0.5f };
		body.set_points()[1] = { 1.0f, -0.5f };
		body.set_points()[2] = { 1.0f,  0.5f };
		body.set_points()[3] = { 0.0f,  0.5f };
		body.set_colors().resize(4);

		head.transformer.attach_parent(&_transformer);
		head.set_points().resize(3);
		head.set_points()[0] = { 1.0f, -0.5f };
		head.set_points()[1] = { 2.0f,  0.0f };
		head.set_points()[2] = { 1.0f,  0.5f };
		head.set_colors().resize(3);
	}

	void ArrowExtension::draw() const
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
				
				head.set_points()[0] = {   0.0f, -0.5f * head_width };
				head.set_points()[1] = { length,               0.0f };
				head.set_points()[2] = {   0.0f,  0.5f * head_width };

				glm::mat2 rotation_matrix = UnitVector2D(end - start).rotation_matrix();
				for (glm::vec2& point : head.set_points())
					point = start + rotation_matrix * point;

				head.set_colors()[0] = start_color;
				head.set_colors()[1] = end_color;
				head.set_colors()[2] = start_color;
			}
			else
			{
				can_draw_body = true;
				can_draw_head = true;

				body.set_points()[0] = {                 0.0f, -0.5f * width };
				body.set_points()[1] = { length - head_height, -0.5f * width };
				body.set_points()[2] = { length - head_height,  0.5f * width };
				body.set_points()[3] = {                 0.0f,  0.5f * width };

				head.set_points()[0] = { length - head_height, -0.5f * head_width };
				head.set_points()[1] = {               length,               0.0f };
				head.set_points()[2] = { length - head_height,  0.5f * head_width };

				glm::mat2 rotation_matrix = UnitVector2D(end - start).rotation_matrix();
				for (glm::vec2& point : body.set_points())
					point = start + rotation_matrix * point;
				for (glm::vec2& point : head.set_points())
					point = start + rotation_matrix * point;

				body.set_colors()[0] = start_color;
				body.set_colors()[1] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				body.set_colors()[2] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				body.set_colors()[3] = start_color;

				head.set_colors()[0] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				head.set_colors()[1] = end_color;
				head.set_colors()[2] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
			}
		}
		if (can_draw_body)
			body.draw();
		if (can_draw_head)
			head.draw();
	}

	StaticArrowExtension::StaticArrowExtension(PolygonBatch* batch)
		: body(batch), head(batch)
	{
		body.set_points().resize(4);
		body.set_points()[0] = { 0.0f, -0.5f };
		body.set_points()[1] = { 1.0f, -0.5f };
		body.set_points()[2] = { 1.0f,  0.5f };
		body.set_points()[3] = { 0.0f,  0.5f };
		body.set_colors().resize(4);

		head.set_points().resize(3);
		head.set_points()[0] = { 1.0f, -0.5f };
		head.set_points()[1] = { 2.0f,  0.0f };
		head.set_points()[2] = { 1.0f,  0.5f };
		head.set_colors().resize(3);
	}

	void StaticArrowExtension::draw() const
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

				head.set_points()[0] = { 0.0f, -0.5f * head_width };
				head.set_points()[1] = { length,               0.0f };
				head.set_points()[2] = { 0.0f,  0.5f * head_width };

				glm::mat2 rotation_matrix = UnitVector2D(end - start).rotation_matrix();
				for (glm::vec2& point : head.set_points())
					point = start + rotation_matrix * point;

				head.set_colors()[0] = start_color;
				head.set_colors()[1] = end_color;
				head.set_colors()[2] = start_color;
			}
			else
			{
				can_draw_body = true;
				can_draw_head = true;

				body.set_points()[0] = { 0.0f, -0.5f * width };
				body.set_points()[1] = { length - head_height, -0.5f * width };
				body.set_points()[2] = { length - head_height,  0.5f * width };
				body.set_points()[3] = { 0.0f,  0.5f * width };

				head.set_points()[0] = { length - head_height, -0.5f * head_width };
				head.set_points()[1] = { length,               0.0f };
				head.set_points()[2] = { length - head_height,  0.5f * head_width };

				glm::mat2 rotation_matrix = UnitVector2D(end - start).rotation_matrix();
				for (glm::vec2& point : body.set_points())
					point = start + rotation_matrix * point;
				for (glm::vec2& point : head.set_points())
					point = start + rotation_matrix * point;

				body.set_colors()[0] = start_color;
				body.set_colors()[1] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				body.set_colors()[2] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				body.set_colors()[3] = start_color;

				head.set_colors()[0] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
				head.set_colors()[1] = end_color;
				head.set_colors()[2] = glm::mix(start_color, end_color, 1.0f - (head_height / length));
			}
		}
		if (can_draw_body)
			body.draw();
		if (can_draw_head)
			head.draw();
	}
}
