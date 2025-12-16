#include "CollisionView.h"

namespace oly::debug
{
	CollisionObject::CollisionObject(Variant&& v)
		: v(std::move(v))
	{
	}
	
	CollisionObject::CollisionObject(Variant&& v, CollisionLayer& layer)
		: v(std::move(v))
	{
		align_layer_batch(layer);
	}

	void CollisionObject::draw() const
	{
		v.visit([](const auto& v) { v.draw(); });
	}

	math::Rect2D CollisionObject::bounds() const
	{
		return v.visit(
			[](const rendering::StaticEllipse& v) {
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
		return v.visit(
			[](const rendering::StaticEllipse& v) {
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

	void CollisionObject::align_layer_batch(CollisionLayer& layer)
	{
		v.visit(
			[&layer](rendering::StaticEllipse& v) { v.set_batch(layer.painter.get_ellipse_batch()); },
			[&layer](rendering::StaticPolygon& v) { v.set_batch(layer.painter.get_polygon_batch()); },
			[&layer](rendering::StaticArrowExtension& v) { v.set_batch(layer.painter.get_polygon_batch()); }
		);
	}

	void CollisionObjectView::align_layer_batch(CollisionLayer& layer)
	{
		v.visit(
			[](EmptyCollision) {},
			[&layer](CollisionObject& obj) { obj.align_layer_batch(layer); },
			[&layer](CollisionObjectGroup& group) { for (CollisionObject& obj : group) obj.align_layer_batch(layer); }
		);
	}

	void CollisionObjectView::merge(CollisionObjectView&& other)
	{
		if (other->empty() || other->holds<EmptyCollision>())
			return;

		if (v.holds<EmptyCollision>())
		{
			v = std::move(other.v);
			return;
		}

		if (auto view = v.safe_get<CollisionObject>())
			v = CollisionObjectGroup{ std::move(*view) };

		other->visit(
			[](const EmptyCollision) {},
			[&view = v.get<CollisionObjectGroup>()](CollisionObject& other_view) { view.push_back(std::move(other_view)); },
			[&view = v.get<CollisionObjectGroup>()](CollisionObjectGroup& other_view) { view.insert(view.end(), std::make_move_iterator(other_view.begin()), std::make_move_iterator(other_view.end())); }
		);
	}

	CollisionView::CollisionView(CollisionLayer& layer)
		: layer(&layer), obj(EmptyCollision{}), sprite(rendering::UNBATCHED)
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::StaticEllipse&& obj)
		: layer(&layer), obj(CollisionObject(std::move(obj), layer)), sprite(rendering::UNBATCHED)
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::StaticPolygon&& obj)
		: layer(&layer), obj(CollisionObject(std::move(obj), layer)), sprite(rendering::UNBATCHED)
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::StaticArrowExtension&& obj)
		: layer(&layer), obj(CollisionObject(std::move(obj), layer)), sprite(rendering::UNBATCHED)
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, CollisionObjectView&& obj)
		: layer(&layer), obj(std::move(obj)), sprite(rendering::UNBATCHED)
	{
		this->obj.align_layer_batch(layer);
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

	void CollisionView::set_layer(CollisionLayer& layer)
	{
		if (valid())
		{
			if (this->layer == &layer)
				return;

			this->layer->unassign(this);
			this->layer = &layer;
			this->layer->assign(this);
			if (auto view = obj->safe_get<CollisionObject>())
			{
				CollisionObject old_obj = std::move(*view);
				obj.v = CollisionObject(std::move(old_obj.v), layer);
			}
			else if (auto view = obj->safe_get<CollisionObjectGroup>())
			{
				std::vector<CollisionObject> old_objs = std::move(*view);
				std::vector<CollisionObject> new_objs = CollisionObjectGroup(old_objs.size(), layer.default_collision_object());
				for (size_t i = 0; i < new_objs.size(); ++i)
					new_objs[i] = std::move(old_objs[i]);
				obj.v = std::move(new_objs);
			}
		}
		else
		{
			this->layer = &layer;
			this->layer->assign(this);
			obj.v = EmptyCollision{};
		}
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
				math::RotatedRect2D obb = obj->visit(
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
				bounds = obj->visit(
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

				sprite.set_transform(Transform2D{ .position = bounds.center(), .rotation = rotation, .scale = scale }.matrix());

				auto paint_context = layer->painter.paint_context(sprite_batch->camera, math::IRect2D::round_out(bounds), rotation, scale);

				obj->visit(
					[](const EmptyCollision) {},
					[](const CollisionObject& view) { view->visit([](const auto& v) { v.draw(); }); },
					[](const CollisionObjectGroup& views) { for (const auto& view : views) view->visit([](const auto& v) { v.draw(); }); }
				);

				paint_context.render();
				paint_context.set_texture(sprite);
			}
			else
				sprite.set_texture(REF_NULL);
		}
		else
		{
			_OLY_ENGINE_LOG_WARNING("RENDERING") << "Cannot write texture to using null sprite batch" << LOG.nl;
		}
	}

	void CollisionView::clear_view()
	{
		if (valid())
		{
			obj.v = EmptyCollision{};
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

	void CollisionView::set_view(rendering::StaticEllipse&& obj)
	{
		if (valid())
			set_view(CollisionObjectView(CollisionObject(std::move(obj), *layer)));
	}

	void CollisionView::set_view(rendering::StaticPolygon&& obj)
	{
		if (valid())
			set_view(CollisionObjectView(CollisionObject(std::move(obj), *layer)));
	}

	void CollisionView::set_view(rendering::StaticArrowExtension&& obj)
	{
		if (valid())
			set_view(CollisionObjectView(CollisionObject(std::move(obj), *layer)));
	}

	size_t CollisionView::view_size() const
	{
		if (valid())
		{
			return obj->visit(
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
		else if (obj->holds<EmptyCollision>())
		{
			if (size == 1)
			{
				obj.v = layer->default_collision_object();
				view_changed();
			}
			else if (size > 1)
			{
				obj.v = CollisionObjectGroup(size, layer->default_collision_object());
				view_changed();
			}
		}
		else if (auto view = obj->safe_get<CollisionObject>())
		{
			if (size > 1)
			{
				CollisionObject old_obj = *view;
				CollisionObjectGroup group(size, layer->default_collision_object());
				group[0] = std::move(old_obj);
				obj.v = std::move(group);
				view_changed();
			}
		}
		else if (auto view = obj->safe_get<CollisionObjectGroup>())
		{
			if (size == 1)
			{
				CollisionObject old_obj = std::move((*view)[0]);
				obj.v = std::move(old_obj);
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

		if (auto group = this->obj->safe_get<CollisionObjectGroup>())
		{
			(*group)[i] = std::move(obj);
			view_changed();
		}
		else
		{
			if (i == 0)
			{
				this->obj.v = std::move(obj);
				view_changed();
			}
			else
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}
	}

	const CollisionObject& CollisionView::get_view(size_t i) const
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		return obj->visit(
			[](const EmptyCollision) -> const CollisionObject& { throw Error(ErrorCode::INDEX_OUT_OF_RANGE); },
			[i](const CollisionObject& view) -> const CollisionObject& { if (i == 0) return view; else throw Error(ErrorCode::INDEX_OUT_OF_RANGE); },
			[i](const CollisionObjectGroup& view) -> const CollisionObject& { return view[i]; }
		);
	}

	CollisionObject& CollisionView::get_view(size_t i)
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		return obj->visit(
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

	static bool update_view_color(CollisionObject& obj, glm::vec4 color)
	{
		return obj->visit(
			[color](rendering::StaticEllipse& obj) {
				if (!obj.get_color().is_uniform() || obj.get_color().fill_inner != color)
				{
					obj.set_color(color);
					return true;
				}
				else
					return false;
			},
			[color](rendering::StaticPolygon& obj) {
				const auto& colors = obj.get_colors();
				if (!(colors.size() == 1 && colors[0] == color))
				{
					obj.set_colors() = { color };
					return true;
				}
				else
					return false;
			},
			[color](rendering::StaticArrowExtension& obj) {
				if (obj.get_start_color() != obj.get_end_color() || obj.set_start_color() != color)
				{
					obj.set_color(color);
					return true;
				}
				else
					return false;
			}
		);
	}

	void CollisionView::update_color(glm::vec4 color)
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		obj->visit(
			[](const EmptyCollision) {},
			[this, color](CollisionObject& view) {
				if (update_view_color(view, color))
					view_changed();
			},
			[this, color](CollisionObjectGroup& group) {
				bool updated = false;
				for (CollisionObject& obj : group)
					updated |= update_view_color(obj, color);

				if (updated)
					view_changed();
			}
		);
	}

	void CollisionView::update_color(glm::vec4 color, size_t view_index)
	{
		if (update_view_color(get_view(view_index), color))
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
		return CollisionObject(rendering::StaticEllipse(painter.get_ellipse_batch()), *this);
	}

	rendering::StaticEllipse CollisionLayer::create_ellipse()
	{
		return rendering::StaticEllipse(painter.get_ellipse_batch());
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
