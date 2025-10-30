#pragma once

#include "graphics/sprites/Sprite.h"
#include "core/base/TransformerExposure.h"

namespace oly::rendering
{
	class SpriteNonant
	{
		mutable FixedVector<Sprite> sprites;

		Sprite& sprite(unsigned char x, unsigned char y) const;

		glm::vec2 nsize{};
		glm::vec2 regular_dimensions{};
		math::UVRect regular_uvs;
		glm::vec4 regular_modulation = glm::vec4(1.0f);
		glm::vec2 regular_mod_dimensions{};
		math::UVRect regular_mod_uvs;
		math::Padding offsets;
		
		struct
		{
			bool grid = false;
			bool modulation = false;
			bool mod_grid = false;
		} mutable dirty;

		Transformer2D transformer;

	public:
		SpriteNonant();
		SpriteNonant(Unbatched);
		SpriteNonant(SpriteBatch& batch);

	private:
		void init();

	public:
		auto get_batch() const { return sprite(0, 0).get_batch(); }
		void set_batch(Unbatched);
		void set_batch(SpriteBatch& batch);

		void draw() const;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure<TExposureParams{ .local = exposure::local::FULL, .chain = exposure::chain::ATTACH_ONLY, .modifier = exposure::modifier::FULL }>
			set_transformer() { return transformer; }

		void copy_sprite_attributes(const Sprite& sprite);
		void set_texture(const ResourcePath& texture_file, unsigned int texture_index = 0);
		void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions);
		void set_texture(const graphics::BindlessTextureRef& texture);
		void set_tex_coords(math::UVRect rect);
		void set_modulation(glm::vec4 modulation);
		void set_frame_format(const graphics::AnimFrameFormat& anim) const;
		void set_camera_invariant(bool is_camera_invariant) const;
		void set_mod_texture(const ResourcePath& texture_file, unsigned int texture_index = 0);
		void set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions);
		void set_mod_texture(const graphics::BindlessTextureRef& texture);
		void set_mod_tex_coords(math::UVRect rect);

		graphics::BindlessTextureRef get_texture() const;
		graphics::BindlessTextureRef get_texture(glm::vec2& dimensions) const;
		math::UVRect get_tex_coords() const;
		glm::vec4 get_modulation() const;
		graphics::AnimFrameFormat get_frame_format() const;
		bool is_camera_invariant() const;
		graphics::BindlessTextureRef get_mod_texture() const;
		graphics::BindlessTextureRef get_mod_texture(glm::vec2& dimensions) const;
		math::UVRect get_mod_tex_coords() const;

		math::Padding& set_offsets();
		math::Padding get_offsets() const { return offsets; }

		float width() const { return nsize.x; }
		float height() const { return nsize.y; }
		glm::vec2 size() const { return nsize; }
		void set_width(float w);
		void set_height(float h);
		void set_size(glm::vec2 size);

		void setup_nonant(glm::vec2 nsize, math::Padding offsets);
		void setup_nonant(const Sprite& copy, glm::vec2 nsize, math::Padding offsets);

		glm::vec2 get_dimensions() const { return regular_dimensions; }

	private:
		void clamp_nsize(glm::vec2 nsize);
		void sync_grid() const;
		void sync_modulation() const;
		void sync_mod_grid() const;
	};

	typedef SmartReference<SpriteNonant> SpriteNonantRef;
}
