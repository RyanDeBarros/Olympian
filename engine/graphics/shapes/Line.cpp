#include "Line.h"

#include "core/base/Approximate.h"

namespace oly::rendering
{
    LineExtension::LineExtension(Unbatched)
        : poly_(kUnbatched)
    {
    }

    LineExtension::LineExtension(PolygonBatch& batch)
        : poly_(batch)
    {
        set_default_polygon();
    }

    internal::PolygonReference::accessor LineExtension::get_batch() const
    {
        return poly_.get_batch();
    }

    void LineExtension::set_batch(Unbatched)
    {
        poly_.set_batch(kUnbatched);
    }

    void LineExtension::set_batch(PolygonBatch& batch)
    {
        poly_.set_batch(batch);
    }

    // TODO technically set_camera_invariant()s everywhere can be const, but makes more sense to be non-const
    void LineExtension::set_camera_invariant(bool camera_invariant) const
    {
        poly_.set_camera_invariant(camera_invariant);
    }

    bool LineExtension::is_camera_invariant() const
    {
        return poly_.is_camera_invariant();
    }

    void LineExtension::draw() const
    {
        if (dirty_)
        {
            dirty_ = false;

            set_default_polygon();

            if (!approx(start_, end_))
            {
                glm::mat2 rotation{
                    glm::normalize(end_ - start_),
                    glm::normalize(glm::vec2{ -(end_ - start_).y, (end_ - start_).x })
                };

                for (size_t i = 0; i < 4; ++i)
                    poly_.set_points()[i] = rotation * poly_.get_points()[i];
            }

            poly_.set_points()[0] += start_;
            poly_.set_points()[1] += end_;
            poly_.set_points()[2] += end_;
            poly_.set_points()[3] += start_;
        }
        poly_.draw();
    }

    glm::vec2 LineExtension::get_start() const
    {
        return start_;
    }

    glm::vec2& LineExtension::set_start()
    {
        dirty_ = true;
        return start_;
    }

    glm::vec2 LineExtension::get_end() const
    {
        return end_;
    }

    glm::vec2& LineExtension::set_end()
    {
        dirty_ = true;
        return end_;
    }

    float LineExtension::get_width() const
    {
        return width_;
    }

    void LineExtension::set_width(float w)
    {
        if (w > 0.0f && w != width_)
        {
            dirty_ = true;
            width_ = w;
        }
    }

    glm::vec4 LineExtension::get_start_color() const
    {
        return start_color_;
    }

    glm::vec4& LineExtension::set_start_color()
    {
        dirty_ = true;
        return start_color_;
    }

    glm::vec4 LineExtension::get_end_color() const
    {
        return end_color_;
    }

    glm::vec4& LineExtension::set_end_color()
    {
        dirty_ = true;
        return end_color_;
    }

    void LineExtension::set_color(glm::vec4 color)
    {
        set_start_color() = color;
        set_end_color() = color;
    }

    const Transformer2D& LineExtension::transformer() const
    {
        return poly_.transformer;
    }

    Transformer2D& LineExtension::transformer()
    {
        return poly_.transformer;
    }

    const Transform2D& LineExtension::get_local() const
    {
        return poly_.get_local();
    }

    Transform2D& LineExtension::set_local()
    {
        return poly_.set_local();
    }

    void LineExtension::set_default_polygon() const
    {
        poly_.set_colors().resize(4);
        poly_.set_points().resize(4);

        poly_.set_colors()[0] = start_color_;
        poly_.set_colors()[1] = end_color_;
        poly_.set_colors()[2] = end_color_;
        poly_.set_colors()[3] = start_color_;

        poly_.set_points()[0] = 0.5f * glm::vec2{ 0.0f, -width_ };
        poly_.set_points()[1] = 0.5f * glm::vec2{ 0.0f, -width_ };
        poly_.set_points()[2] = 0.5f * glm::vec2{ 0.0f,  width_ };
        poly_.set_points()[3] = 0.5f * glm::vec2{ 0.0f,  width_ };
    }
}
