#pragma once

#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"
#include "physics/collision/objects/BVH.h"
#include "physics/collision/methods/SpecialCasting.h"

#include "graphics/shapes/GeometryPainter.h"
#include "graphics/shapes/Arrow.h"

namespace oly::debug
{
	class CollisionLayer;

	struct CollisionObject
	{
		using Variant = Variant<rendering::EllipseReference, rendering::StaticPolygon, rendering::StaticArrowExtension>;

		CollisionLayer& layer;
		std::unique_ptr<Variant> v;
		CollisionObject(CollisionLayer& layer, Variant&& v);
		CollisionObject(const CollisionObject&);
		CollisionObject(CollisionObject&&) noexcept;
		CollisionObject& operator=(const CollisionObject&);
		CollisionObject& operator=(CollisionObject&&) noexcept;

		Variant& operator*() { return *v; }
		const Variant& operator*() const { return *v; }
		Variant* operator->() { return v.get(); }
		const Variant* operator->() const { return v.get(); }

		void paint(rendering::GeometryPainter::PaintContext& paint_context) const;
		math::Rect2D bounds() const;
		math::RotatedRect2D rotated_bounds() const;
	};

	struct EmptyCollision {};
	using CollisionObjectGroup = std::vector<CollisionObject>;
	using CollisionObjectView = Variant<EmptyCollision, CollisionObject, CollisionObjectGroup>;

	class CollisionView
	{
		friend class CollisionLayer;

		CollisionLayer* layer;
		CollisionObjectView obj;
		mutable rendering::StaticSprite sprite;
		mutable bool dirty = true;

		void invalidate_layer() { layer = nullptr; obj = EmptyCollision{}; }
		bool valid() const { return !obj.empty() && layer; }

	public:
		// TODO v6 mkdocs for all collision debugging, especially CollisionView/CollisionLayer and PaintOptions
		// TODO v6 option to pass PaintOptions to collision_view()/update_collision() functions - use better system, since there's so many of them?
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
		} paint_options = {};

		CollisionView(CollisionLayer& layer);
		CollisionView(CollisionLayer& layer, rendering::EllipseReference&& obj);
		CollisionView(CollisionLayer& layer, rendering::StaticPolygon&& obj);
		CollisionView(CollisionLayer& layer, rendering::StaticArrowExtension&& obj);
		CollisionView(const CollisionView&);
		CollisionView(CollisionView&&) noexcept;
		~CollisionView();
		CollisionView& operator=(const CollisionView&);
		CollisionView& operator=(CollisionView&&) noexcept;

		void init_on_layer(CollisionLayer& layer);
		const CollisionLayer& get_layer() const;
		CollisionLayer& get_layer();

	private:
		void draw_sprite() const;
		void repaint() const;

	public:
		void clear_view();
		void set_view(CollisionObjectView&& obj);
		void set_view(rendering::EllipseReference&& obj);
		void set_view(rendering::StaticPolygon&& obj);
		void set_view(rendering::StaticArrowExtension&& obj);
		size_t view_size() const;
		void resize_view(size_t size);
		void set_view(size_t i, CollisionObject&& obj);
		void merge(CollisionView&& other);

		const CollisionObject& get_view(size_t i = 0) const;
		CollisionObject& get_view(size_t i = 0);
		void view_changed() const;

		void update_color(glm::vec4 color);
	};

	class CollisionLayer
	{
		friend struct CollisionObject;
		friend class CollisionView;

		rendering::GeometryPainter painter;
		rendering::SpriteBatch sprite_batch;
		std::unordered_set<CollisionView*> collision_views;

	public:
		CollisionLayer() = default;
		CollisionLayer(const CollisionLayer&);
		CollisionLayer(CollisionLayer&&) noexcept;
		~CollisionLayer();
		CollisionLayer& operator=(const CollisionLayer&);
		CollisionLayer& operator=(CollisionLayer&&) noexcept;

		const rendering::SpriteBatch& get_sprite_batch() const { return sprite_batch; }

	private:
		void assign(CollisionView* view);
		void unassign(CollisionView* view);

	public:
		void draw() const;
		CollisionObject default_collision_object();
		rendering::EllipseReference create_ellipse();
		rendering::StaticPolygon create_polygon();
		rendering::StaticArrowExtension create_arrow();
	};
}
