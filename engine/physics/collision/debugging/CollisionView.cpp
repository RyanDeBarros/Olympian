#include "CollisionView.h"

#include "core/base/Context.h"

namespace oly::debug
{
	CollisionView::CollisionView(CollisionView&& other) noexcept
		: obj(std::move(other.obj)), layers(std::move(other.layers))
	{
		for (CollisionLayer* layer : layers)
		{
			auto it = std::find(layer->collision_views.begin(), layer->collision_views.end(), &other);
			if (it != layer->collision_views.end())
				*it = this;
		}
	}

	CollisionView::~CollisionView()
	{
		for (CollisionLayer* layer : layers)
		{
			layer->collision_views.erase(std::find(layer->collision_views.begin(), layer->collision_views.end(), this));
			layer->dirty_views = true;
		}
	}

	CollisionView& CollisionView::operator=(CollisionView&& other) noexcept
	{
		if (this != &other)
		{
			for (CollisionLayer* layer : layers)
			{
				layer->collision_views.erase(std::find(layer->collision_views.begin(), layer->collision_views.end(), this));
				layer->dirty_views = true;
			}
			
			obj = std::move(other.obj);
			layers = std::move(other.layers);
			
			for (CollisionLayer* layer : layers)
			{
				auto it = std::find(layer->collision_views.begin(), layer->collision_views.end(), &other);
				if (it != layer->collision_views.end())
					*it = this;
			}
		}
		return *this;
	}

	void CollisionView::draw() const
	{
		static const auto draw_object = [](auto&& obj) { std::visit([](auto&& obj) { obj.draw(); }, obj); };
		std::visit([](auto&& view) {
			if constexpr (std::is_same_v<std::decay_t<decltype(view)>, Object>)
				draw_object(view);
			else if constexpr (std::is_same_v<std::decay_t<decltype(view)>, std::vector<Object>>)
				for (const auto& obj : view)
					draw_object(obj);
			}, obj);
	}

	void CollisionView::clear_view()
	{
		obj = Empty{};
		for (CollisionLayer* layer : layers)
			layer->dirty_views = true;
	}

	void CollisionView::set_view(ObjectView&& obj)
	{
		this->obj = std::move(obj);
		for (CollisionLayer* layer : layers)
			layer->dirty_views = true;
	}

	void CollisionView::merge(CollisionView&& other)
	{
		if (other.obj.index() == EMPTY)
			return;

		if (obj.index() == SINGLE)
		{
			Object old_view = std::move(std::get<SINGLE>(obj));
			obj = std::vector<Object>();
			std::get<VECTOR>(obj).push_back(std::move(old_view));
		}

		std::visit([&view = std::get<VECTOR>(obj)](auto&& other_view) {
			if constexpr (std::is_same_v<std::decay_t<decltype(other_view)>, Object>)
				view.push_back(std::move(other_view));
			else if constexpr (std::is_same_v<std::decay_t<decltype(other_view)>, std::vector<Object>>)
				view.insert(view.end(), std::make_move_iterator(other_view.begin()), std::make_move_iterator(other_view.end()));
			}, std::move(other.obj));

		for (CollisionLayer* layer : layers)
			layer->dirty_views = true;
	}

	void CollisionView::assign(CollisionLayer& layer)
	{
		if (std::find(layers.begin(), layers.end(), &layer) == layers.end())
		{
			layers.push_back(&layer);
			layer.collision_views.push_back(this);
			layer.dirty_views = true;
		}
	}

	void CollisionView::unassign(CollisionLayer& layer)
	{
		auto it = std::find(layers.begin(), layers.end(), &layer);
		if (it != layers.end())
		{
			layers.erase(it);
			layer.collision_views.erase(std::find(layer.collision_views.begin(), layer.collision_views.end(), this));
			layer.dirty_views = true;
		}
	}

