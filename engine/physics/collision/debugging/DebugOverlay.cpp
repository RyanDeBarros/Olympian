#include "DebugOverlay.h"

namespace oly::debug
{
	DebugShape::DebugShape(Variant&& shape)
		: shape(std::move(shape))
	{
	}
	
	DebugShape::DebugShape(Variant&& shape, DebugOverlayLayer& layer)
		: shape(std::move(shape))
	{
		align_layer_batch(layer);
	}

	void DebugShape::draw() const
	{
		shape.visit([](const auto& s) { s.draw(); });
	}

	math::Rect2D DebugShape::bounds() const
	{
		return shape.visit(
			[](const rendering::StaticEllipse& s) {
				return s.bounds();
			},
			[](const rendering::StaticPolygon& s) {
				const auto& points = s.get_points();
				return col2d::AABB::wrap(points.data(), points.size()).rect();
			},
			[](const rendering::StaticArrowExtension& s) {
				const auto points = s.get_all_points();
				return col2d::AABB::wrap(points.data(), points.size()).rect();
			}
		);
	}

	math::RotatedRect2D DebugShape::rotated_bounds() const
	{
		return shape.visit(
			[](const rendering::StaticEllipse& s) {
				return s.rotated_bounds();
			},
			[](const rendering::StaticPolygon& s) {
				const auto& points = s.get_points();
				return col2d::OBB::fast_wrap(points.data(), points.size()).rect();
			},
			[](const rendering::StaticArrowExtension& s) {
				const auto points = s.get_all_points();
				return col2d::OBB::fast_wrap(points.data(), points.size()).rect();
			}
		);
	}

	void DebugShape::align_layer_batch(DebugOverlayLayer& layer)
	{
		shape.visit(
			[&layer](rendering::StaticEllipse& v) { v.set_batch(layer.painter.get_ellipse_batch()); },
			[&layer](rendering::StaticPolygon& v) { v.set_batch(layer.painter.get_polygon_batch()); },
			[&layer](rendering::StaticArrowExtension& v) { v.set_batch(layer.painter.get_polygon_batch()); }
		);
	}

	void DebugShapeGroup::align_layer_batch(DebugOverlayLayer& layer)
	{
		for (DebugShape& shape : shapes)
			shape.align_layer_batch(layer);
	}

	void DebugShapeGroup::merge(DebugShapeGroup&& other)
	{
		if (other.shapes.empty())
			return;

		if (shapes.empty())
		{
			shapes = std::move(other.shapes);
			return;
		}

		shapes.insert(shapes.end(), std::make_move_iterator(other.shapes.begin()), std::make_move_iterator(other.shapes.end()));
	}

	DebugOverlay::DebugOverlay(DebugOverlayLayer& layer, PaintOptions paint_options)
		: layer(&layer), sprite(rendering::UNBATCHED), paint_options(paint_options)
	{
		layer.assign(this);
	}

	DebugOverlay::DebugOverlay(DebugOverlayLayer& layer, rendering::StaticEllipse&& shape, PaintOptions paint_options)
		: layer(&layer), debug_group(DebugShape(std::move(shape), layer)), sprite(rendering::UNBATCHED), paint_options(paint_options)
	{
		layer.assign(this);
	}

	DebugOverlay::DebugOverlay(DebugOverlayLayer& layer, rendering::StaticPolygon&& shape, PaintOptions paint_options)
		: layer(&layer), debug_group(DebugShape(std::move(shape), layer)), sprite(rendering::UNBATCHED), paint_options(paint_options)
	{
		layer.assign(this);
	}

	DebugOverlay::DebugOverlay(DebugOverlayLayer& layer, rendering::StaticArrowExtension&& shape, PaintOptions paint_options)
		: layer(&layer), debug_group(DebugShape(std::move(shape), layer)), sprite(rendering::UNBATCHED), paint_options(paint_options)
	{
		layer.assign(this);
	}

	DebugOverlay::DebugOverlay(DebugOverlayLayer& layer, DebugShapeGroup&& group, PaintOptions paint_options)
		: layer(&layer), debug_group(std::move(group)), sprite(rendering::UNBATCHED), paint_options(paint_options)
	{
		debug_group.align_layer_batch(layer);
		layer.assign(this);
	}

	DebugOverlay::DebugOverlay(const DebugOverlay& other)
		: layer(other.layer), debug_group(other.debug_group), sprite(other.sprite), dirty(other.dirty)
	{
		if (layer)
			layer->assign(this);
	}

