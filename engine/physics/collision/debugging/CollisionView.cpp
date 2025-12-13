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

	void CollisionObject::paint(rendering::GeometryPainter::PaintContext& paint_context) const
	{
		v->visit(
			[&paint_context](const rendering::EllipseReference& v) {
				paint_context.pre_ellipse_draw();
				v.draw();
			},
			[&paint_context](const rendering::StaticPolygon& v) {
				paint_context.pre_polygon_draw();
				v.draw();
			},
			[&paint_context](const rendering::StaticArrowExtension& v) {
				paint_context.pre_polygon_draw();
				v.draw();
			}
		);
	}

	math::Rect2D CollisionObject::bounds() const
	{
		return v->visit(
			[](const rendering::EllipseReference& v) {
				return v.bounds();
			},
			[](const rendering::StaticPolygon& v) {
				const auto& points = v.get_points();
				return col2d::AABB::wrap(points.data(), points.size()).rect();
			},
			[](const rendering::StaticArrowExtension& v) {
				const auto points = v.get_all_points();
				return col2d::AABB::wrap(points.data(), points.size()).rect();
			}
		);
	}

	math::RotatedRect2D CollisionObject::rotated_bounds() const
	{
		return v->visit(
			[](const rendering::EllipseReference& v) {
				return v.rotated_bounds();
			},
			[](const rendering::StaticPolygon& v) {
				const auto& points = v.get_points();
				return col2d::OBB::fast_wrap(points.data(), points.size()).rect();
			},
			[](const rendering::StaticArrowExtension& v) {
				const auto points = v.get_all_points();
				return col2d::OBB::fast_wrap(points.data(), points.size()).rect();
			}
		);
	}

	CollisionView::CollisionView(CollisionLayer& layer)
		: layer(&layer), obj(EmptyCollision{}), sprite(rendering::UNBATCHED)
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::EllipseReference&& obj)
		: layer(&layer), obj(CollisionObject(layer, std::move(obj))), sprite(rendering::UNBATCHED)
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::StaticPolygon&& obj)
		: layer(&layer), obj(CollisionObject(layer, std::move(obj))), sprite(rendering::UNBATCHED)
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::StaticArrowExtension&& obj)
		: layer(&layer), obj(CollisionObject(layer, std::move(obj))), sprite(rendering::UNBATCHED)
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(const CollisionView& other)
		: layer(other.layer), obj(other.obj), sprite(other.sprite), dirty(other.dirty)
	{
		if (valid())
			layer->assign(this);
	}

	CollisionView::CollisionView(CollisionView&& other) noexcept
		: layer(other.layer), obj(std::move(other.obj)), sprite(std::move(other.sprite)), dirty(other.dirty)
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
			obj = other.obj;
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
			if (auto view = obj.safe_get<CollisionObject>())
			{
				CollisionObject old_obj = std::move(*view);
				obj = CollisionObject(layer, std::move(*old_obj.v));
			}
			else if (auto view = obj.safe_get<CollisionObjectGroup>())
			{
				std::vector<CollisionObject> old_objs = std::move(*view);
				std::vector<CollisionObject> new_objs = CollisionObjectGroup(old_objs.size(), layer.default_collision_object());
				for (size_t i = 0; i < new_objs.size(); ++i)
					new_objs[i] = std::move(old_objs[i]);
				obj = std::move(new_objs);
			}
		}
		else
		{
			this->layer = &layer;
			this->layer->assign(this);
			obj = EmptyCollision{};
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

	void CollisionView::draw_sprite() const
	{
		if (!valid())
			return;

		if (dirty)
			repaint();

		sprite.draw();
	}

	// TODO v8 repaint in separate thread?
	void CollisionView::repaint() const
	{
		if (auto sprite_batch = sprite.get_batch())
		{
			dirty = false;

			math::Rect2D bounds;
			float rotation = 0.0f;

			if (paint_options.bounds_use_rotation)
			{
				math::RotatedRect2D obb = obj.visit(
					[](const EmptyCollision) { return math::RotatedRect2D{}; },
					[use_rotation = paint_options.bounds_use_rotation](const CollisionObject& view) { return view.rotated_bounds(); },
					[use_rotation = paint_options.bounds_use_rotation](const CollisionObjectGroup& view) {
						std::vector<glm::vec2> points;
						for (size_t i = 0; i < view.size(); ++i)
						{
							auto pts = view.at(i).rotated_bounds().points();
							points.insert(points.end(), pts.begin(), pts.end());
						}
						return col2d::OBB::fast_wrap(points.data(), points.size()).rect();
					}
				);

				bounds = {
					.x1 = obb.center.x - 0.5f * obb.size.x,
					.x2 = obb.center.x + 0.5f * obb.size.x,
					.y1 = obb.center.y - 0.5f * obb.size.y,
					.y2 = obb.center.y + 0.5f * obb.size.y
				};
				rotation = obb.rotation;
			}
			else
			{
				bounds = obj.visit(
					[](const EmptyCollision) { return math::Rect2D{}; },
					[](const CollisionObject& view) { return view.bounds(); },
					[](const CollisionObjectGroup& view) {
						math::Rect2D bounds = view.at(0).bounds();
						for (size_t i = 1; i < view.size(); ++i)
							bounds.include_rect(view.at(i).bounds());
						return bounds;
					}
				);
			}

			if (bounds.width() > 0 && bounds.height() > 0)
			{
				glm::vec2 scale = glm::vec2(1.0f);
				if (paint_options.quality < 1.0f)
				{
					if (bounds.width() > paint_options.preferred_min_size.x)
					{
						float w = std::lerp(paint_options.preferred_min_size.x, bounds.width(), paint_options.quality.get());
						scale.x = bounds.width() / w;
						float offset = 0.5f * (bounds.width() - w);
						bounds.x1 += offset;
						bounds.x2 -= offset;
					}

					if (bounds.height() > paint_options.preferred_min_size.y)
					{
						float h = std::lerp(paint_options.preferred_min_size.y, bounds.height(), paint_options.quality.get());
						scale.y = bounds.height() / h;
						float offset = 0.5f * (bounds.height() - h);
						bounds.y1 += offset;
						bounds.y2 -= offset;
					}
				}

				auto paint_context = layer->painter.paint_context(sprite_batch->camera, math::IRect2D::round_out(bounds), rotation, scale);
				sprite.set_transform(Transform2D{ .position = bounds.center(), .rotation = rotation, .scale = scale }.matrix());

				obj.visit(
					[](const EmptyCollision) {},
					[&paint_context](const CollisionObject& view) { view.paint(paint_context); },
					[&paint_context](const CollisionObjectGroup& views) { for (const auto& view : views) view.paint(paint_context); }
				);

				paint_context.flush();
				paint_context.set_texture(sprite);
			}
			else
				sprite.set_texture(REF_NULL);
		}
		else
		{
			OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Cannot write texture to using null sprite batch" << LOG.nl;

			// TODO v6 convenience macro for OLY_LOG_*(true, "...") << LOG.source_info.full_source()
		}
	}

	void CollisionView::clear_view()
	{
		if (valid())
		{
			obj = EmptyCollision{};
			view_changed();
		}
	}

	void CollisionView::set_view(CollisionObjectView&& obj)
	{
		if (valid())
		{
			this->obj = std::move(obj);
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
			return obj.visit(
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
		else if (obj.holds<EmptyCollision>())
		{
			if (size == 1)
			{
				obj = layer->default_collision_object();
				view_changed();
			}
			else if (size > 1)
			{
				obj = CollisionObjectGroup(size, layer->default_collision_object());
				view_changed();
			}
		}
		else if (auto view = obj.safe_get<CollisionObject>())
		{
			if (size > 1)
			{
				CollisionObject old_obj = *view;
				CollisionObjectGroup group(size, layer->default_collision_object());
				group[0] = std::move(old_obj);
				obj = std::move(group);
				view_changed();
			}
		}
		else if (auto view = obj.safe_get<CollisionObjectGroup>())
		{
			if (size == 1)
			{
				CollisionObject old_obj = std::move((*view)[0]);
				obj = std::move(old_obj);
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

		if (auto group = this->obj.safe_get<CollisionObjectGroup>())
		{
			(*group)[i] = std::move(obj);
			view_changed();
		}
		else
		{
			if (i == 0)
			{
				this->obj = std::move(obj);
				view_changed();
			}
			else
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}
	}

	// TODO v6 merge() should take an alternative overload where only CollisionObjectView is passed, so no other initialization needs to be done in compound collision_view()s
	void CollisionView::merge(CollisionView&& other)
	{
		if (!valid() || !other.valid())
			return;

		if (other.obj.holds<EmptyCollision>())
			return;

		if (obj.holds<EmptyCollision>())
		{
			set_view(std::move(other.obj));
			return;
		}

		if (auto view = obj.safe_get<CollisionObject>())
			obj = CollisionObjectGroup{ std::move(*view) };

		other.obj.visit(
			[](const EmptyCollision) {},
			[&view = obj.get<CollisionObjectGroup>()](CollisionObject& other_view) { view.push_back(std::move(other_view)); },
			[&view = obj.get<CollisionObjectGroup>()](CollisionObjectGroup& other_view) { view.insert(view.end(), std::make_move_iterator(other_view.begin()), std::make_move_iterator(other_view.end())); }
		);

		view_changed();
	}

	const CollisionObject& CollisionView::get_view(size_t i) const
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		return obj.visit(
			[](const EmptyCollision) -> const CollisionObject& { throw Error(ErrorCode::INDEX_OUT_OF_RANGE); },
			[i](const CollisionObject& view) -> const CollisionObject& { if (i == 0) return view; else throw Error(ErrorCode::INDEX_OUT_OF_RANGE); },
			[i](const CollisionObjectGroup& view) -> const CollisionObject& { return view[i]; }
		);
	}

	CollisionObject& CollisionView::get_view(size_t i)
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		return obj.visit(
			[](const EmptyCollision) -> CollisionObject& { throw Error(ErrorCode::INDEX_OUT_OF_RANGE); },
			[i](CollisionObject& view) -> CollisionObject& { if (i == 0) return view; else throw Error(ErrorCode::INDEX_OUT_OF_RANGE); },
			[i](CollisionObjectGroup& view) -> CollisionObject& { return view[i]; }
		);
	}

	void CollisionView::view_changed() const
	{
		if (valid())
			dirty = true;
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

		bool updated = obj.visit(
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

	CollisionLayer::CollisionLayer(const CollisionLayer& other)
		: painter(other.painter)
	{
	}
	
	CollisionLayer::CollisionLayer(CollisionLayer&& other) noexcept
		: painter(std::move(other.painter)), collision_views(std::move(other.collision_views))
	{
		for (CollisionView* view : collision_views)
		{
			view->layer = this;
			view->sprite.set_batch(sprite_batch);
		}
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
			collision_views = std::move(other.collision_views);

			for (CollisionView* view : collision_views)
			{
				view->layer = this;
				view->sprite.set_batch(sprite_batch);
			}
		}
		return *this;
	}

	void CollisionLayer::assign(CollisionView* view)
	{
		if (collision_views.insert(view).second)
			view->sprite.set_batch(sprite_batch);
	}

	void CollisionLayer::unassign(CollisionView* view)
	{
		if (collision_views.erase(view))
			view->sprite.set_batch(rendering::UNBATCHED);
	}

	void CollisionLayer::draw() const
	{
		for (CollisionView* view : collision_views)
			view->draw_sprite();
		sprite_batch->render();
	}

	CollisionObject CollisionLayer::default_collision_object()
	{
		return CollisionObject(*this, rendering::EllipseReference());
	}

	rendering::EllipseReference CollisionLayer::create_ellipse()
	{
		return rendering::EllipseReference(painter.get_ellipse_batch());
	}

	rendering::StaticPolygon CollisionLayer::create_polygon()
	{
		return rendering::StaticPolygon(painter.get_polygon_batch());
	}
	
	rendering::StaticArrowExtension CollisionLayer::create_arrow()
	{
		return rendering::StaticArrowExtension(painter.get_polygon_batch());
	}
}
