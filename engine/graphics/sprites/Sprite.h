#pragma once

#include "graphics/sprites/SpriteBatch.h"
#include "core/base/Transforms.h"

namespace oly::rendering
{
	// ASSET
	class StaticSprite
	{
		internal::SpriteReference ref;

	public:
		StaticSprite() = default;
		StaticSprite(Unbatched) : ref(UNBATCHED) {}
		StaticSprite(internal::SpriteBatch& batch) : ref(batch) {}
		StaticSprite(SpriteBatch& batch) : ref(batch) {}
		StaticSprite(const StaticSprite&) = default;
		StaticSprite(StaticSprite&&) noexcept = default;
		StaticSprite& operator=(const StaticSprite&) = default;
		StaticSprite& operator=(StaticSprite&&) noexcept = default;

		auto get_batch() const { return ref.get_batch(); }
		void set_batch(Unbatched) { ref.set_batch(UNBATCHED); }
		void set_batch(SpriteBatch& batch) { ref.set_batch(batch); }

		void draw() const;

		void set_texture(const ResourcePath& texture_file, unsigned int texture_index = 0) const { ref.set_texture(texture_file, texture_index); }
		void set_texture(const graphics::BindlessTextureRef& texture) const { ref.set_texture(texture); }
		void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const { ref.set_texture(texture, dimensions); }
		void set_tex_coords(math::UVRect uvs) const { ref.set_tex_coords(uvs); }
		void set_modulation(glm::vec4 modulation) const { ref.set_modulation(modulation); }
		void set_frame_format(const graphics::AnimFrameFormat& anim) const { ref.set_frame_format(anim); }
		void set_mod_texture(const ResourcePath& texture_file, unsigned int texture_index = 0) const { ref.set_mod_texture(texture_file, texture_index); }
		void set_mod_texture(const graphics::BindlessTextureRef& texture) const { ref.set_mod_texture(texture); }
		void set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const { ref.set_mod_texture(texture, dimensions); }
		void set_mod_tex_coords(math::UVRect uvs) const { ref.set_mod_tex_coords(uvs); }
		void set_transform(const glm::mat3& transform) { ref.set_transform(transform); }

		graphics::BindlessTextureRef get_texture() const { return ref.get_texture(); }
		graphics::BindlessTextureRef get_texture(glm::vec2& dimensions) const { return ref.get_texture(dimensions); }
		math::UVRect get_tex_coords() const { return ref.get_tex_coords(); }
		glm::vec4 get_modulation() const { return ref.get_modulation(); }
		graphics::AnimFrameFormat get_frame_format() const { return ref.get_frame_format(); }
		graphics::BindlessTextureRef get_mod_texture() const { return ref.get_mod_texture(); }
		graphics::BindlessTextureRef get_mod_texture(glm::vec2& dimensions) const { return ref.get_mod_texture(dimensions); }
		math::UVRect get_mod_tex_coords() const { return ref.get_mod_tex_coords(); }
		glm::mat3 get_transform() const { return ref.get_transform(); }
	};

	class Sprite
	{
		internal::SpriteReference ref;

	public:
		Transformer2D transformer;

		Sprite() = default;
		Sprite(Unbatched) : ref(UNBATCHED) {}
		Sprite(internal::SpriteBatch& batch) : ref(batch) {}
		Sprite(SpriteBatch& batch) : ref(batch) {}
		Sprite(const Sprite&) = default;
		Sprite(Sprite&&) noexcept = default;
		Sprite& operator=(const Sprite&) = default;
		Sprite& operator=(Sprite&&) noexcept = default;

		auto get_batch() const { return ref.get_batch(); }
		void set_batch(Unbatched) { ref.set_batch(UNBATCHED); }
		void set_batch(SpriteBatch& batch) { ref.set_batch(batch); }

		void draw() const;

		void set_texture(const ResourcePath& texture_file, unsigned int texture_index = 0) const { ref.set_texture(texture_file, texture_index); }
		void set_texture(const graphics::BindlessTextureRef& texture) const { ref.set_texture(texture); }
		void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const { ref.set_texture(texture, dimensions); }
		void set_tex_coords(math::UVRect uvs) const { ref.set_tex_coords(uvs); }
		void set_modulation(glm::vec4 modulation) const { ref.set_modulation(modulation); }
		void set_frame_format(const graphics::AnimFrameFormat& anim) const { ref.set_frame_format(anim); }
		void set_mod_texture(const ResourcePath& texture_file, unsigned int texture_index = 0) const { ref.set_mod_texture(texture_file, texture_index); }
		void set_mod_texture(const graphics::BindlessTextureRef& texture) const { ref.set_mod_texture(texture); }
		void set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const { ref.set_mod_texture(texture, dimensions); }
		void set_mod_tex_coords(math::UVRect uvs) const { ref.set_mod_tex_coords(uvs); }

		graphics::BindlessTextureRef get_texture() const { return ref.get_texture(); }
		graphics::BindlessTextureRef get_texture(glm::vec2& dimensions) const { return ref.get_texture(dimensions); }
		math::UVRect get_tex_coords() const { return ref.get_tex_coords(); }
		glm::vec4 get_modulation() const { return ref.get_modulation(); }
		graphics::AnimFrameFormat get_frame_format() const { return ref.get_frame_format(); }
		graphics::BindlessTextureRef get_mod_texture() const { return ref.get_mod_texture(); }
		graphics::BindlessTextureRef get_mod_texture(glm::vec2& dimensions) const { return ref.get_mod_texture(dimensions); }
		math::UVRect get_mod_tex_coords() const { return ref.get_mod_tex_coords(); }

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }
	};

	typedef SmartReference<Sprite> SpriteRef;
}
