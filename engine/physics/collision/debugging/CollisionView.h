#pragma once

#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"
#include "physics/collision/objects/BVH.h"
#include "physics/collision/methods/SpecialCasting.h"

#include "graphics/shapes/GeometryPainter.h"
#include "graphics/shapes/Arrow.h"

#include "core/platform/WindowEvents.h"
#include "core/platform/EventHandler.h"

namespace oly::debug
{
	class CollisionLayer;

	struct CollisionObject
	{
		enum Type
		{
			ELLIPSE,
			POLYGON,
			ARROW
		};
		using Variant = std::variant<rendering::EllipseReference, rendering::StaticPolygon, rendering::StaticArrowExtension>;

		CollisionLayer& layer;
		std::unique_ptr<Variant> v;
		CollisionObject(CollisionLayer& layer, Variant&& v);
		CollisionObject(const CollisionObject&);
		CollisionObject(CollisionObject&&) noexcept;
		CollisionObject& operator=(const CollisionObject&);
		CollisionObject& operator=(CollisionObject&&) noexcept;

	private:
		void init(Type);

	public:
		Variant& operator*() { return *v; }
		const Variant& operator*() const { return *v; }
		Variant* operator->() { return v.get(); }
		const Variant* operator->() const { return v.get(); }
	};

	struct EmptyCollision {};
	using CollisionObjectGroup = std::vector<CollisionObject>;

	struct CollisionObjectView
	{
		enum Type
		{
			EMPTY,
			SINGLE,
			GROUP
		};
		using Variant = std::variant<EmptyCollision, CollisionObject, CollisionObjectGroup>;

		Variant view;

		CollisionObjectView(Variant&& view) : view(std::move(view)) {}
	};

	class CollisionView
	{
		friend class CollisionLayer;

		CollisionLayer* layer;
		std::unique_ptr<CollisionObjectView> obj;

		void invalidate_layer() { layer = nullptr; obj.reset(); }
		bool valid() const { return obj && layer; }

	public:
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
		void draw() const;

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
		std::unordered_set<CollisionView*> collision_views;

	public:
		CollisionLayer();
		CollisionLayer(rendering::SpriteBatch* batch);
		CollisionLayer(const CollisionLayer&);
		CollisionLayer(CollisionLayer&&) noexcept;
		~CollisionLayer();
		CollisionLayer& operator=(const CollisionLayer&);
		CollisionLayer& operator=(CollisionLayer&&) noexcept;

		rendering::SpriteBatch* get_batch() const { return painter.get_sprite_batch(); }
		void set_batch(rendering::SpriteBatch* batch) { painter.set_sprite_batch(batch); }

	private:
		std::function<void()> paint_fn() const;
		
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
