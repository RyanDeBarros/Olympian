#include "Arrow.h"

#include "core/base/Approximate.h"

#include "core/cmath/Triangulation.h"

namespace oly::rendering
{
	ArrowExtension::ArrowExtension(Unbatched)
		: body_(kUnbatched)
        , head_(kUnbatched)
	{
		body_.transformer.attach_parent(&transformer_);
		head_.transformer.attach_parent(&transformer_);
	}

	ArrowExtension::ArrowExtension(PolygonBatch& batch)
		: body_(batch)
        , head_(batch)
	{
		body_.transformer.attach_parent(&transformer_);
		body_.set_points().resize(4);
		body_.set_points()[0] = { 0.0f, -0.5f };
		body_.set_points()[1] = { 1.0f, -0.5f };
		body_.set_points()[2] = { 1.0f,  0.5f };
		body_.set_points()[3] = { 0.0f,  0.5f };

		head_.transformer.attach_parent(&transformer_);
		head_.set_points().resize(3);
		head_.set_points()[0] = { 1.0f, -0.5f };
		head_.set_points()[1] = { 2.0f,  0.0f };
		head_.set_points()[2] = { 1.0f,  0.5f };
	}

    internal::PolygonReference::accessor ArrowExtension::get_batch() const
    {
        return body_.get_batch();
    }
    
    void ArrowExtension::set_batch(Unbatched)
    {
        body_.set_batch(kUnbatched);
        head_.set_batch(kUnbatched);
    }
    
    void ArrowExtension::set_batch(PolygonBatch& batch)
    {
        body_.set_batch(batch);
        head_.set_batch(batch);
    }

    void ArrowExtension::set_camera_invariant(bool camera_invariant) const
    {
        body_.set_camera_invariant(camera_invariant);
        head_.set_camera_invariant(camera_invariant);
    }
    
    bool ArrowExtension::is_camera_invariant() const
    {
        return body_.is_camera_invariant();
    }

	void ArrowExtension::draw() const
	{
		if (dirty_)
		{
			dirty_ = false;

			float length = glm::length(start_ - end_);
			if (near_zero(length))
			{
				can_draw_body_ = false;
				can_draw_head_ = false;
			}
			else if (length <= head_height_)
			{
				can_draw_body_ = false;
				can_draw_head_ = true;

				head_.set_points().resize(3);
				head_.set_points()[0] = { 0.0f,  -0.5f * head_width_ };
				head_.set_points()[1] = { length, 0.0f };
				head_.set_points()[2] = { 0.0f,   0.5f * head_width_ };

				glm::mat2 rotation_matrix = UnitVector2D(end_ - start_).rotation_matrix();
				for (glm::vec2& point : head_.set_points())
					point = start_ + rotation_matrix * point;

				head_.set_colors().resize(3);
				head_.set_colors()[0] = start_color_;
				head_.set_colors()[1] = end_color_;
				head_.set_colors()[2] = start_color_;
			}
			else
			{
				can_draw_body_ = true;
				can_draw_head_ = true;

				body_.set_points().resize(4);
				body_.set_points()[0] = { 0.0f, -0.5f * width_ };
				body_.set_points()[1] = { length - head_height_, -0.5f * width_ };
				body_.set_points()[2] = { length - head_height_,  0.5f * width_ };
				body_.set_points()[3] = { 0.0f,  0.5f * width_ };

				head_.set_points().resize(3);
				head_.set_points()[0] = { length - head_height_, -0.5f * head_width_ };
				head_.set_points()[1] = { length, 0.0f };
				head_.set_points()[2] = { length - head_height_,  0.5f * head_width_ };

				glm::mat2 rotation_matrix = UnitVector2D(end_ - start_).rotation_matrix();
				for (glm::vec2& point : body_.set_points())
					point = start_ + rotation_matrix * point;
				for (glm::vec2& point : head_.set_points())
					point = start_ + rotation_matrix * point;

				body_.set_colors().resize(4);
				body_.set_colors()[0] = start_color_;
				body_.set_colors()[1] = glm::mix(start_color_, end_color_, 1.0f - (head_height_ / length));
				body_.set_colors()[2] = glm::mix(start_color_, end_color_, 1.0f - (head_height_ / length));
				body_.set_colors()[3] = start_color_;

				head_.set_colors().resize(4);
				head_.set_colors()[0] = glm::mix(start_color_, end_color_, 1.0f - (head_height_ / length));
				head_.set_colors()[1] = end_color_;
				head_.set_colors()[2] = glm::mix(start_color_, end_color_, 1.0f - (head_height_ / length));
			}
		}

		if (can_draw_body_)
			body_.draw();
		if (can_draw_head_)
			head_.draw();
	}

    glm::vec2 ArrowExtension::get_start() const
    {
        return start_;
    }
    
