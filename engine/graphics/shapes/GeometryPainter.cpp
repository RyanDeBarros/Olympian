#include "GeometryPainter.h"

#include "core/context/Platform.h"

#include "core/context/rendering/Rendering.h"
#include "core/context/rendering/Sprites.h"

namespace oly::rendering
{
    GeometryPainter::GeometryPainter(const GeometryPainter& other)
    {
    }

    GeometryPainter::GeometryPainter(GeometryPainter&& other) noexcept
        : framebuffer_(std::move(other.framebuffer_))
    {
    }

    GeometryPainter& GeometryPainter::operator=(const GeometryPainter& other)
    {
        return *this;
    }

    GeometryPainter& GeometryPainter::operator=(GeometryPainter&& other) noexcept
    {
        if (this != &other)
            framebuffer_ = std::move(other.framebuffer_);
        return *this;
    }

    const PolygonBatch& GeometryPainter::get_polygon_batch() const
    {
        return polygon_batch_;
    }

    PolygonBatch& GeometryPainter::get_polygon_batch()
    {
        return polygon_batch_;
    }

    const EllipseBatch& GeometryPainter::get_ellipse_batch() const
    {
        return ellipse_batch_;
    }

    EllipseBatch& GeometryPainter::get_ellipse_batch()
    {
        return ellipse_batch_;
    }

    GeometryPainter::PaintContext::PaintContext(
        GeometryPainter& painter,
        const Camera2DRef& camera,
        math::IRect2D bounds,
        float rotation,
        glm::vec2 scale,
        int texture_cpp)
        : painter_(painter)
        , scope_(*camera, {}, false, bounds.size())
        , dimensions_(bounds.size())
        , texture_(GL_TEXTURE_2D)
    {
        painter_.context_locked_ = true;

        Camera2DRef new_camera = kRefInit;
        new_camera->project_to_rect((math::Rect2D)bounds);
        new_camera->transformer.set_local().rotation = rotation;
        new_camera->transformer.set_local().scale = scale;
        painter_.polygon_batch_->camera = new_camera;
        painter_.ellipse_batch_->camera = new_camera;

        graphics::tex::storage_2d(*texture_, { .w = dimensions_.x, .h = dimensions_.y, .cpp = texture_cpp });
        texture_->texture().set_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        texture_->texture().set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        texture_->set_and_use_handle();

        painter_.framebuffer_.bind(graphics::Framebuffer::Target::Draw);
        painter_.framebuffer_.attach_2d_texture(graphics::Framebuffer::ColorAttachment::color(0), *texture_);
        OLY_ASSERT(painter_.framebuffer_.status() == graphics::Framebuffer::Status::Complete);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    GeometryPainter::PaintContext::~PaintContext()
    {
        painter_.framebuffer_.detach_texture(graphics::Framebuffer::ColorAttachment::color(0));
        painter_.framebuffer_.unbind(graphics::Framebuffer::Target::Draw);
        painter_.context_locked_ = false;
    }

    void GeometryPainter::PaintContext::render()
    {
        painter_.ellipse_batch_->render();
        painter_.polygon_batch_->render();
    }

    glm::vec2 GeometryPainter::PaintContext::get_texture_size() const
    {
        return dimensions_;
    }

    graphics::BindlessTextureRef GeometryPainter::PaintContext::get_texture() const
    {
        return texture_;
    }

    void GeometryPainter::PaintContext::set_texture(StaticSprite& sprite) const
    {
        sprite.set_texture(texture_, dimensions_);
    }

    void GeometryPainter::PaintContext::set_texture(Sprite& sprite) const
    {
        sprite.set_texture(texture_, dimensions_);
    }

    GeometryPainter::PaintContext GeometryPainter::paint_context(
        const Camera2DRef& camera,
        math::IRect2D bounds,
        float rotation,
        glm::vec2 scale,
        int texture_cpp)
    {
        if (context_locked_)
            throw Error(ErrorCode::LockedResource);

        if (bounds.width() <= 0 || bounds.height() <= 0)
            throw Error(ErrorCode::InvalidSize);

        return PaintContext(*this, camera, bounds, rotation, scale, texture_cpp);
    }
}
