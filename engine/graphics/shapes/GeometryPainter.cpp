#include "GeometryPainter.h"

#include "core/context/Platform.h"
#include "core/context/rendering/Rendering.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Scopes.h"

namespace oly::rendering
{
	PolygonBatch& internal::get_polygon_batch(GeometryPainter& painter)
	{
		return painter.get_polygon_batch();
	}

	EllipseBatch& internal::get_ellipse_batch(GeometryPainter& painter)
	{
		return painter.get_ellipse_batch();
	}

	GeometryPainter::WindowResizeHandler::WindowResizeHandler(GeometryPainter& painter)
		: painter(painter)
	{
	}

	bool GeometryPainter::WindowResizeHandler::consume(const input::WindowResizeEventData& data)
	{
		if (auto sprite_batch = painter.sprite.get_batch())
		{
			const Camera2D& camera = *sprite_batch->camera;
			if (!camera.is_stretch())
			{
				painter.set_sprite_scale(camera.get_viewport().size() / glm::vec2(painter.dimensions));
				painter.dirty = true;
			}
		}
		return false;
	}

	void GeometryPainter::PaintSupport::pre_polygon_draw()
	{
		if (batch == Batch::ELLIPSE)
			painter.render_ellipses();
		batch = Batch::POLYGON;
	}

	void GeometryPainter::PaintSupport::pre_ellipse_draw()
	{
		if (batch == Batch::POLYGON)
			painter.render_polygons();
		batch = Batch::ELLIPSE;
	}

	void GeometryPainter::PaintSupport::final_flush()
	{
		if (batch == Batch::ELLIPSE)
			painter.render_ellipses();
		else if (batch == Batch::POLYGON)
			painter.render_polygons();
		batch = Batch::NONE;
	}

	GeometryPainter::GeometryPainter(const rendering::GeometryPainter::PaintFunction& paint_fn)
		: sprite(), texture(GL_TEXTURE_2D), window_resize_handler(*this), paint_fn(paint_fn)
	{
		sync_sprite_batch();
		set_dimensions(context::get_platform().window().get_size());
		setup_texture();
	}

	GeometryPainter::GeometryPainter(const rendering::GeometryPainter::PaintFunction& paint_fn, Unbatched)
		: sprite(UNBATCHED), texture(GL_TEXTURE_2D), window_resize_handler(*this), paint_fn(paint_fn)
	{
		sync_sprite_batch();
		set_dimensions(context::get_platform().window().get_size());
		setup_texture();
	}

	GeometryPainter::GeometryPainter(const rendering::GeometryPainter::PaintFunction& paint_fn, rendering::SpriteBatch& batch)
		: sprite(batch), texture(GL_TEXTURE_2D), window_resize_handler(*this), paint_fn(paint_fn)
	{
		sync_sprite_batch();
		set_dimensions(context::get_platform().window().get_size());
		setup_texture();
	}

	GeometryPainter::GeometryPainter(const GeometryPainter& other)
		: window_resize_handler(*this), sprite(other.sprite), dirty(other.dirty), texture(GL_TEXTURE_2D), paint_fn(other.paint_fn)
	{
		sync_sprite_batch();
		set_dimensions(other.dimensions);
		copy_texture(*other.texture);
	}

	GeometryPainter::GeometryPainter(GeometryPainter&& other) noexcept
		: window_resize_handler(*this), sprite(std::move(other.sprite)), framebuffer(std::move(other.framebuffer)), texture(std::move(other.texture)),
		dirty(other.dirty), paint_fn(std::move(other.paint_fn))
	{
		sync_sprite_batch();
		set_dimensions(other.dimensions);
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
			set_dimensions(other.dimensions);
			dirty = other.dirty;
			paint_fn = other.paint_fn;

			*texture = graphics::BindlessTexture(GL_TEXTURE_2D);
			copy_texture(*other.texture);
			sync_sprite_batch();
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
			set_dimensions(other.dimensions);
			dirty = other.dirty;
			paint_fn = std::move(other.paint_fn);
			sync_sprite_batch();
		}
		return *this;
	}

	void GeometryPainter::render_polygons() const
	{
		polygon_batch->render(projection);
	}
	
	void GeometryPainter::render_ellipses() const
	{
		ellipse_batch->render(projection);
	}

	void GeometryPainter::write_texture() const
	{
		if (auto sprite_batch = sprite.get_batch())
		{
			dirty = false;
			context::ScopedFullFramebufferDrawing drawing(*sprite_batch->camera, framebuffer, dimensions);
			PaintSupport ps(*this);
			paint_fn(ps);
			ps.final_flush();
		}
		else
		{
			OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Cannot write texture to using null sprite batch" << LOG.nl;
		}
	}

	void GeometryPainter::set_sprite_scale(glm::vec2 scale)
	{
		glm::mat3 m = 1.0f;
		m[0][0] = scale.x;
		m[1][1] = scale.y;
		sprite.set_transform(m);
	}

	void GeometryPainter::set_dimensions(glm::ivec2 dimensions)
	{
		this->dimensions = dimensions;
		projection = glm::ortho(-0.5f * dimensions[0], 0.5f * dimensions[0], -0.5f * dimensions[1], 0.5f * dimensions[1]);
	}

	void GeometryPainter::draw() const
	{
		if (dirty)
			write_texture();
		sprite.draw();
	}

	void GeometryPainter::regen_to_current_resolution()
	{
		*texture = graphics::BindlessTexture(GL_TEXTURE_2D);
		set_dimensions(context::get_platform().window().get_size());
		setup_texture();
		dirty = true;
	}

	void GeometryPainter::sync_sprite_batch()
	{
		polygon_batch->camera = nullptr;
		ellipse_batch->camera = nullptr;

		window_resize_handler.detach();
		if (auto sprite_batch = sprite.get_batch())
			window_resize_handler.attach(sprite_batch->camera.base());
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
		glm::vec2 sprite_scale = glm::vec2(1.0f);
		if (auto sprite_batch = sprite.get_batch())
			sprite_scale = sprite_batch->camera->get_viewport().size() / glm::vec2(dimensions);
		set_sprite_scale(sprite_scale);
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
