#pragma once

#include "graphics/primitives/Polygons.h"

namespace oly::rendering
{
	class LineExtension
	{
		mutable Polygon poly;
		glm::vec2 start = {}, end = {};
		glm::vec4 start_color = {}, end_color = {};
		float width = 1.0f;
		mutable bool dirty = false;
		mutable bool can_draw = true;

	public:
		LineExtension();

		void draw(BatchBarrier barrier = batch::BARRIER) const;

		glm::vec2 get_start() const { return start; }
		glm::vec2& set_start() { dirty = true; return start; }
		glm::vec2 get_end() const { return end; }
		glm::vec2& set_end() { dirty = true; return end; }
		float get_width() const { return width; }
		void set_width(float w) { if (w > 0.0f && w != width) { dirty = true; width = w; } }
		
		glm::vec4 get_start_color() const { return start_color; }
		glm::vec4& set_start_color() { dirty = true; return start_color; }
		glm::vec4 get_end_color() const { return end_color; }
		glm::vec4& set_end_color() { dirty = true; return end_color; }
		void set_color(glm::vec4 color) { set_start_color() = set_end_color() = color; }

		const Transformer2D& transformer() const { return poly.transformer; }
		Transformer2D& transformer() { return poly.transformer; }
		const Transform2D& get_local() const { return poly.get_local(); }
		Transform2D& set_local() { return poly.set_local(); }

	private:
		void set_default_polygon() const;
	};
}
