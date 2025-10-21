#include "CollisionView.h"

namespace oly::debug
{
	static std::unique_ptr<CollisionObject::Variant> make_collision_object(const CollisionObject::Variant& from, CollisionLayer& layer)
	{
		return from.visit(
			[&layer](const rendering::EllipseReference&) { return std::make_unique<CollisionObject::Variant>(layer.create_ellipse()); },
			[&layer](const rendering::StaticPolygon&) { return std::make_unique<CollisionObject::Variant>(layer.create_polygon()); },
			[&layer](const rendering::StaticArrowExtension&) { return std::make_unique<CollisionObject::Variant>(layer.create_arrow()); }
		);
	}

	CollisionObject::CollisionObject(CollisionLayer& layer, Variant&& v)
		: layer(layer)
	{
		this->v = make_collision_object(v, layer);
		*this->v = std::move(v);
	}

	CollisionObject::CollisionObject(const CollisionObject& other)
		: layer(other.layer), v(std::make_unique<Variant>(*other.v))
	{
	}

	CollisionObject::CollisionObject(CollisionObject&& other) noexcept
		: layer(other.layer), v(std::move(other.v))
	{
	}

	CollisionObject& CollisionObject::operator=(const CollisionObject& other)
	{
		if (this != &other)
		{
			if (v->get_type() != other.v->get_type() && &layer != &other.layer)
				v = make_collision_object(*other.v, layer);
			*v = *other.v;
		}
		return *this;
	}

	CollisionObject& CollisionObject::operator=(CollisionObject&& other) noexcept
	{
		if (this != &other)
		{
			if (&layer == &other.layer)
				v = std::move(other.v);
			else
			{
				if (v->get_type() != other.v->get_type())
					v = make_collision_object(*other.v, layer);
				*v = std::move(*other.v);
			}
		}
		return *this;
	}

	void CollisionObject::draw(rendering::GeometryPainter::PaintSupport& ps) const
	{
		v->visit(
			[&ps](const rendering::EllipseReference& v) { ps.pre_ellipse_draw(); v.draw(); },
			[&ps](const rendering::StaticPolygon& v) { ps.pre_polygon_draw(); v.draw(); },
			[&ps](const rendering::StaticArrowExtension& v) { ps.pre_polygon_draw(); v.draw(); }
		);
	}