	DebugOverlay::DebugOverlay(DebugOverlay&& other) noexcept
		: layer(other.layer), debug_group(std::move(other.debug_group)), sprite(std::move(other.sprite)), dirty(other.dirty)
	{
		if (layer)
		{
			layer->collision_views.erase(&other);
			layer->collision_views.insert(this);
		}
	}

	DebugOverlay::~DebugOverlay()
	{
		if (layer)
			layer->unassign(this);
	}

	DebugOverlay& DebugOverlay::operator=(const DebugOverlay& other)
	{
		if (this != &other)
		{
			debug_group = other.debug_group;
			shapes_modified();
		}
		return *this;
	}

	DebugOverlay& DebugOverlay::operator=(DebugOverlay&& other) noexcept
	{
		if (this != &other)
		{
			debug_group = std::move(other.debug_group);
			shapes_modified();
		}
		return *this;
	}

	const DebugOverlayLayer& DebugOverlay::get_layer() const
	{
		if (layer)
			return *layer;
		else
			throw Error(ErrorCode::NullPointer);
	}

	DebugOverlayLayer& DebugOverlay::get_layer()
	{
		if (layer)
			return *layer;
		else
			throw Error(ErrorCode::NullPointer);
	}

	void DebugOverlay::set_layer(DebugOverlayLayer& layer)
	{
		if (this->layer)
		{
			if (this->layer == &layer)
				return;

			this->layer->unassign(this);
		}
		this->layer = &layer;
		this->layer->assign(this);

		debug_group.align_layer_batch(layer);
	}

	void DebugOverlay::draw_sprite() const
	{
		if (dirty)
			repaint();

		sprite.draw();
	}

	glm::vec2 DebugOverlay::PaintOptions::compress(math::Rect2D& bounds) const
	{
		glm::vec2 scale = glm::vec2(1.0f);

		if (quality < 1.0f)
		{
			if (bounds.width() > preferred_min_size.x)
			{
				float w = std::lerp(preferred_min_size.x, bounds.width(), quality.get());
				scale.x = bounds.width() / w;
				float offset = 0.5f * (bounds.width() - w);
				bounds.x1 += offset;
				bounds.x2 -= offset;
			}

			if (bounds.height() > preferred_min_size.y)
			{
				float h = std::lerp(preferred_min_size.y, bounds.height(), quality.get());
				scale.y = bounds.height() / h;
				float offset = 0.5f * (bounds.height() - h);
				bounds.y1 += offset;
				bounds.y2 -= offset;
			}
		}

		return scale;
	}

	// TODO v9 repaint in separate thread?
	void DebugOverlay::repaint() const
	{
		auto sprite_batch = sprite.get_batch();
		if (!sprite_batch)
		{
			_OLY_ENGINE_LOG_WARNING("RENDERING") << "Cannot write texture to using null sprite batch" << LOG.nl;
			return;
		}

		dirty = false;
		
		if (debug_group.shapes.empty())
		{
			sprite.set_texture(REF_NULL);
			return;
		}

		math::Rect2D bounds;
		float rotation = 0.0f;

		if (paint_options.bounds_use_rotation)
		{
			math::RotatedRect2D obb;
			if (debug_group.shapes.size() == 1)
				obb = debug_group.shapes[0].rotated_bounds();
			else
			{
				std::vector<glm::vec2> points;
				for (size_t i = 0; i < debug_group.shapes.size(); ++i)
				{
					auto pts = debug_group.shapes[i].rotated_bounds().points();
					points.insert(points.end(), pts.begin(), pts.end());
				}
				obb = col2d::OBB::fast_wrap(points.data(), points.size()).rect();
			}
				
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
			bounds = debug_group.shapes[0].bounds();
			for (size_t i = 1; i < debug_group.shapes.size(); ++i)
				bounds.include_rect(debug_group.shapes[i].bounds());
		}

		if (bounds.width() <= 0 || bounds.height() <= 0)
		{
			sprite.set_texture(REF_NULL);
			return;
		}

		glm::vec2 scale = paint_options.compress(bounds);
		auto paint_context = layer->painter.paint_context(sprite_batch->camera, math::IRect2D::round_out(bounds), rotation, scale);
		
		for (const auto& shape : debug_group.shapes)
			shape.draw();
		
		paint_context.render();
		paint_context.set_texture(sprite);
		sprite.set_transform(Transform2D{ .position = bounds.center(), .rotation = rotation, .scale = scale }.matrix());
	}

