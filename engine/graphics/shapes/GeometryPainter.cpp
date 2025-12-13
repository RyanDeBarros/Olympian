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
		: framebuffer(std::move(other.framebuffer))
	{
	}
	
	GeometryPainter& GeometryPainter::operator=(const GeometryPainter& other)
	{
		return *this;
	}
	
	GeometryPainter& GeometryPainter::operator=(GeometryPainter&& other) noexcept
	{
		if (this != &other)
			framebuffer = std::move(other.framebuffer);
		return *this;
	}

	GeometryPainter::PaintContext::PaintContext(GeometryPainter& painter, const Camera2DRef& camera, math::IRect2D bounds, float rotation, glm::vec2 scale, int texture_cpp)
		: painter(painter), scope(*camera, {}, false, bounds.size()), dimensions(bounds.size()), texture(GL_TEXTURE_2D)
	{
		painter.context_locked = true;

		Camera2DRef new_camera = REF_INIT;
		new_camera->project_to_rect((math::Rect2D)bounds);
		new_camera->transformer.set_local().rotation = rotation;
		new_camera->transformer.set_local().scale = scale;
		painter.polygon_batch->camera = new_camera;
		painter.ellipse_batch->camera = new_camera;
		
		graphics::tex::storage_2d(*texture, { .w = dimensions.x, .h = dimensions.y, .cpp = texture_cpp });
		texture->texture().set_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		texture->texture().set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		texture->set_and_use_handle();

		painter.framebuffer.bind(graphics::Framebuffer::Target::DRAW);
		painter.framebuffer.attach_2d_texture(graphics::Framebuffer::ColorAttachment::color(0), *texture);
		OLY_ASSERT(painter.framebuffer.status() == graphics::Framebuffer::Status::COMPLETE);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	GeometryPainter::PaintContext::~PaintContext()
	{
		painter.framebuffer.detach_texture(graphics::Framebuffer::ColorAttachment::color(0));
		painter.framebuffer.unbind(graphics::Framebuffer::Target::DRAW);
		painter.context_locked = false;
	}

	void GeometryPainter::PaintContext::pre_polygon_draw()
	{
		if (batch == Batch::ELLIPSE)
			painter.ellipse_batch->render();
		batch = Batch::POLYGON;
	}

	void GeometryPainter::PaintContext::pre_ellipse_draw()
	{
		if (batch == Batch::POLYGON)
			painter.polygon_batch->render();
		batch = Batch::ELLIPSE;
	}

	void GeometryPainter::PaintContext::flush()
	{
		if (batch == Batch::ELLIPSE)
			painter.ellipse_batch->render();
		else if (batch == Batch::POLYGON)
			painter.polygon_batch->render();
		batch = Batch::NONE;
	}

	void GeometryPainter::PaintContext::set_texture(StaticSprite& sprite) const
	{
		sprite.set_texture(texture, dimensions);
	}

	void GeometryPainter::PaintContext::set_texture(Sprite& sprite) const
	{
		sprite.set_texture(texture, dimensions);
	}

	GeometryPainter::PaintContext GeometryPainter::paint_context(const Camera2DRef& camera, math::IRect2D bounds, float rotation, glm::vec2 scale, int texture_cpp)
	{
		if (context_locked)
			throw Error(ErrorCode::LOCKED_RESOURCE);

		if (bounds.width() <= 0 || bounds.height() <= 0)
			throw Error(ErrorCode::INVALID_SIZE);

		return PaintContext(*this, camera, bounds, rotation, scale, texture_cpp);
	}
}
