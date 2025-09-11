#pragma once

#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"
#include "physics/collision/objects/BVH.h"
#include "physics/collision/methods/SpecialCasting.h"

#include "graphics/primitives/Sprites.h"
#include "graphics/primitives/Ellipses.h"
#include "graphics/primitives/Polygons.h"
#include "graphics/extensions/Arrow.h"
#include "graphics/backend/basic/Framebuffers.h"

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
		using Variant = std::variant<rendering::EllipseBatch::EllipseReference, rendering::StaticPolygon, rendering::StaticArrowExtension>;

		CollisionLayer& layer;
		std::unique_ptr<Variant> v;
		CollisionObject(CollisionLayer& layer, Variant&& v = {});
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

		CollisionObjectView(Variant&& view) : view(view) {}
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
		CollisionView(CollisionLayer& layer, rendering::EllipseBatch::EllipseReference&& obj);
		CollisionView(CollisionLayer& layer, rendering::StaticPolygon&& obj);
		CollisionView(CollisionLayer& layer, rendering::StaticArrowExtension&& obj);
		CollisionView(const CollisionView&);
		CollisionView(CollisionView&&) noexcept;
		~CollisionView();
		CollisionView& operator=(const CollisionView&);
		CollisionView& operator=(CollisionView&&) noexcept;

		void init_on_layer(CollisionLayer& layer);

	private:
		void draw() const;

	public:
		void clear_view();
		void set_view(CollisionObjectView&& obj);
		void set_view(rendering::EllipseBatch::EllipseReference&& obj);
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

		rendering::PolygonBatch polygon_batch;
		rendering::EllipseBatch ellipse_batch;
		rendering::StaticSprite sprite;
		graphics::Framebuffer framebuffer;
		graphics::BindlessTextureRef texture;
		glm::ivec2 dimensions;
		mutable bool dirty_views = false;
		std::unordered_set<CollisionView*> collision_views;

		struct WindowResizeHandler : public EventHandler<input::WindowResizeEventData>
		{
			CollisionLayer* layer = nullptr;

			WindowResizeHandler(CollisionLayer* layer);

			bool consume(const input::WindowResizeEventData& data) override;

			void set_projection();
		} window_resize_handler;
		friend struct WindowResizeHandler;

	public:
		CollisionLayer();
		CollisionLayer(const CollisionLayer&);
		CollisionLayer(CollisionLayer&&) noexcept;
		~CollisionLayer();
		CollisionLayer& operator=(const CollisionLayer&);
		CollisionLayer& operator=(CollisionLayer&&) noexcept;

	private:
		void write_texture() const;
		void set_sprite_scale(glm::vec2 scale);

		void assign(CollisionView* view);
		void unassign(CollisionView* view);

	public:
		void draw(BatchBarrier barrier = batch::BARRIER) const;
		void regen_to_current_resolution();

	private:
		void setup_texture();
	};
	
	extern void render_layers();
}
