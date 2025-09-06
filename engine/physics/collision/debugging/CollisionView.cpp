#include "CollisionView.h"

#include "core/context/Platform.h"
#include "core/context/rendering/Rendering.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Ellipses.h"
#include "core/context/rendering/Polygons.h"

namespace oly::debug
{
	CollisionView::CollisionView(const CollisionView& other)
		: obj(other.obj)
	{
		for (CollisionLayer* layer : other.layers)
		{
			layers.insert(layer);
			layer->collision_views.insert(this);
			layer->dirty_views = true;
		}
	}

	CollisionView::CollisionView(CollisionView&& other) noexcept
		: obj(std::move(other.obj)), layers(std::move(other.layers))
	{
		for (CollisionLayer* layer : layers)
		{
			layer->collision_views.erase(&other);
			layer->collision_views.insert(this);
		}
	}

	CollisionView::~CollisionView()
	{
		for (CollisionLayer* layer : layers)
		{
			layer->collision_views.erase(this);
			layer->dirty_views = true;
		}
	}

	CollisionView& CollisionView::operator=(const CollisionView& other)
	{
		if (this != &other)
		{
			for (CollisionLayer* layer : layers)
			{
				if (!other.layers.count(layer))
				{
					layer->collision_views.erase(this);
					layer->dirty_views = true;
				}
			}

			obj = other.obj;
			
			for (CollisionLayer* layer : other.layers)
			{
				if (!layers.count(layer))
				{
					layer->collision_views.insert(this);
					layer->dirty_views = true;
				}
			}

			layers = other.layers;
		}
		return *this;
	}

	CollisionView& CollisionView::operator=(CollisionView&& other) noexcept
	{
		if (this != &other)
		{
			for (CollisionLayer* layer : layers)
			{
				layer->collision_views.erase(this);
				layer->dirty_views = true;
			}
			
			obj = std::move(other.obj);
			layers = std::move(other.layers);
			
			for (CollisionLayer* layer : layers)
			{
				layer->collision_views.erase(&other);
				layer->collision_views.insert(this);
			}
		}
		return *this;
	}

	void CollisionView::draw() const
	{
		// TODO v5 specialized shader for collision view
		std::visit([](const auto& view) {
			if constexpr (visiting_class_is<decltype(view), CollisionObject>)
				std::visit([](const auto& obj) { obj.draw(); }, view);
			else if constexpr (visiting_class_is<decltype(view), CollisionObjectGroup>)
				for (const auto& v : view)
					std::visit([](const auto& obj) { obj.draw(); }, v);
			}, obj);
	}

	void CollisionView::clear_view()
	{
		obj = EmptyCollision{};
		view_changed();
	}

	void CollisionView::set_view(CollisionObjectView&& obj)
	{
		this->obj = std::move(obj);
		view_changed();
	}

	size_t CollisionView::view_size() const
	{
		if (obj.index() == CollisionObjectViewType::EMPTY)
			return 0;
		else if (obj.index() == CollisionObjectViewType::SINGLE)
			return 1;
		else
			return std::get<CollisionObjectViewType::GROUP>(obj).size();
	}

	void CollisionView::resize_view(size_t size)
	{
		if (size == 0)
			clear_view();
		else if (obj.index() == CollisionObjectViewType::EMPTY)
		{
			if (size == 1)
			{
				obj = CollisionObject();
				view_changed();
			}
			else if (size > 1)
			{
				obj = CollisionObjectGroup(size);
				view_changed();
			}
		}
		else if (obj.index() == CollisionObjectViewType::SINGLE)
		{
			if (size > 1)
			{
				CollisionObject old_obj = std::move(std::get<CollisionObjectViewType::SINGLE>(obj));
				obj = CollisionObjectGroup(size);
				std::get<CollisionObjectViewType::GROUP>(obj)[0] = std::move(old_obj);
				view_changed();
			}
		}
		else
		{
			if (size == 1)
			{
				CollisionObject old_obj = std::move(std::get<CollisionObjectViewType::GROUP>(obj)[0]);
				obj = std::move(old_obj);
				view_changed();
			}
			else
			{
				std::get<CollisionObjectViewType::GROUP>(obj).resize(size);
				view_changed();
			}
		}
	}