	CollisionLayer::CollisionLayer()
	{
		texture = std::make_shared<graphics::BindlessTexture>(GL_TEXTURE_2D);

		// TODO change
		texture_dimensions = { .w = context::get_platform().window().get_width(), .h = context::get_platform().window().get_height(), .cpp = 4 };

		glBindTexture(GL_TEXTURE_2D, *texture);
		graphics::pixel_alignment_pre_send(texture_dimensions.cpp);
		glTexImage2D(GL_TEXTURE_2D, 0, graphics::texture_internal_format(texture_dimensions.cpp), texture_dimensions.w, texture_dimensions.h, 0, graphics::texture_format(texture_dimensions.cpp), GL_UNSIGNED_BYTE, nullptr);
		graphics::pixel_alignment_post_send(texture_dimensions.cpp);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		texture->set_and_use_handle();

		framebuffer.bind();
		framebuffer.attach_2d_texture(graphics::Framebuffer::ColorAttachment::color(0), *texture);
		OLY_ASSERT(framebuffer.status() == graphics::Framebuffer::Status::COMPLETE);
		graphics::Framebuffer::unbind();

		sprite.set_texture(texture, texture_dimensions.dimensions());
	}

	CollisionLayer::CollisionLayer(CollisionLayer&& other) noexcept
		: sprite(std::move(other.sprite)), framebuffer(std::move(other.framebuffer)), texture(std::move(other.texture)),
		texture_dimensions(std::move(other.texture_dimensions)), collision_views(std::move(other.collision_views)), dirty_views(other.dirty_views)
	{
		for (CollisionView* view : collision_views)
		{
			auto it = std::find(view->layers.begin(), view->layers.end(), &other);
			if (it != view->layers.end())
				*it = this;
		}
	}

	CollisionLayer::~CollisionLayer()
	{
		// TODO use std::unordered_set instead of vector? For both collision_views and layers
		for (CollisionView* view : collision_views)
			view->layers.erase(std::find(view->layers.begin(), view->layers.end(), this));
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
			texture_dimensions = std::move(other.texture_dimensions);
			collision_views = std::move(other.collision_views);
			dirty_views = other.dirty_views;
			
			for (CollisionView* view : collision_views)
			{
				auto it = std::find(view->layers.begin(), view->layers.end(), &other);
				if (it != view->layers.end())
					*it = this;
			}
		}
		return *this;
	}

	void CollisionLayer::write_texture() const
	{
		bool was_blending = context::blend_enabled();
		glm::vec4 clear_color = context::clear_color();
		framebuffer.bind();
		glViewport(0, 0, texture_dimensions.w, texture_dimensions.h);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		if (was_blending)
			glDisable(GL_BLEND);

		for (const auto& collision_view : collision_views)
			collision_view->draw();
		context::render_polygons();
		context::render_ellipses();

		framebuffer.unbind();
		// TODO put this viewport in context function
		glViewport(0, 0, context::get_platform().window().get_width(), context::get_platform().window().get_height());
		glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
		glClear(GL_COLOR_BUFFER_BIT);
		if (was_blending)
			glEnable(GL_BLEND);
	}

	void CollisionLayer::draw() const
	{
		if (dirty_views)
		{
			dirty_views = false;
			write_texture();
		}
		sprite.draw();
	}

	void CollisionLayer::assign(CollisionView& view)
	{
		if (std::find(collision_views.begin(), collision_views.end(), &view) == collision_views.end())
		{
			collision_views.push_back(&view);
			view.layers.push_back(this);
			dirty_views = true;
		}
	}

	void CollisionLayer::unassign(CollisionView& view)
	{
		auto it = std::find(collision_views.begin(), collision_views.end(), &view);
		if (it != collision_views.end())
		{
			collision_views.erase(it);
			view.layers.erase(std::find(view.layers.begin(), view.layers.end(), this));
			dirty_views = true;
		}
	}

	void render_layers()
	{
		context::render_sprites();
	}
}