	void DebugOverlay::clear_shape_group()
	{
		debug_group.shapes.clear();
		shapes_modified();
	}

	void DebugOverlay::set_shape_group(DebugShapeGroup&& group)
	{
		debug_group = std::move(group);
		shapes_modified();
	}

	void DebugOverlay::set_shape_group(rendering::StaticEllipse&& shape)
	{
		if (layer)
			set_shape_group(DebugShapeGroup(DebugShape(std::move(shape), *layer)));
	}

	void DebugOverlay::set_shape_group(rendering::StaticPolygon&& shape)
	{
		if (layer)
			set_shape_group(DebugShapeGroup(DebugShape(std::move(shape), *layer)));
	}

	void DebugOverlay::set_shape_group(rendering::StaticArrowExtension&& shape)
	{
		if (layer)
			set_shape_group(DebugShapeGroup(DebugShape(std::move(shape), *layer)));
	}

	void DebugOverlay::resize_shape_group(size_t size)
	{
		debug_group.shapes.resize(size, layer->default_debug_shape());
		shapes_modified();
	}

	const DebugShape& DebugOverlay::operator[](size_t i) const
	{
		return debug_group.shapes[i];
	}

	DebugShape& DebugOverlay::operator[](size_t i)
	{
		shapes_modified();
		return debug_group.shapes[i];
	}

	void DebugOverlay::shapes_modified() const
	{
		dirty = true;
	}

	static bool update_view_color(DebugShape& obj, glm::vec4 color)
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

	void DebugOverlay::update_color(glm::vec4 color)
	{
		bool updated = false;
		for (DebugShape& shape : debug_group.shapes)
			updated |= update_view_color(shape, color);

		if (updated)
			shapes_modified();
	}

	void DebugOverlay::update_color(glm::vec4 color, size_t shape_index)
	{
		if (update_view_color(debug_group.shapes[shape_index], color))
			shapes_modified();
	}

	DebugOverlayLayer::DebugOverlayLayer(const DebugOverlayLayer& other)
		: painter(other.painter)
	{
	}
	
	DebugOverlayLayer::DebugOverlayLayer(DebugOverlayLayer&& other) noexcept
		: painter(std::move(other.painter)), collision_views(std::move(other.collision_views))
	{
		for (DebugOverlay* view : collision_views)
		{
			view->layer = this;
			view->sprite.set_batch(sprite_batch);
		}
	}

	DebugOverlayLayer::~DebugOverlayLayer()
	{
		for (DebugOverlay* view : collision_views)
			view->invalidate_layer();
	}

	DebugOverlayLayer& DebugOverlayLayer::operator=(const DebugOverlayLayer& other)
	{
		if (this != &other)
		{
			for (DebugOverlay* view : collision_views)
				view->invalidate_layer();
			collision_views.clear();

			painter = other.painter;
		}
		return *this;
	}

	DebugOverlayLayer& DebugOverlayLayer::operator=(DebugOverlayLayer&& other) noexcept
	{
		if (this != &other)
		{
			for (DebugOverlay* view : collision_views)
				view->invalidate_layer();
			collision_views.clear();

			painter = std::move(other.painter);
			collision_views = std::move(other.collision_views);

			for (DebugOverlay* view : collision_views)
			{
				view->layer = this;
				view->sprite.set_batch(sprite_batch);
			}
		}
		return *this;
	}

	void DebugOverlayLayer::assign(DebugOverlay* view)
	{
		if (collision_views.insert(view).second)
			view->sprite.set_batch(sprite_batch);
	}

	void DebugOverlayLayer::unassign(DebugOverlay* view)
	{
		if (collision_views.erase(view))
			view->sprite.set_batch(rendering::UNBATCHED);
	}

	void DebugOverlayLayer::draw() const
	{
		for (DebugOverlay* view : collision_views)
			view->draw_sprite();
		sprite_batch->render();
	}

	DebugShape DebugOverlayLayer::default_debug_shape()
	{
		return DebugShape(rendering::StaticEllipse(painter.get_ellipse_batch()), *this);
	}

	rendering::StaticEllipse DebugOverlayLayer::create_ellipse()
	{
		return rendering::StaticEllipse(painter.get_ellipse_batch());
	}

	rendering::StaticPolygon DebugOverlayLayer::create_polygon()
	{
		return rendering::StaticPolygon(painter.get_polygon_batch());
	}
	
	rendering::StaticArrowExtension DebugOverlayLayer::create_arrow()
	{
		return rendering::StaticArrowExtension(painter.get_polygon_batch());
	}
}