    glm::vec2& ArrowExtension::set_start()
    {
        dirty_ = true;
        return start_;
    }
    
    glm::vec2 ArrowExtension::get_end() const
    {
        return end_;
    }
    
    glm::vec2& ArrowExtension::set_end()
    {
        dirty_ = true;
        return end_;
    }
    
    float ArrowExtension::get_width() const
    {
        return width_;
    }
    
    void ArrowExtension::set_width(float w)
    {
        if (w > 0.0f && w != width_)
        {
            dirty_ = true;
            width_ = w;
        }
    }
    
    float ArrowExtension::get_head_width() const
    {
        return head_width_;
    }
    
    void ArrowExtension::set_head_width(float w)
    {
        if (w > 0.0f && w != head_width_)
        {
            dirty_ = true;
            head_width_ = w;
        }
    }
    
    float ArrowExtension::get_head_height() const
    {
        return head_height_;
    }
    
    void ArrowExtension::set_head_height(float h)
    {
        if (h > 0.0f && h != head_height_)
        {
            dirty_ = true;
            head_height_ = h;
        }
    }

    glm::vec4 ArrowExtension::get_start_color() const
    {
        return start_color_;
    }
    
    glm::vec4& ArrowExtension::set_start_color()
    {
        dirty_ = true;
        return start_color_;
    }
    
    glm::vec4 ArrowExtension::get_end_color() const
    {
        return end_color_;
    }
    
    glm::vec4& ArrowExtension::set_end_color()
    {
        dirty_ = true;
        return end_color_;
    }
    
    void ArrowExtension::set_color(glm::vec4 color)
    {
        set_start_color() = color;
        set_end_color() = color;
    }

    const Transformer2D& ArrowExtension::transformer() const
    {
        return transformer_;
    }
    
    Transformer2D& ArrowExtension::transformer()
    {
        return transformer_;
    }
    
    const Transform2D& ArrowExtension::get_local() const
    {
        return transformer_.get_local();
    }
    
    Transform2D& ArrowExtension::set_local()
    {
        return transformer_.set_local();
    }

    void ArrowExtension::scale_head(float by)
    {
        set_head_width(head_width_ * by);
        set_head_height(head_height_ * by);
    }
    
    void ArrowExtension::adjust_standard_head_for_width(float w)
    {
        if (w > 0.0f)
        {
            dirty_ = true;
            width_ = w;
            head_width_ = 3.0f * w;
            head_height_ = 3.0f * w;
        }
    }

	StaticArrowExtension::StaticArrowExtension(Unbatched)
		: body_(kUnbatched)
        , head_(kUnbatched)
	{
	}

	StaticArrowExtension::StaticArrowExtension(PolygonBatch& batch)
		: body_(batch)
        , head_(batch)
	{
		body_.set_points().resize(4);
		body_.set_points()[0] = { 0.0f, -0.5f };
		body_.set_points()[1] = { 1.0f, -0.5f };
		body_.set_points()[2] = { 1.0f,  0.5f };
		body_.set_points()[3] = { 0.0f,  0.5f };

		head_.set_points().resize(3);
		head_.set_points()[0] = { 1.0f, -0.5f };
		head_.set_points()[1] = { 2.0f,  0.0f };
		head_.set_points()[2] = { 1.0f,  0.5f };
	}

    internal::PolygonReference::accessor StaticArrowExtension::get_batch() const
    {
        return body_.get_batch();
    }
    
    void StaticArrowExtension::set_batch(Unbatched)
    {
        body_.set_batch(kUnbatched);
        head_.set_batch(kUnbatched);
    }
    
    void StaticArrowExtension::set_batch(PolygonBatch& batch)
    {
        body_.set_batch(batch);
        head_.set_batch(batch);
    }

    void StaticArrowExtension::set_camera_invariant(bool camera_invariant) const
    {
        body_.set_camera_invariant(camera_invariant);
        head_.set_camera_invariant(camera_invariant);
    }

    bool StaticArrowExtension::is_camera_invariant() const
    {
        return body_.is_camera_invariant();
    }

	void StaticArrowExtension::draw() const
	{
		if (dirty_)
			clean();

		if (can_draw_body_)
			body_.draw();

		if (can_draw_head_)
			head_.draw();
	}

    glm::vec2 StaticArrowExtension::get_start() const
    {
        return start_;
    }

    glm::vec2& StaticArrowExtension::set_start()
    {
        dirty_ = true;
        return start_;
    }

    glm::vec2 StaticArrowExtension::get_end() const
    {
        return end_;
    }

    glm::vec2& StaticArrowExtension::set_end()
    {
        dirty_ = true;
        return end_;
    }

    float StaticArrowExtension::get_width() const
    {
        return width_;
    }

