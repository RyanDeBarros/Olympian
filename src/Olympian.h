#pragma once

#include "rendering/core/Core.h"
#include "rendering/TextureRegistry.h"
#include "rendering/mutable/Sprites.h"
#include "rendering/mutable/SpriteRegistry.h"

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
			mut::Sprite sprite() const { return mut::Sprite(internal.sprite_batch.get()); }
			std::weak_ptr<mut::Sprite> sprite(const std::string& name, bool register_if_nonexistant = true) const { return context->internal.mut_sprite_registry.get_sprite(context, name, register_if_nonexistant); }
			void render_sprites() const { internal.sprite_batch->render(); }
			void draw_sprite_list(const std::string& draw_list_name, bool register_if_nonexistant = true) const;
		} mut;

		void sync_texture_handle(const rendering::BindlessTextureRes& texture);
		void sync_texture_handle(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions);
	};
}
