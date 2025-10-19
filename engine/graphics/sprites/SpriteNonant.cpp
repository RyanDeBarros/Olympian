#include "SpriteNonant.h"

#include "core/context/rendering/Textures.h"

namespace oly::rendering
{
	Sprite& SpriteNonant::sprite(unsigned char x, unsigned char y) const
	{
		return sprites[size_t(3 * y + x)];
	}

	SpriteNonant::SpriteNonant()
		: sprites(9, Sprite())
	{
		init();
	}

	SpriteNonant::SpriteNonant(Unbatched)
		: sprites(9, Sprite(UNBATCHED))
	{
		init();
	}

	SpriteNonant::SpriteNonant(SpriteBatch& batch)
		: sprites(9, Sprite(batch))
	{
		init();
	}

	void SpriteNonant::init()
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
			{
				sprite(x, y).transformer.attach_parent(&transformer);
				PivotTransformModifier2D modifier;
				modifier.pivot.x = -0.5f * x + 1.0f;
				modifier.pivot.y = -0.5f * y + 1.0f;
				sprite(x, y).transformer.set_modifier() = std::make_unique<PivotTransformModifier2D>(std::move(modifier));
			}
	}

	void SpriteNonant::set_batch(Unbatched)
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_batch(UNBATCHED);
	}

	void SpriteNonant::set_batch(SpriteBatch& batch)
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_batch(batch);
	}

	void SpriteNonant::draw() const
	{
		if (dirty.grid)
		{
			sync_grid();
			dirty.grid = false;
		}
		if (dirty.modulation)
		{
			sync_modulation();
			dirty.modulation = false;
		}
		if (dirty.mod_grid)
		{
			sync_mod_grid();
			dirty.mod_grid = false;
		}
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).draw();
	}

	void SpriteNonant::copy_sprite_attributes(const Sprite& sprite)
	{
		set_local() = sprite.get_local();
		set_frame_format(sprite.get_frame_format());

		// set texture
		glm::vec2 dimensions;
		auto texture = sprite.get_texture(dimensions);
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				this->sprite(x, y).set_texture(texture, dimensions);
		regular_dimensions = dimensions;

		// set mod texture
		texture = sprite.get_mod_texture(dimensions);
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				this->sprite(x, y).set_mod_texture(texture, dimensions);
		regular_mod_dimensions = dimensions;

		// set other attributes
		nsize = regular_dimensions;
		regular_uvs = sprite.get_tex_coords();
		regular_modulation = sprite.get_modulation();
		regular_mod_uvs = sprite.get_mod_tex_coords();

		dirty.grid = true;
		dirty.modulation = true;
		dirty.mod_grid = true;
	}

	void SpriteNonant::set_texture(const ResourcePath& texture_file, unsigned int texture_index)
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_texture(texture_file, texture_index);
		regular_dimensions = context::get_texture_dimensions(texture_file, texture_index);
		nsize = regular_dimensions;
		dirty.grid = true;
		dirty.mod_grid = true;
	}

	void SpriteNonant::set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions)
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_texture(texture, dimensions);
		regular_dimensions = dimensions;
		nsize = regular_dimensions;
		dirty.grid = true;
		dirty.mod_grid = true;
	}

	void SpriteNonant::set_texture(const graphics::BindlessTextureRef& texture)
	{
		glm::vec2 dimensions = context::get_texture_dimensions(texture);
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_texture(texture, dimensions);
		regular_dimensions = dimensions;
		nsize = regular_dimensions;
		dirty.grid = true;
		dirty.mod_grid = true;
	}

	void SpriteNonant::set_tex_coords(math::UVRect rect)
	{
		regular_uvs = rect;
		dirty.grid = true;
	}

	void SpriteNonant::set_modulation(glm::vec4 modulation)
	{
		regular_modulation = modulation;
		dirty.modulation = true;
	}

	void SpriteNonant::set_frame_format(const graphics::AnimFrameFormat& anim) const
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_frame_format(anim);
	}

	void SpriteNonant::set_mod_texture(const ResourcePath& texture_file, unsigned int texture_index)
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_mod_texture(texture_file, texture_index);
		regular_mod_dimensions = context::get_texture_dimensions(texture_file, texture_index);
	}

	void SpriteNonant::set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions)
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_mod_texture(texture, dimensions);
		regular_mod_dimensions = dimensions;
	}

	void SpriteNonant::set_mod_texture(const graphics::BindlessTextureRef& texture)
	{
		glm::vec2 dimensions = context::get_texture_dimensions(texture);
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_mod_texture(texture, dimensions);
		regular_mod_dimensions = dimensions;
	}

	void SpriteNonant::set_mod_tex_coords(math::UVRect rect)
	{
		regular_mod_uvs = rect;
		dirty.mod_grid = true;
	}

	graphics::BindlessTextureRef SpriteNonant::get_texture() const
	{
		return sprite(0, 0).get_texture();
	}

	graphics::BindlessTextureRef SpriteNonant::get_texture(glm::vec2& dimensions) const
	{
		dimensions = regular_dimensions;
		return sprite(0, 0).get_texture();
	}

	math::UVRect SpriteNonant::get_tex_coords() const
	{
		return regular_uvs;
	}

	glm::vec4 SpriteNonant::get_modulation() const
	{
		return regular_modulation;
	}

	graphics::AnimFrameFormat SpriteNonant::get_frame_format() const
	{
		return sprite(0, 0).get_frame_format();
	}

	graphics::BindlessTextureRef SpriteNonant::get_mod_texture() const
	{
		return sprite(0, 0).get_mod_texture();
	}

	graphics::BindlessTextureRef SpriteNonant::get_mod_texture(glm::vec2& dimensions) const
	{
		dimensions = regular_mod_dimensions;
		return sprite(0, 0).get_mod_texture();
	}

	math::UVRect SpriteNonant::get_mod_tex_coords() const
	{
		return regular_mod_uvs;
	}

	math::Padding& SpriteNonant::set_offsets()
	{
		dirty.grid = true;
		dirty.mod_grid = true;
		return offsets;
	}
	
	void SpriteNonant::set_width(float w)
	{
		clamp_nsize({ w, nsize.y });
	}

	void SpriteNonant::set_height(float h)
	{
		clamp_nsize({ nsize.x, h });
	}

	void SpriteNonant::set_size(glm::vec2 size)
	{
		clamp_nsize(size);
	}

	void SpriteNonant::setup_nonant(glm::vec2 size, math::Padding offs)
	{
		offsets = offs;
		clamp_nsize(size);
	}

	void SpriteNonant::setup_nonant(const Sprite& copy, glm::vec2 size, math::Padding offs)
	{
		copy_sprite_attributes(copy);
		setup_nonant(size, offs);
	}

	void SpriteNonant::clamp_nsize(glm::vec2 size)
	{
		nsize.x = std::max(size.x, offsets.left + offsets.right);
		nsize.y = std::max(size.y, offsets.bottom + offsets.top);
		dirty.grid = true;
		dirty.mod_grid = true;
	}

	void SpriteNonant::sync_grid() const
	{
		// [2][0] [2][1] [2][2]
		// [1][0] [1][1] [1][2]
		// [0][0] [0][1] [0][2]

		float xuvs[4]{ 0.0f, offsets.left / regular_dimensions.x, 1.0f - offsets.right / regular_dimensions.x, 1.0f };
		float yuvs[4]{ 0.0f, offsets.bottom / regular_dimensions.y, 1.0f - offsets.top / regular_dimensions.y, 1.0f };

		for (size_t n = 0; n < 4; ++n)
		{
			xuvs[n] = regular_uvs.interp_x(xuvs[n]);
			yuvs[n] = regular_uvs.interp_y(yuvs[n]);
		}

		float widths[3]{ offsets.left, nsize.x - offsets.left - offsets.right, offsets.right };
		float heights[3]{ offsets.bottom, nsize.y - offsets.bottom - offsets.top, offsets.top };

		float xpos[3]{ -0.5f * widths[1], 0.0f, 0.5f * widths[1] };
		float ypos[3]{ -0.5f * heights[1], 0.0f, 0.5f * heights[1] };

		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
			{
				sprite(x, y).set_tex_coords({.x1 = xuvs[x], .x2 = xuvs[x + 1], .y1 = yuvs[y], .y2 = yuvs[y + 1]});
				glm::vec2 size = { widths[x], heights[y] };
				sprite(x, y).transformer.ref_modifier<PivotTransformModifier2D>().size = size;
				Transform2D& local = sprite(x, y).set_local();
				local.scale = size / regular_dimensions;
				local.position = glm::vec2{ xpos[x], ypos[y] };
			}
	}

	void SpriteNonant::sync_modulation() const
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_modulation(regular_modulation);
	}

	void SpriteNonant::sync_mod_grid() const
	{
		// [2][0] [2][1] [2][2]
		// [1][0] [1][1] [1][2]
		// [0][0] [0][1] [0][2]

		float xuvs[4]{ 0.0f, offsets.left / nsize.x, 1.0f - offsets.right / nsize.x, 1.0f };
		float yuvs[4]{ 0.0f, offsets.bottom / nsize.y, 1.0f - offsets.top / nsize.y, 1.0f };

		for (size_t n = 0; n < 4; ++n)
		{
			xuvs[n] = regular_mod_uvs.interp_x(xuvs[n]);
			yuvs[n] = regular_mod_uvs.interp_y(yuvs[n]);
		}

		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_mod_tex_coords({ .x1 = xuvs[x], .x2 = xuvs[x + 1], .y1 = yuvs[y], .y2 = yuvs[y + 1] });
	}
}
