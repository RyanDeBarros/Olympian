#pragma once

#include "graphics/shapes/Polygons.h"

namespace oly::rendering
{
	// ASSET
	class ArrowExtension
	{
        mutable Polygon body_;
        mutable Polygon head_;
		Transformer2D transformer_;
        glm::vec2 start_ = {};
        glm::vec2 end_ = {};
        float head_width_ = 3.0f;
        float head_height_ = 3.0f;
        glm::vec4 start_color_ = glm::vec4(1.0f);
        glm::vec4 end_color_ = glm::vec4(1.0f);
		float width_ = 1.0f;
		mutable bool dirty_ = false;
        mutable bool can_draw_body_ = false;
        mutable bool can_draw_head_ = false;

	public:
		ArrowExtension(Unbatched = kUnbatched);
		ArrowExtension(PolygonBatch& batch);

    public:
        internal::PolygonReference::accessor get_batch() const;
        void set_batch(Unbatched);
        void set_batch(PolygonBatch& batch);

        void set_camera_invariant(bool camera_invariant) const;
        bool is_camera_invariant() const;

		void draw() const;

        glm::vec2 get_start() const;
        glm::vec2& set_start();
        glm::vec2 get_end() const;
        glm::vec2& set_end();
        float get_width() const;
        void set_width(float w);
        float get_head_width() const;
        void set_head_width(float w);
        float get_head_height() const;
        void set_head_height(float h);

        glm::vec4 get_start_color() const;
        glm::vec4& set_start_color();
        glm::vec4 get_end_color() const;
        glm::vec4& set_end_color();
        void set_color(glm::vec4 color);

        const Transformer2D& transformer() const;
        Transformer2D& transformer();
        const Transform2D& get_local() const;
        Transform2D& set_local();

        void scale_head(float by);
        void adjust_standard_head_for_width(float w);
	};

	// ASSET
	class StaticArrowExtension
	{
        mutable StaticPolygon body_;
        mutable StaticPolygon head_;
        glm::vec2 start_ = {};
        glm::vec2 end_ = {};
        float head_width_ = 3.0f;
        float head_height_ = 3.0f;
        glm::vec4 start_color_ = glm::vec4(1.0f);
        glm::vec4 end_color_ = glm::vec4(1.0f);
		float width_ = 1.0f;
		mutable bool dirty_ = false;
        mutable bool can_draw_body_ = false;
        mutable bool can_draw_head_ = false;

	public:
		StaticArrowExtension(Unbatched = kUnbatched);
		StaticArrowExtension(PolygonBatch& batch);

    public:
        internal::PolygonReference::accessor get_batch() const;
        void set_batch(Unbatched);
        void set_batch(PolygonBatch& batch);

        void set_camera_invariant(bool camera_invariant) const;
        bool is_camera_invariant() const;

		void draw() const;

		glm::vec2 get_start() const;
        glm::vec2& set_start();
		glm::vec2 get_end() const;
        glm::vec2& set_end();
		float get_width() const;
        void set_width(float w);
		float get_head_width() const;
        void set_head_width(float w);
		float get_head_height() const;
        void set_head_height(float h);

        glm::vec4 get_start_color() const;
        glm::vec4& set_start_color();
        glm::vec4 get_end_color() const;
        glm::vec4& set_end_color();
        void set_color(glm::vec4 color);

        void scale_head(float by);
        void adjust_standard_head_for_width(float w);

		std::vector<glm::vec2> get_all_points() const;

	private:
		void clean() const;
	};
}
