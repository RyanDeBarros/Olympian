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
		using Variant = Variant<rendering::StaticEllipse, rendering::StaticPolygon, rendering::StaticArrowExtension>;

		Variant v;

		CollisionObject(Variant&& v);
		CollisionObject(Variant&& v, CollisionLayer& layer);
		
		Variant& operator*() { return v; }
		const Variant& operator*() const { return v; }
		Variant* operator->() { return &v; }
		const Variant* operator->() const { return &v; }

		void draw() const;
		math::Rect2D bounds() const;
		math::RotatedRect2D rotated_bounds() const;

		void align_layer_batch(CollisionLayer& layer);
	};

	struct EmptyCollision {};
	using CollisionObjectGroup = std::vector<CollisionObject>;
	
	// TODO v6 put in internal and rename
	struct CollisionObjectView
	{
		// TODO v6 just use std::vector<CollisionObject> or keep variant? Also, probably rename CollisionObjectView to CollisionObjectList or something
		using Variant = Variant<EmptyCollision, CollisionObject, CollisionObjectGroup>;

		Variant v;

		CollisionObjectView(Variant&& v) : v(std::move(v)) {}
		CollisionObjectView(EmptyCollision v) : v(std::move(v)) {}
		CollisionObjectView(rendering::StaticEllipse&& v) : v(CollisionObject(std::move(v))) {}
		CollisionObjectView(rendering::StaticPolygon&& v) : v(CollisionObject(std::move(v))) {}
		CollisionObjectView(rendering::StaticArrowExtension&& v) : v(CollisionObject(std::move(v))) {}
		CollisionObjectView(CollisionObjectGroup&& v) : v(std::move(v)) {}

		Variant& operator*() { return v; }
		const Variant& operator*() const { return v; }
		Variant* operator->() { return &v; }
		const Variant* operator->() const { return &v; }

		void align_layer_batch(CollisionLayer& layer);
		void merge(CollisionObjectView&& other);
	};

	class CollisionView
	{
		friend class CollisionLayer;

		CollisionLayer* layer;
		CollisionObjectView obj;
		mutable rendering::StaticSprite sprite;
		mutable bool dirty = true;

		void invalidate_layer() { layer = nullptr; *obj = EmptyCollision{}; }
		bool valid() const { return !obj->empty() && layer; }

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
		CollisionView(CollisionLayer& layer, rendering::StaticEllipse&& obj);
		CollisionView(CollisionLayer& layer, rendering::StaticPolygon&& obj);
		CollisionView(CollisionLayer& layer, rendering::StaticArrowExtension&& obj);
		CollisionView(CollisionLayer& layer, CollisionObjectView&& obj);
		CollisionView(const CollisionView&);
		CollisionView(CollisionView&&) noexcept;
		~CollisionView();
		CollisionView& operator=(const CollisionView&);
		CollisionView& operator=(CollisionView&&) noexcept;

		const CollisionLayer& get_layer() const;
		CollisionLayer& get_layer();
		void set_layer(CollisionLayer& layer);

	private:
		void draw_sprite() const;
		void repaint() const;

	public:
		void clear_view();
		void set_view(CollisionObjectView&& obj);
		void set_view(rendering::StaticEllipse&& obj);
		void set_view(rendering::StaticPolygon&& obj);
		void set_view(rendering::StaticArrowExtension&& obj);
		size_t view_size() const;
		void resize_view(size_t size);
		void set_view(size_t i, CollisionObject&& obj);

		const CollisionObject& get_view(size_t i = 0) const;
		CollisionObject& get_view(size_t i = 0);
		void view_changed() const;

		void update_color(glm::vec4 color);
		void update_color(glm::vec4 color, size_t view_index);
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
		rendering::StaticEllipse create_ellipse();
		rendering::StaticPolygon create_polygon();
		rendering::StaticArrowExtension create_arrow();
	};
}
