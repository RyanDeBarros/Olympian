#pragma once

#include "core/context/rendering/Scopes.h"

#include "graphics/shapes/Ellipses.h"
#include "graphics/shapes/Polygons.h"

#include "graphics/sprites/Sprite.h"

#include "graphics/backend/basic/Framebuffers.h"

namespace oly::rendering
{
    class GeometryPainter
    {
        PolygonBatch polygon_batch_;
        EllipseBatch ellipse_batch_;
        graphics::Framebuffer framebuffer_;
        mutable bool context_locked_ = false;

    public:
        GeometryPainter() = default;
        GeometryPainter(const GeometryPainter&);
        GeometryPainter(GeometryPainter&&) noexcept;

        GeometryPainter& operator=(const GeometryPainter&);
        GeometryPainter& operator=(GeometryPainter&&) noexcept;

    public:
        const PolygonBatch& get_polygon_batch() const;
        PolygonBatch& get_polygon_batch();
        const EllipseBatch& get_ellipse_batch() const;
        EllipseBatch& get_ellipse_batch();

        class PaintContext
        {
            friend class GeometryPainter;

        private:
            GeometryPainter& painter_;
            context::ScopedViewportChange scope_;
            graphics::BindlessTextureRef texture_;
            glm::ivec2 dimensions_;

        private:
            PaintContext(
                GeometryPainter& painter,
                const Camera2DRef& camera,
                math::IRect2D bounds,
                float rotation,
                glm::vec2 scale,
                int texture_cpp);

            PaintContext(const PaintContext&) = delete;
            PaintContext(PaintContext&&) = delete;

        public:
            ~PaintContext();

            void render();

            glm::vec2 get_texture_size() const;
            graphics::BindlessTextureRef get_texture() const;

            void set_texture(StaticSprite& sprite) const;
            void set_texture(Sprite& sprite) const;
        };

        PaintContext paint_context(
            const Camera2DRef& camera,
            math::IRect2D bounds,
            float rotation = 0.0f,
            glm::vec2 scale = glm::vec2(1.0f),
            int texture_cpp = 4);
    };
}
