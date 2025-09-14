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
		StaticSprite(SpriteBatch* batch = nullptr);
		StaticSprite(const StaticSprite&);
		StaticSprite(StaticSprite&&) noexcept;
		StaticSprite& operator=(const StaticSprite&);
		StaticSprite& operator=(StaticSprite&&) noexcept;
		~StaticSprite();

		void draw() const;

		void set_texture(const std::string& texture_file, unsigned int texture_index = 0) const { ref.set_texture(texture_file, texture_index); }
		void set_texture(const graphics::BindlessTextureRef& texture) const { ref.set_texture(texture); }
		void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const { ref.set_texture(texture, dimensions); }
		void set_tex_coords(math::Rect2D uvs) const { ref.set_tex_coords(uvs); }
		void set_modulation(glm::vec4 modulation) const { ref.set_modulation(modulation); }
		void set_frame_format(const graphics::AnimFrameFormat& anim) const { ref.set_frame_format(anim); }
		void set_transform(const glm::mat3& transform) { ref.set_transform(transform); }

		graphics::BindlessTextureRef get_texture() const { return ref.get_texture(); }
		graphics::BindlessTextureRef get_texture(glm::vec2& dimensions) const { return ref.get_texture(dimensions); }
		math::Rect2D get_tex_coords() const { return ref.get_tex_coords(); }
		glm::vec4 get_modulation() const { return ref.get_modulation(); }
		graphics::AnimFrameFormat get_frame_format() const { return ref.get_frame_format(); }
		glm::mat3 get_transform() const { return ref.get_transform(); }
	};

	class Sprite
	{
		internal::SpriteReference ref;

	public:
		Transformer2D transformer;

		Sprite(SpriteBatch* batch = nullptr);
		Sprite(const Sprite&);
		Sprite(Sprite&&) noexcept;
		Sprite& operator=(const Sprite&);
		Sprite& operator=(Sprite&&) noexcept;
		~Sprite();

		void draw() const;

		void set_texture(const std::string& texture_file, unsigned int texture_index = 0) const { ref.set_texture(texture_file, texture_index); }
		void set_texture(const graphics::BindlessTextureRef& texture) const { ref.set_texture(texture); }
		void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const { ref.set_texture(texture, dimensions); }
		void set_tex_coords(math::Rect2D uvs) const { ref.set_tex_coords(uvs); }
		void set_modulation(glm::vec4 modulation) const { ref.set_modulation(modulation); }
		void set_frame_format(const graphics::AnimFrameFormat& anim) const { ref.set_frame_format(anim); }

		graphics::BindlessTextureRef get_texture() const { return ref.get_texture(); }
		graphics::BindlessTextureRef get_texture(glm::vec2& dimensions) const { return ref.get_texture(dimensions); }
		math::Rect2D get_tex_coords() const { return ref.get_tex_coords(); }
		glm::vec4 get_modulation() const { return ref.get_modulation(); }
		graphics::AnimFrameFormat get_frame_format() const { return ref.get_frame_format(); }

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }
	};

	typedef SmartReference<Sprite> SpriteRef;
}
