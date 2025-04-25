#pragma once

#include "rendering/core/Core.h"
#include "rendering/TextureRegistry.h"
#include "rendering/mutable/SpriteRegistry.h"
#include "rendering/immutable/PolygonRegistry.h"

namespace oly
{
	class Context
	{
		struct
		{
			std::unique_ptr<rendering::Window> window;
			TextureRegistry texture_registry;
			rendering::NSVGContext nsvg_context;
			mut::SpriteRegistry mut_sprite_registry;
			immut::PolygonRegistry immut_polygon_registry;
		} internal;

	public:
		Context(const char* filepath);
		Context(const Context&) = delete;
		~Context();

		GLenum per_frame_clear_mask = GL_COLOR_BUFFER_BIT;
		bool frame() const;

		const TextureRegistry& texture_registry() const { return internal.texture_registry; }
		TextureRegistry& texture_registry() { return internal.texture_registry; }
		const rendering::NSVGContext& nsvg_context() const { return internal.nsvg_context; }
		rendering::NSVGContext& nsvg_context() { return internal.nsvg_context; }
		const mut::SpriteRegistry& mut_sprite_registry() const { return internal.mut_sprite_registry; }
		mut::SpriteRegistry& mut_sprite_registry() { return internal.mut_sprite_registry; }
		const rendering::Window& window() const { return *internal.window; }
		rendering::Window& window() { return *internal.window; }

		class Mut
		{
			friend class Context;
			const Context* context = nullptr;
			struct
			{
				std::unique_ptr<mut::SpriteBatch> sprite_batch;
			} internal;

		public:
			const mut::SpriteBatch& sprite_batch() const { return *internal.sprite_batch; }
			mut::SpriteBatch& sprite_batch() { return *internal.sprite_batch; }
			mut::Sprite sprite() const;
			mut::Sprite sprite(const std::string& name) const;
			std::weak_ptr<mut::Sprite> ref_sprite(const std::string& name, bool register_if_nonexistant = true) const;
			void render_sprites() const { internal.sprite_batch->render(); }
			void draw_sprite_list(const std::string& draw_list_name, bool register_if_nonexistant = true) const;
		} mut;

		class Immut
		{
			friend class Context;
			const Context* context = nullptr;
			struct
			{
				std::unique_ptr<immut::PolygonBatch> polygon_batch;
			} internal;
			
		public:
			const immut::PolygonBatch& polygon_batch() const { return *internal.polygon_batch; }
			immut::PolygonBatch& polygon_batch() { return *internal.polygon_batch; }
			immut::Polygon polygon() const { return immut::Polygon(internal.polygon_batch.get()); }
			immut::Polygon polygon(const std::string& name) const { return context->internal.immut_polygon_registry.create_polygon(context, name); }
			immut::Composite composite() const { return immut::Composite(internal.polygon_batch.get()); }
			immut::Composite composite(const std::string& name) const { return context->internal.immut_polygon_registry.create_composite(context, name); }
			immut::NGon ngon() const { return immut::NGon(internal.polygon_batch.get()); }
			immut::NGon ngon(const std::string& name) const { return context->internal.immut_polygon_registry.create_ngon(context, name); }
			std::weak_ptr<immut::Polygonal> ref_polygonal(const std::string& name) const { return context->internal.immut_polygon_registry.ref_polygonal(name); }
			std::weak_ptr<immut::Polygon> ref_polygon(const std::string& name) const { return std::dynamic_pointer_cast<immut::Polygon>(ref_polygonal(name).lock()); }
			std::weak_ptr<immut::Composite> ref_composite(const std::string& name) const { return std::dynamic_pointer_cast<immut::Composite>(ref_polygonal(name).lock()); }
			std::weak_ptr<immut::NGon> ref_ngon(const std::string& name) const { return std::dynamic_pointer_cast<immut::NGon>(ref_polygonal(name).lock()); }
			void draw_polygons(size_t draw_spec) const { internal.polygon_batch->draw(draw_spec); }
		} immut;

		void sync_texture_handle(const rendering::BindlessTextureRes& texture) const;
		void sync_texture_handle(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions) const;
	};
}