	CollisionView::CollisionView(CollisionLayer& layer)
		: layer(&layer), obj(std::make_unique<CollisionObjectView>(EmptyCollision{}))
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::EllipseReference&& obj)
		: layer(&layer), obj(std::make_unique<CollisionObjectView>(CollisionObject(layer, std::move(obj))))
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::StaticPolygon&& obj)
		: layer(&layer), obj(std::make_unique<CollisionObjectView>(CollisionObject(layer, std::move(obj))))
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::StaticArrowExtension&& obj)
		: layer(&layer), obj(std::make_unique<CollisionObjectView>(CollisionObject(layer, std::move(obj))))
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(const CollisionView& other)
		: layer(other.layer), obj(other.valid() ? std::make_unique<CollisionObjectView>(*other.obj) : nullptr)
	{
		if (valid())
			layer->assign(this);
	}

	CollisionView::CollisionView(CollisionView&& other) noexcept
		: layer(other.layer), obj(std::move(other.obj))
	{
		if (valid())
		{
			layer->collision_views.erase(&other);
			layer->collision_views.insert(this);
		}
	}

	CollisionView::~CollisionView()
	{
		if (valid())
			layer->unassign(this);
	}

	CollisionView& CollisionView::operator=(const CollisionView& other)
	{
		if (this != &other)
		{
			obj = other.valid() ? std::make_unique<CollisionObjectView>(*other.obj) : nullptr;
			view_changed();
		}
		return *this;
	}

	CollisionView& CollisionView::operator=(CollisionView&& other) noexcept
	{
		if (this != &other)
		{
			obj = std::move(other.obj);
			view_changed();
		}
		return *this;
	}

	void CollisionView::init_on_layer(CollisionLayer& layer)
	{
		if (valid())
		{
			if (this->layer == &layer)
				return;

			this->layer->unassign(this);
			this->layer = &layer;
			this->layer->assign(this);
			if (auto view = obj->view.safe_get<CollisionObject>())
			{
				CollisionObject old_obj = std::move(*view);
				obj = std::make_unique<CollisionObjectView>(CollisionObject(layer, std::move(*old_obj.v)));
			}
			else if (auto view = obj->view.safe_get<CollisionObjectGroup>())
			{
				std::vector<CollisionObject> old_objs = std::move(*view);
				std::vector<CollisionObject> new_objs = CollisionObjectGroup(old_objs.size(), layer.default_collision_object());
				for (size_t i = 0; i < new_objs.size(); ++i)
					new_objs[i] = std::move(old_objs[i]);
				obj = std::make_unique<CollisionObjectView>(std::move(new_objs));
			}
		}
		else
		{
			this->layer = &layer;
			this->layer->assign(this);
			obj = std::make_unique<CollisionObjectView>(EmptyCollision{});
		}
	}

	const CollisionLayer& CollisionView::get_layer() const
	{
		if (valid())
			return *layer;
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	CollisionLayer& CollisionView::get_layer()
	{
		if (valid())
			return *layer;
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void CollisionView::draw(rendering::GeometryPainter::PaintSupport& ps) const
	{
		// TODO v5 specialized shader for collision view, especially since it's rendered on a separate framebuffer.
		if (valid())
		{
			obj->view.visit(
				[](const EmptyCollision) {},
				[&ps](const CollisionObject& view) { view.draw(ps); },
				[&ps](const CollisionObjectGroup& views) { for (const auto& view : views) view.draw(ps); }
				);
		}
	}

	void CollisionView::clear_view()
	{
		if (valid())
		{
			obj->view = EmptyCollision{};
			view_changed();
		}
	}

	void CollisionView::set_view(CollisionObjectView&& obj)
	{
		if (valid())
		{
			*this->obj = std::move(obj);
			view_changed();
		}
	}

	void CollisionView::set_view(rendering::EllipseReference&& obj)
	{
		if (valid())
			set_view(CollisionObjectView(CollisionObject(*layer, std::move(obj))));
	}

	void CollisionView::set_view(rendering::StaticPolygon&& obj)
	{
		if (valid())
			set_view(CollisionObjectView(CollisionObject(*layer, std::move(obj))));
	}

	void CollisionView::set_view(rendering::StaticArrowExtension&& obj)
	{
		if (valid())
			set_view(CollisionObjectView(CollisionObject(*layer, std::move(obj))));
	}

	size_t CollisionView::view_size() const
	{
		if (valid())
		{
			return obj->view.visit(
				[](const EmptyCollision) { return size_t(0); },
				[](const CollisionObject&) { return size_t(1); },
				[](const CollisionObjectGroup& group) { return group.size(); }
			);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void CollisionView::resize_view(size_t size)
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		if (size == 0)
			clear_view();
		else if (obj->view.holds<EmptyCollision>())
		{
			if (size == 1)
			{
				obj->view = layer->default_collision_object();
				view_changed();
			}
			else if (size > 1)
			{
				obj->view = CollisionObjectGroup(size, layer->default_collision_object());
				view_changed();
			}
		}
		else if (auto view = obj->view.safe_get<CollisionObject>())
		{
			if (size > 1)
			{
				CollisionObject old_obj = *view;
				CollisionObjectGroup group(size, layer->default_collision_object());
				group[0] = std::move(old_obj);
				obj->view = std::move(group);
				view_changed();
			}
		}
		else if (auto view = obj->view.safe_get<CollisionObjectGroup>())
		{
			if (size == 1)
			{
				CollisionObject old_obj = std::move((*view)[0]);
				obj->view = std::move(old_obj);
				view_changed();
			}
			else
			{
				view->resize(size, layer->default_collision_object());
				view_changed();
			}
		}
	}

	void CollisionView::set_view(size_t i, CollisionObject&& obj)
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		if (auto group = this->obj->view.safe_get<CollisionObjectGroup>())
		{
			(*group)[i] = std::move(obj);
			view_changed();
		}
		else
		{
			if (i == 0)
			{
				this->obj->view = std::move(obj);
				view_changed();
			}
			else
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}
	}

	void CollisionView::merge(CollisionView&& other)
	{
		if (!valid() || !other.valid())
			return;

		if (other.obj->view.holds<EmptyCollision>())
			return;

		if (obj->view.holds<EmptyCollision>())
		{
			set_view(std::move(*other.obj));
			return;
		}

		if (auto view = obj->view.safe_get<CollisionObject>())
			obj->view = CollisionObjectGroup{ std::move(*view) };

		other.obj->view.visit(
			[](const EmptyCollision) {},
			[&view = obj->view.get<CollisionObjectGroup>()](CollisionObject& other_view) { view.push_back(std::move(other_view)); },
			[&view = obj->view.get<CollisionObjectGroup>()](CollisionObjectGroup& other_view) { view.insert(view.end(), std::make_move_iterator(other_view.begin()), std::make_move_iterator(other_view.end())); }
		);

		view_changed();
	}

	const CollisionObject& CollisionView::get_view(size_t i) const
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		return obj->view.visit(
			[](const EmptyCollision) -> const CollisionObject& { throw Error(ErrorCode::INDEX_OUT_OF_RANGE); },
			[i](const CollisionObject& view) -> const CollisionObject& { if (i == 0) return view; else throw Error(ErrorCode::INDEX_OUT_OF_RANGE); },
			[i](const CollisionObjectGroup& view) -> const CollisionObject& { return view[i]; }
		);
	}

	CollisionObject& CollisionView::get_view(size_t i)
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		return obj->view.visit(
			[](const EmptyCollision) -> CollisionObject& { throw Error(ErrorCode::INDEX_OUT_OF_RANGE); },
			[i](CollisionObject& view) -> CollisionObject& { if (i == 0) return view; else throw Error(ErrorCode::INDEX_OUT_OF_RANGE); },
			[i](CollisionObjectGroup& view) -> CollisionObject& { return view[i]; }
		);
	}

	void CollisionView::view_changed() const
	{
		if (valid())
			layer->painter.flag_dirty();
	}

	void CollisionView::update_color(glm::vec4 color)
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		static const auto update_color = [](CollisionObject& obj, glm::vec4 color) {
			obj->visit(
				[color](rendering::EllipseReference& obj) { obj.set_color().fill_outer = color; },
				[color](rendering::StaticPolygon& obj) { obj.set_colors() = { color }; },
				[color](rendering::StaticArrowExtension& obj) { obj.set_color(color); }
			);
		};

		bool updated = obj->view.visit(
			[](const EmptyCollision) { return false; },
			[color](CollisionObject& view) { update_color(view, color); return true; },
			[color](CollisionObjectGroup& group)
			{
				for (CollisionObject& obj : group)
					update_color(obj, color);
				return !group.empty();
			}
		);

		if (updated)
			view_changed();
	}

	CollisionLayer::CollisionLayer()
		: painter(paint_fn())
	{
		painter.paint_fn = paint_fn();
	}

	CollisionLayer::CollisionLayer(rendering::Unbatched)
		: painter(paint_fn(), rendering::UNBATCHED)
	{
		painter.paint_fn = paint_fn();
	}

	CollisionLayer::CollisionLayer(rendering::SpriteBatch& batch)
		: painter(paint_fn(), batch)
	{
		painter.paint_fn = paint_fn();
	}

	CollisionLayer::CollisionLayer(const CollisionLayer& other)
		: painter(other.painter)
	{
		painter.paint_fn = paint_fn();
	}

	CollisionLayer::CollisionLayer(CollisionLayer&& other) noexcept
		: painter(std::move(other.painter)), collision_views(std::move(other.collision_views))
	{
		painter.paint_fn = paint_fn();
	}

	CollisionLayer::~CollisionLayer()
	{
		for (CollisionView* view : collision_views)
			view->invalidate_layer();
	}

	CollisionLayer& CollisionLayer::operator=(const CollisionLayer& other)
	{
		if (this != &other)
		{
			for (CollisionView* view : collision_views)
				view->invalidate_layer();
			collision_views.clear();
			painter = other.painter;
			painter.paint_fn = paint_fn();
		}
		return *this;
	}

	CollisionLayer& CollisionLayer::operator=(CollisionLayer&& other) noexcept
	{
		if (this != &other)
		{
			for (CollisionView* view : collision_views)
				view->invalidate_layer();
			collision_views.clear();
			painter = std::move(other.painter);
			painter.paint_fn = paint_fn();
		}
		return *this;
	}

	rendering::GeometryPainter::PaintFunction CollisionLayer::paint_fn() const
	{
		return [this](rendering::GeometryPainter::PaintSupport& ps) {
			for (const auto& collision_view : collision_views)
				collision_view->draw(ps);
			};
	}

	void CollisionLayer::assign(CollisionView* view)
	{
		if (collision_views.insert(view).second)
			painter.flag_dirty();
	}

	void CollisionLayer::unassign(CollisionView* view)
	{
		if (collision_views.erase(view))
			painter.flag_dirty();
	}

	void CollisionLayer::draw() const
	{
		painter.draw();
	}

	CollisionObject CollisionLayer::default_collision_object()
	{
		return CollisionObject(*this, rendering::EllipseReference());
	}

	rendering::EllipseReference CollisionLayer::create_ellipse()
	{
		return rendering::EllipseReference(rendering::internal::get_ellipse_batch(painter));
	}

	rendering::StaticPolygon CollisionLayer::create_polygon()
	{
		return rendering::StaticPolygon(rendering::internal::get_polygon_batch(painter));
	}
	
	rendering::StaticArrowExtension CollisionLayer::create_arrow()
	{
		return rendering::StaticArrowExtension(rendering::internal::get_polygon_batch(painter));
	}
}
