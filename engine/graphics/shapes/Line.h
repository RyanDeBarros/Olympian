#pragma once

#include "graphics/shapes/Polygons.h"

namespace oly::rendering
{
    class LineExtension
    {
        mutable Polygon poly_;
        glm::vec2 start_ = {};
        glm::vec2 end_ = {};
        glm::vec4 start_color_ = {};
        glm::vec4 end_color_ = {};
        float width_ = 1.0f;
        mutable bool dirty_ = false;

    public:
        LineExtension(Unbatched = kUnbatched);
        LineExtension(PolygonBatch& batch);

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

        glm::vec4 get_start_color() const;
        glm::vec4& set_start_color();
        glm::vec4 get_end_color() const;
        glm::vec4& set_end_color();
        void set_color(glm::vec4 color);

        const Transformer2D& transformer() const;
        Transformer2D& transformer();
        const Transform2D& get_local() const;
        Transform2D& set_local();

    private:
        void set_default_polygon() const;
    };
}
