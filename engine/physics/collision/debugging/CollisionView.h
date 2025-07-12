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

#include "core/platform/Input.h"

namespace oly::debug
{
	enum CollisionObjectType
	{
		ELLIPSE,
		POLYGON,
		ARROW
	};
	using CollisionObject = std::variant<rendering::EllipseBatch::EllipseReference, rendering::StaticPolygon, rendering::StaticArrowExtension>;
	struct EmptyCollision {};
	enum CollisionObjectViewType
	{
		EMPTY,
		SINGLE,
		GROUP
	};
	using CollisionObjectGroup = std::vector<CollisionObject>;
	using CollisionObjectView = std::variant<EmptyCollision, CollisionObject, CollisionObjectGroup>;

	class CollisionLayer;
	class CollisionView
	{
		friend class CollisionLayer;

		CollisionObjectView obj;
		std::unordered_set<CollisionLayer*> layers;

	public:
		CollisionView() : obj(EmptyCollision{}) {}
		CollisionView(rendering::EllipseBatch::EllipseReference&& obj) : obj(std::move(obj)) {}
		CollisionView(rendering::StaticPolygon&& obj) : obj(std::move(obj)) {}
		CollisionView(rendering::StaticArrowExtension&& obj) : obj(std::move(obj)) {}
		CollisionView(const CollisionView&);
		CollisionView(CollisionView&&) noexcept;
		~CollisionView();
		CollisionView& operator=(const CollisionView&);
		CollisionView& operator=(CollisionView&&) noexcept;

	private:
		enum class Batch
		{
			NONE,
			POLYGON,
			ELLIPSE
		};
		void draw(Batch& current_batch) const;

	public:
		void clear_view();
		void set_view(CollisionObjectView&& obj);
		size_t view_size() const;
		void resize_view(size_t size);
		void set_view(size_t i, CollisionObject&& obj);
		void merge(CollisionView&& other);

		void assign(CollisionLayer& layer);
		void unassign(CollisionLayer& layer);

		const CollisionObject& get_view(size_t i = 0) const;
		CollisionObject& get_view(size_t i = 0);
		void view_changed() const;

		void update_color(glm::vec4 color);
	};

	class CollisionLayer
	{
		friend class CollisionView;

		rendering::StaticSprite sprite;
		graphics::Framebuffer framebuffer;
		graphics::BindlessTextureRes texture;
		glm::ivec2 dimensions;
		mutable bool dirty_views = false;
		std::unordered_set<CollisionView*> collision_views;

		struct WindowResizeHandler : public EventHandler<input::WindowResizeEventData>
		{
			CollisionLayer* layer = nullptr;

			WindowResizeHandler();

			bool consume(const input::WindowResizeEventData& data) override;
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

	public:
		void draw() const;

		void assign(CollisionView& view);
		void unassign(CollisionView& view);

		void regen_to_current_resolution();

	private:
		void setup_texture();
	};
	
	extern void render_layers();
}