	void CollisionView::set_view(size_t i, CollisionObject&& obj)
	{
		if (obj.index() == CollisionObjectViewType::EMPTY || obj.index() == CollisionObjectViewType::SINGLE)
		{
			if (i == 0)
			{
				this->obj = std::move(obj);
				view_changed();
			}
			else
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}
		else
		{
			std::get<CollisionObjectViewType::GROUP>(this->obj)[i] = std::move(obj);
			view_changed();
		}
	}

	void CollisionView::merge(CollisionView&& other)
	{
		if (other.obj.index() == CollisionObjectViewType::EMPTY)
			return;

		if (obj.index() == CollisionObjectViewType::EMPTY)
		{
			set_view(std::move(other.obj));
			return;
		}

		if (obj.index() == CollisionObjectViewType::SINGLE)
		{
			CollisionObject old_view = std::move(std::get<CollisionObjectViewType::SINGLE>(obj));
			obj = CollisionObjectGroup();
			std::get<CollisionObjectViewType::GROUP>(obj).push_back(std::move(old_view));
		}

		std::visit([&view = std::get<CollisionObjectViewType::GROUP>(obj)](auto&& other_view) {
			if constexpr (visiting_class_is<decltype(other_view), CollisionObject>)
				view.push_back(std::move(other_view));
			else if constexpr (visiting_class_is<decltype(other_view), CollisionObjectGroup>)
				view.insert(view.end(), std::make_move_iterator(other_view.begin()), std::make_move_iterator(other_view.end()));
			}, std::move(other.obj));

		view_changed();
	}

	void CollisionView::assign(CollisionLayer& layer)
	{
		if (std::find(layers.begin(), layers.end(), &layer) == layers.end())
		{
			layers.insert(&layer);
			layer.collision_views.insert(this);
			layer.dirty_views = true;
		}
	}

	void CollisionView::unassign(CollisionLayer& layer)
	{
		if (layers.erase(&layer))
		{
			layer.collision_views.erase(this);
			layer.dirty_views = true;
		}
	}

	const CollisionObject& CollisionView::get_view(size_t i) const
	{
		if (obj.index() == CollisionObjectViewType::EMPTY)
			throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		else if (obj.index() == CollisionObjectViewType::SINGLE)
		{
			if (i == 0)
				return std::get<CollisionObjectViewType::SINGLE>(obj);
			else
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}
		else
			return std::get<CollisionObjectViewType::GROUP>(obj)[i];
	}

	CollisionObject& CollisionView::get_view(size_t i)
	{
		if (obj.index() == CollisionObjectViewType::EMPTY)
			throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		else if (obj.index() == CollisionObjectViewType::SINGLE)
		{
			if (i == 0)
				return std::get<CollisionObjectViewType::SINGLE>(obj);
			else
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}
		else
			return std::get<CollisionObjectViewType::GROUP>(obj)[i];
	}

	void CollisionView::view_changed() const
	{
		for (CollisionLayer* layer : layers)
			layer->dirty_views = true;
	}