    void StaticArrowExtension::set_width(float w)
    {
        if (w > 0.0f && w != width_)
        {
            dirty_ = true;
            width_ = w;
        }
    }

    float StaticArrowExtension::get_head_width() const
    {
        return head_width_;
    }

    void StaticArrowExtension::set_head_width(float w)
    {
        if (w > 0.0f && w != head_width_)
        {
            dirty_ = true;
            head_width_ = w;
        }
    }

    float StaticArrowExtension::get_head_height() const
    {
        return head_height_;
    }

    void StaticArrowExtension::set_head_height(float h)
    {
        if (h > 0.0f && h != head_height_)
        {
            dirty_ = true;
            head_height_ = h;
        }
    }

    glm::vec4 StaticArrowExtension::get_start_color() const
    {
        return start_color_;
    }

    glm::vec4& StaticArrowExtension::set_start_color()
    {
        dirty_ = true;
        return start_color_;
    }

    glm::vec4 StaticArrowExtension::get_end_color() const
    {
        return end_color_;
    }

    glm::vec4& StaticArrowExtension::set_end_color()
    {
        dirty_ = true;
        return end_color_;
    }

    void StaticArrowExtension::set_color(glm::vec4 color)
    {
        set_start_color() = color;
        set_end_color() = color;
    }

    void StaticArrowExtension::scale_head(float by)
    {
        set_head_width(head_width_ * by);
        set_head_height(head_height_ * by);
    }
    
    void StaticArrowExtension::adjust_standard_head_for_width(float w)
    {
        if (w > 0.0f)
        {
            dirty_ = true;
            width_ = w;
            head_width_ = 3.0f * w;
            head_height_ = 3.0f * w;
        }
    }

	std::vector<glm::vec2> StaticArrowExtension::get_all_points() const
	{
		if (dirty_)
			clean();

		std::vector<glm::vec2> points;

		if (can_draw_body_)
			points.insert(points.end(), body_.get_points().begin(), body_.get_points().end());
		
        if (can_draw_head_)
			points.insert(points.end(), head_.get_points().begin(), head_.get_points().end());
		
        return points;
	}

	void StaticArrowExtension::clean() const
	{
		dirty_ = false;

		float length = glm::length(start_ - end_);
		if (near_zero(length))
		{
			can_draw_body_ = false;
			can_draw_head_ = false;
		}
		else if (length <= head_height_)
		{
			can_draw_body_ = false;
			can_draw_head_ = true;

			head_.set_points().resize(3);
			head_.set_points()[0] = { 0.0f, -0.5f * head_width_ };
			head_.set_points()[1] = { length,               0.0f };
			head_.set_points()[2] = { 0.0f,  0.5f * head_width_ };

			glm::mat2 rotation_matrix = UnitVector2D(end_ - start_).rotation_matrix();
			for (glm::vec2& point : head_.set_points())
				point = start_ + rotation_matrix * point;

			head_.set_colors().resize(3);
			head_.set_colors()[0] = start_color_;
			head_.set_colors()[1] = end_color_;
			head_.set_colors()[2] = start_color_;
		}
		else
		{
			can_draw_body_ = true;
			can_draw_head_ = true;

			body_.set_points().resize(4);
			body_.set_points()[0] = { 0.0f, -0.5f * width_ };
			body_.set_points()[1] = { length - head_height_, -0.5f * width_ };
			body_.set_points()[2] = { length - head_height_,  0.5f * width_ };
			body_.set_points()[3] = { 0.0f,  0.5f * width_ };

			head_.set_points().resize(3);
			head_.set_points()[0] = { length - head_height_, -0.5f * head_width_ };
			head_.set_points()[1] = { length,               0.0f };
			head_.set_points()[2] = { length - head_height_,  0.5f * head_width_ };

			glm::mat2 rotation_matrix = UnitVector2D(end_ - start_).rotation_matrix();
			for (glm::vec2& point : body_.set_points())
				point = start_ + rotation_matrix * point;
			for (glm::vec2& point : head_.set_points())
				point = start_ + rotation_matrix * point;

			body_.set_colors().resize(4);
			body_.set_colors()[0] = start_color_;
			body_.set_colors()[1] = glm::mix(start_color_, end_color_, 1.0f - (head_height_ / length));
			body_.set_colors()[2] = glm::mix(start_color_, end_color_, 1.0f - (head_height_ / length));
			body_.set_colors()[3] = start_color_;

			head_.set_colors().resize(3);
			head_.set_colors()[0] = glm::mix(start_color_, end_color_, 1.0f - (head_height_ / length));
			head_.set_colors()[1] = end_color_;
			head_.set_colors()[2] = glm::mix(start_color_, end_color_, 1.0f - (head_height_ / length));
		}
	}
}
