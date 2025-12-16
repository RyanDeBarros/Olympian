#pragma once

#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"
#include "physics/collision/objects/BVH.h"
#include "physics/collision/methods/SpecialCasting.h"

#include "graphics/shapes/GeometryPainter.h"
#include "graphics/shapes/Arrow.h"

namespace oly::debug
{
	class DebugOverlayLayer;

	struct DebugShape
	{
		using Variant = Variant<rendering::StaticEllipse, rendering::StaticPolygon, rendering::StaticArrowExtension>;

		Variant shape;

		DebugShape(Variant&& shape);
		DebugShape(Variant&& shape, DebugOverlayLayer& layer);
		
		Variant& operator*() { return shape; }
		const Variant& operator*() const { return shape; }
		Variant* operator->() { return &shape; }
		const Variant* operator->() const { return &shape; }

		void draw() const;
		math::Rect2D bounds() const;
		math::RotatedRect2D rotated_bounds() const;

		void align_layer_batch(DebugOverlayLayer& layer);
	};

	struct DebugShapeGroup
	{
		std::vector<DebugShape> shapes;

		DebugShapeGroup() = default;
		DebugShapeGroup(DebugShape&& shape) { shapes.push_back(std::move(shape)); }
		DebugShapeGroup(rendering::StaticEllipse&& shape) { shapes.push_back(DebugShape(std::move(shape))); }
		DebugShapeGroup(rendering::StaticPolygon&& shape) { shapes.push_back(DebugShape(std::move(shape))); }
		DebugShapeGroup(rendering::StaticArrowExtension&& v) { shapes.push_back(DebugShape(std::move(v))); }

		void align_layer_batch(DebugOverlayLayer& layer);
		void merge(DebugShapeGroup&& other);
	};

	class DebugOverlay
	{
		friend class DebugOverlayLayer;

		DebugOverlayLayer* layer;
		DebugShapeGroup debug_group;
		mutable rendering::StaticSprite sprite;
		mutable bool dirty = true;

		void invalidate_layer() { layer = nullptr; debug_group.shapes.clear(); }

	public:
		// TODO v6 mkdocs for all collision debugging, especially DebugOverlay/DebugOverlayLayer and PaintOptions
		// TODO v6 option to pass PaintOptions to collision_view()/update_collision() functions - use better system, since there's so many of them? Also, rename function to reflect the new class names
		struct PaintOptions
		{
			/**
			 * If true, will use OBB of object to paint texture->can be slower but can use smaller texture
			 */
			bool bounds_use_rotation = false;

			/**
			 * Factor by which to reduce the size of texture to the preferred minimum size.
			 * At 0, the texture's size is set to the preferred minimum, while at 1, the texture's size is the full bounds of the object.
			 */
			BoundedUnitInterval quality = 1.0f;
			glm::vec2 preferred_min_size = glm::vec2(50.0f);

			glm::vec2 compress(math::Rect2D& bounds) const;
		} paint_options = {};

		DebugOverlay(DebugOverlayLayer& layer);
		DebugOverlay(DebugOverlayLayer& layer, rendering::StaticEllipse&& shape);
		DebugOverlay(DebugOverlayLayer& layer, rendering::StaticPolygon&& shape);
		DebugOverlay(DebugOverlayLayer& layer, rendering::StaticArrowExtension&& shape);
		DebugOverlay(DebugOverlayLayer& layer, DebugShapeGroup&& group);
		DebugOverlay(const DebugOverlay&);
		DebugOverlay(DebugOverlay&&) noexcept;
		~DebugOverlay();
		DebugOverlay& operator=(const DebugOverlay&);
		DebugOverlay& operator=(DebugOverlay&&) noexcept;

		const DebugOverlayLayer& get_layer() const;
		DebugOverlayLayer& get_layer();
		void set_layer(DebugOverlayLayer& layer);

	private:
		void draw_sprite() const;
		void repaint() const;

	public:
		void clear_shape_group();
		void set_shape_group(DebugShapeGroup&& group);
		void set_shape_group(rendering::StaticEllipse&& shape);
		void set_shape_group(rendering::StaticPolygon&& shape);
		void set_shape_group(rendering::StaticArrowExtension&& shape);
		size_t shape_count() const { return debug_group.shapes.size(); }
		void resize_shape_group(size_t size);

		// TODO v6 use operator[]
		void set_shape(size_t i, DebugShape&& shape);
		const DebugShape& get_shape(size_t i = 0) const { return debug_group.shapes[i]; }
		DebugShape& get_shape(size_t i = 0) { return debug_group.shapes[i]; }

		void shapes_modified() const;

		void update_color(glm::vec4 color);
		void update_color(glm::vec4 color, size_t shape_index);
	};

	class DebugOverlayLayer
	{
		friend struct DebugShape;
		friend class DebugOverlay;

		rendering::GeometryPainter painter;
		rendering::SpriteBatch sprite_batch;
		std::unordered_set<DebugOverlay*> collision_views;

	public:
		DebugOverlayLayer() = default;
		DebugOverlayLayer(const DebugOverlayLayer&);
		DebugOverlayLayer(DebugOverlayLayer&&) noexcept;
		~DebugOverlayLayer();
		DebugOverlayLayer& operator=(const DebugOverlayLayer&);
		DebugOverlayLayer& operator=(DebugOverlayLayer&&) noexcept;

		const rendering::SpriteBatch& get_sprite_batch() const { return sprite_batch; }

	private:
		void assign(DebugOverlay* view);
		void unassign(DebugOverlay* view);

	public:
		void draw() const;

		DebugShape default_debug_shape();
		rendering::StaticEllipse create_ellipse();
		rendering::StaticPolygon create_polygon();
		rendering::StaticArrowExtension create_arrow();
	};
}