	void CollisionView::update_color(glm::vec4 color)
	{
		static const auto update_color = [](CollisionObject& obj, glm::vec4 color) {
			std::visit([color](auto&& obj) {
				if constexpr (visiting_class_is<decltype(obj), rendering::EllipseBatch::EllipseReference>)
					obj.set_color().fill_outer = color;
				else if constexpr (visiting_class_is<decltype(obj), rendering::StaticPolygon>)
				{
					obj.polygon.colors = { color };
					obj.send_colors_only();
				}
				else if constexpr (visiting_class_is<decltype(obj), rendering::StaticArrowExtension>)
					obj.set_color(color);
				}, obj);
			};

		bool updated = std::visit([color](auto&& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), CollisionObject>)
			{
				update_color(obj, color);
				return true;
			}
			else if constexpr (visiting_class_is<decltype(obj), CollisionObjectGroup>)
			{
				for (CollisionObject& subobj : obj)
					update_color(subobj, color);
				return !obj.empty();
			}
			else
				return false;
			}, obj);

		if (updated)
			view_changed();
	}

	CollisionLayer::WindowResizeHandler::WindowResizeHandler()
	{
		attach(&context::get_wr_drawer());
	}

	bool CollisionLayer::WindowResizeHandler::consume(const input::WindowResizeEventData& data)
	{
		auto& wr = context::get_wr_viewport();
		if (!wr.stretch)
		{
			layer->set_sprite_scale(wr.get_size() / glm::vec2(layer->dimensions));
			layer->dirty_views = true;
		}
		return false;
	}

	CollisionLayer::CollisionLayer()
		: texture(GL_TEXTURE_2D)
	{
		window_resize_handler.layer = this;
		setup_texture();
	}

	CollisionLayer::CollisionLayer(const CollisionLayer& other)
		: sprite(other.sprite), dimensions(other.dimensions), dirty_views(other.dirty_views), collision_views(other.collision_views),
		texture(GL_TEXTURE_2D)
	{
		window_resize_handler.layer = this;
		for (CollisionView* view : collision_views)
			view->layers.insert(this);

		const int cpp = 4;
		graphics::pixel_alignment_pre_send(cpp);
		graphics::tex_image_2d(GL_TEXTURE_2D, graphics::ImageDimensions{ .w = dimensions.x, .h = dimensions.y, .cpp = cpp });
		glCopyImageSubData(*other.texture, GL_TEXTURE_2D, 0, 0, 0, 0, *texture, GL_TEXTURE_2D, 0, 0, 0, 0, dimensions.x, dimensions.y, 1);
		graphics::pixel_alignment_post_send(cpp);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		texture->set_and_use_handle();

		framebuffer.bind();
		framebuffer.attach_2d_texture(graphics::Framebuffer::ColorAttachment::color(0), *texture);
		OLY_ASSERT(framebuffer.status() == graphics::Framebuffer::Status::COMPLETE);
		framebuffer.unbind();

		set_sprite_scale(context::get_wr_viewport().get_size() / glm::vec2(dimensions));
		sprite.set_texture(texture, dimensions);
	}

	CollisionLayer::CollisionLayer(CollisionLayer&& other) noexcept
		: sprite(std::move(other.sprite)), framebuffer(std::move(other.framebuffer)), texture(std::move(other.texture)),
		dimensions(other.dimensions), collision_views(std::move(other.collision_views)), dirty_views(other.dirty_views)
	{
		window_resize_handler.layer = this;
		for (CollisionView* view : collision_views)
		{
			view->layers.erase(&other);
			view->layers.insert(this);
		}
	}

	CollisionLayer::~CollisionLayer()
	{
		window_resize_handler.detach();
		for (CollisionView* view : collision_views)
			view->layers.erase(std::find(view->layers.begin(), view->layers.end(), this));
	}

	CollisionLayer& CollisionLayer::operator=(const CollisionLayer& other)
	{
		if (this != &other)
		{
			for (CollisionView* view : collision_views)
			{
				if (!other.collision_views.count(view))
					view->layers.erase(this);
			}

			sprite = other.sprite;
			dimensions = other.dimensions;
			dirty_views = other.dirty_views;

			*texture = graphics::BindlessTexture(GL_TEXTURE_2D);

			const int cpp = 4;
			glBindTexture(GL_TEXTURE_2D, *texture);
			graphics::pixel_alignment_pre_send(cpp);
			graphics::tex_image_2d(GL_TEXTURE_2D, graphics::ImageDimensions{ .w = dimensions.x, .h = dimensions.y, .cpp = cpp });
			glCopyImageSubData(*other.texture, GL_TEXTURE_2D, 0, 0, 0, 0, *texture, GL_TEXTURE_2D, 0, 0, 0, 0, dimensions.x, dimensions.y, 1);
			graphics::pixel_alignment_post_send(cpp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			texture->set_and_use_handle();

			framebuffer.bind();
			framebuffer.attach_2d_texture(graphics::Framebuffer::ColorAttachment::color(0), *texture);
			OLY_ASSERT(framebuffer.status() == graphics::Framebuffer::Status::COMPLETE);
			framebuffer.unbind();

			set_sprite_scale(context::get_wr_viewport().get_size() / glm::vec2(dimensions));
			sprite.set_texture(texture, dimensions);
			
			for (CollisionView* view : other.collision_views)
			{
				if (!collision_views.count(view))
					view->layers.insert(this);
			}

			collision_views = other.collision_views;
		}
		return *this;
	}

	CollisionLayer& CollisionLayer::operator=(CollisionLayer&& other) noexcept
	{
		if (this != &other)
		{
			for (CollisionView* view : collision_views)
				view->layers.erase(std::find(view->layers.begin(), view->layers.end(), this));
			
			sprite = std::move(other.sprite);
			framebuffer = std::move(other.framebuffer);
			texture = std::move(other.texture);
			dimensions = other.dimensions;
			collision_views = std::move(other.collision_views);
			dirty_views = other.dirty_views;
			
			for (CollisionView* view : collision_views)
			{
				view->layers.erase(&other);
				view->layers.insert(this);
			}
		}
		return *this;
	}

	void CollisionLayer::write_texture() const
	{
		bool was_blending = context::blend_enabled();
		glm::vec4 clear_color = context::clear_color();
		// TODO v4 currently, it's necessary to flush the internal batches when switching framebuffers.
		// Ideally, make batches instances instead of singletons - and attach a batch per framebuffer + screen framebuffer.
		// Then, in sprite/ellipse/etc. constructor, attach to a particular batch - use internal pointer.
		context::flush_internal_rendering();
		framebuffer.bind();
		glViewport(0, 0, dimensions.x, dimensions.y);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		if (was_blending)
			glDisable(GL_BLEND);

		for (const auto& collision_view : collision_views)
			collision_view->draw();
		context::polygon_batch().render();
		context::ellipse_batch().render();

		context::flush_internal_rendering();
		framebuffer.unbind();
		context::set_standard_viewport();
		glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
		if (was_blending)
			glEnable(GL_BLEND);
	}

	void CollisionLayer::set_sprite_scale(glm::vec2 scale)
	{
		glm::mat3 m = 1.0f;
		m[0][0] = scale.x;
		m[1][1] = scale.y;
		sprite.set_transform(m);
	}

	void CollisionLayer::draw(BatchBarrier barrier) const
	{
		if (dirty_views)
		{
			dirty_views = false;
			write_texture();
		}
		sprite.draw(barrier);
	}

	void CollisionLayer::assign(CollisionView& view)
	{
		if (std::find(collision_views.begin(), collision_views.end(), &view) == collision_views.end())
		{
			collision_views.insert(&view);
			view.layers.insert(this);
			dirty_views = true;
		}
	}

	void CollisionLayer::unassign(CollisionView& view)
	{
		if (collision_views.erase(&view))
		{
			view.layers.erase(this);
			dirty_views = true;
		}
	}

	void CollisionLayer::regen_to_current_resolution()
	{
		*texture = graphics::BindlessTexture(GL_TEXTURE_2D);
		setup_texture();
		dirty_views = true;
	}

	void CollisionLayer::setup_texture()
	{
		dimensions = context::get_platform().window().get_size();

		const int cpp = 4;
		glBindTexture(GL_TEXTURE_2D, *texture);
		graphics::pixel_alignment_pre_send(cpp);
		graphics::tex_image_2d(GL_TEXTURE_2D, graphics::ImageDimensions{ .w = dimensions.x, .h = dimensions.y, .cpp = cpp });
		graphics::pixel_alignment_post_send(cpp);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		texture->set_and_use_handle();

		framebuffer.bind();
		framebuffer.attach_2d_texture(graphics::Framebuffer::ColorAttachment::color(0), *texture);
		OLY_ASSERT(framebuffer.status() == graphics::Framebuffer::Status::COMPLETE);
		framebuffer.unbind();

		set_sprite_scale(context::get_wr_viewport().get_size() / glm::vec2(dimensions));
		sprite.set_texture(texture, dimensions);
	}

	void render_layers()
	{
		context::render_sprites();
	}
}
