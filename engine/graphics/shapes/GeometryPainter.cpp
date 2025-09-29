#include "GeometryPainter.h"

#include "core/context/Platform.h"
#include "core/context/rendering/Rendering.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Scopes.h"

namespace oly::rendering
{
	GeometryPainter::WindowResizeHandler::WindowResizeHandler(GeometryPainter* painter)
		: painter(painter)
	{
		attach(&context::get_wr_drawer());
		set_projection();
	}

	bool GeometryPainter::WindowResizeHandler::consume(const input::WindowResizeEventData& data)
	{
		auto& wr = context::get_wr_viewport();
		if (!wr.stretch)
		{
			painter->set_sprite_scale(wr.get_size() / glm::vec2(painter->dimensions));
			painter->dirty = true;
			set_projection();
		}
		return false;
	}

	void GeometryPainter::WindowResizeHandler::set_projection()
	{
		auto viewport = context::get_wr_viewport().get_viewport();
		glm::vec4 bounds = 0.5f * glm::vec4{ -viewport.w, viewport.w, -viewport.h, viewport.h };
		glm::mat3 projection = glm::ortho(bounds[0], bounds[1], bounds[2], bounds[3]);
		painter->polygon_batch.projection = projection;
		painter->ellipse_batch.projection = projection;
	}

	void GeometryPainter::PaintSupport::pre_polygon_draw()
	{
		if (batch == Batch::ELLIPSE)
			painter.ellipse_batch.render();
		batch = Batch::POLYGON;
	}

	void GeometryPainter::PaintSupport::pre_ellipse_draw()
	{
		if (batch == Batch::POLYGON)
			painter.polygon_batch.render();
		batch = Batch::ELLIPSE;
	}

	void GeometryPainter::PaintSupport::final_flush()
	{
		if (batch == Batch::ELLIPSE)
			painter.ellipse_batch.render();
		else if (batch == Batch::POLYGON)
			painter.polygon_batch.render();
		batch = Batch::NONE;
	}

	GeometryPainter::GeometryPainter(const rendering::GeometryPainter::PaintFunction& paint_fn)
		: sprite(), texture(GL_TEXTURE_2D), window_resize_handler(this), paint_fn(paint_fn)
	{
		dimensions = context::get_platform().window().get_size();
		setup_texture();
	}

	GeometryPainter::GeometryPainter(const rendering::GeometryPainter::PaintFunction& paint_fn, Unbatched)
		: sprite(UNBATCHED), texture(GL_TEXTURE_2D), window_resize_handler(this), paint_fn(paint_fn)
	{
		dimensions = context::get_platform().window().get_size();
		setup_texture();
	}

	GeometryPainter::GeometryPainter(const rendering::GeometryPainter::PaintFunction& paint_fn, rendering::SpriteBatch& batch)
		: sprite(batch), texture(GL_TEXTURE_2D), window_resize_handler(this), paint_fn(paint_fn)
	{
		dimensions = context::get_platform().window().get_size();
		setup_texture();
	}

	GeometryPainter::GeometryPainter(const GeometryPainter& other)
		: window_resize_handler(this), sprite(other.sprite), dimensions(other.dimensions), dirty(other.dirty), texture(GL_TEXTURE_2D), paint_fn(other.paint_fn)
	{
		copy_texture(*other.texture);
	}

	GeometryPainter::GeometryPainter(GeometryPainter&& other) noexcept
		: window_resize_handler(this), sprite(std::move(other.sprite)), framebuffer(std::move(other.framebuffer)), texture(std::move(other.texture)),
		dimensions(other.dimensions), dirty(other.dirty), paint_fn(std::move(other.paint_fn))
	{
	}

	GeometryPainter::~GeometryPainter()
	{
		window_resize_handler.detach();
	}

	GeometryPainter& GeometryPainter::operator=(const GeometryPainter& other)
	{
		if (this != &other)
		{
			sprite = other.sprite;
			dimensions = other.dimensions;
			dirty = other.dirty;
			paint_fn = other.paint_fn;

			*texture = graphics::BindlessTexture(GL_TEXTURE_2D);
			copy_texture(*other.texture);
		}
		return *this;
	}

	GeometryPainter& GeometryPainter::operator=(GeometryPainter&& other) noexcept
	{
		if (this != &other)
		{
			sprite = std::move(other.sprite);
			framebuffer = std::move(other.framebuffer);
			texture = std::move(other.texture);
			dimensions = other.dimensions;
			dirty = other.dirty;
			paint_fn = std::move(other.paint_fn);
		}
		return *this;
	}

	void GeometryPainter::write_texture() const
	{
		context::ScopedFullFramebufferDrawing drawing(framebuffer, dimensions);
		PaintSupport ps(*this);
		paint_fn(ps);
		ps.final_flush();
	}

	void GeometryPainter::set_sprite_scale(glm::vec2 scale)
	{
		glm::mat3 m = 1.0f;
		m[0][0] = scale.x;
		m[1][1] = scale.y;
		sprite.set_transform(m);
	}

	void GeometryPainter::draw() const
	{
		if (dirty)
		{
			dirty = false;
			write_texture();
		}
		sprite.draw();
	}

	void GeometryPainter::regen_to_current_resolution()
	{
		*texture = graphics::BindlessTexture(GL_TEXTURE_2D);
		dimensions = context::get_platform().window().get_size();
		setup_texture();
		dirty = true;
	}

	void GeometryPainter::setup_texture()
	{
		tex_image();
		sync_texture();
	}

	void GeometryPainter::copy_texture(const graphics::BindlessTexture& other)
	{
		tex_image();
		glCopyImageSubData(other, GL_TEXTURE_2D, 0, 0, 0, 0, *texture, GL_TEXTURE_2D, 0, 0, 0, 0, dimensions.x, dimensions.y, 1);
		sync_texture();
	}

	void GeometryPainter::sync_texture()
	{
		set_and_use_texture_handle();
		setup_framebuffer();
		set_sprite_scale(context::get_wr_viewport().get_size() / glm::vec2(dimensions));
		sprite.set_texture(texture, dimensions);
	}

	void GeometryPainter::tex_image()
	{
		graphics::tex::storage_2d(*texture, { .w = dimensions.x, .h = dimensions.y, .cpp = TEXTURE_CPP });
	}

	void GeometryPainter::set_and_use_texture_handle()
	{
		texture->texture().set_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		texture->texture().set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		texture->set_and_use_handle();
	}

	void GeometryPainter::setup_framebuffer()
	{
		framebuffer.bind();
		framebuffer.attach_2d_texture(graphics::Framebuffer::ColorAttachment::color(0), *texture);
		OLY_ASSERT(framebuffer.status() == graphics::Framebuffer::Status::COMPLETE);
		framebuffer.unbind();
	}
}
