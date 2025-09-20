#pragma once

#include "graphics/shapes/Polygons.h"

namespace oly::rendering
{
	// ASSET
	class ArrowExtension
	{
		mutable Polygon body, head;
		Transformer2D _transformer;
		glm::vec2 start = {}, end = {};
		float head_width = 3.0f, head_height = 3.0f;
		glm::vec4 start_color = glm::vec4(1.0f), end_color = glm::vec4(1.0f);
		float width = 1.0f;
		mutable bool dirty = false;
		mutable bool can_draw_body = false, can_draw_head = false;

	public:
		ArrowExtension() = default;
		ArrowExtension(PolygonBatch* batch);

		PolygonBatch* get_batch() const { return body.get_batch(); }
		void set_batch(PolygonBatch* batch) { body.set_batch(batch); head.set_batch(batch); }

		void draw() const;

		glm::vec2 get_start() const { return start; }
		glm::vec2& set_start() { dirty = true; return start; }
		glm::vec2 get_end() const { return end; }
		glm::vec2& set_end() { dirty = true; return end; }
		float get_width() const { return width; }
		void set_width(float w) { if (w > 0.0f && w != width) { dirty = true; width = w; } }
		float get_head_width() const { return head_width; }
		void set_head_width(float w) { if (w > 0.0f && w != head_width) { dirty = true; head_width = w; } }
		float get_head_height() const { return head_height; }
		void set_head_height(float h) { if (h > 0.0f && h != head_height) { dirty = true; head_height = h; } }

		glm::vec4 get_start_color() const { return start_color; }
		glm::vec4& set_start_color() { dirty = true; return start_color; }
		glm::vec4 get_end_color() const { return end_color; }
		glm::vec4& set_end_color() { dirty = true; return end_color; }
		void set_color(glm::vec4 color) { set_start_color() = set_end_color() = color; }

		const Transformer2D& transformer() const { return _transformer; }
		Transformer2D& transformer() { return _transformer; }
		const Transform2D& get_local() const { return _transformer.get_local(); }
		Transform2D& set_local() { return _transformer.set_local(); }

		void scale_head(float by) { set_head_width(head_width * by); set_head_height(head_height * by); }
		void adjust_standard_head_for_width(float w)
		{
			if (w > 0.0f)
			{
				dirty = true;
				width = w;
				head_width = 3.0f * w;
				head_height = 3.0f * w;
			}
		}
	};

	// ASSET
	class StaticArrowExtension
	{
		mutable StaticPolygon body, head;
		glm::vec2 start = {}, end = {};
		float head_width = 3.0f, head_height = 3.0f;
		glm::vec4 start_color = glm::vec4(1.0f), end_color = glm::vec4(1.0f);
		float width = 1.0f;
		mutable bool dirty = false;
		mutable bool can_draw_body = false, can_draw_head = false;

	public:
		StaticArrowExtension() = default;
		StaticArrowExtension(PolygonBatch* batch);

		PolygonBatch* get_batch() const { return body.get_batch(); }
		void set_batch(PolygonBatch* batch) { body.set_batch(batch); head.set_batch(batch); }

		void draw() const;

		glm::vec2 get_start() const { return start; }
		glm::vec2& set_start() { dirty = true; return start; }
		glm::vec2 get_end() const { return end; }
		glm::vec2& set_end() { dirty = true; return end; }
		float get_width() const { return width; }
		void set_width(float w) { if (w > 0.0f && w != width) { dirty = true; width = w; } }
		float get_head_width() const { return head_width; }
		void set_head_width(float w) { if (w > 0.0f && w != head_width) { dirty = true; head_width = w; } }
		float get_head_height() const { return head_height; }
		void set_head_height(float h) { if (h > 0.0f && h != head_height) { dirty = true; head_height = h; } }

		glm::vec4 get_start_color() const { return start_color; }
		glm::vec4& set_start_color() { dirty = true; return start_color; }
		glm::vec4 get_end_color() const { return end_color; }
		glm::vec4& set_end_color() { dirty = true; return end_color; }
		void set_color(glm::vec4 color) { set_start_color() = set_end_color() = color; }

		void scale_head(float by) { set_head_width(head_width * by); set_head_height(head_height * by); }
		void adjust_standard_head_for_width(float w)
		{
			if (w > 0.0f)
			{
				dirty = true;
				width = w;
				head_width = 3.0f * w;
				head_height = 3.0f * w;
			}
		}
	};
}
