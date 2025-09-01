#include "SpriteNonant.h"

#include "core/context/rendering/Textures.h"

namespace oly::rendering
{
	Sprite& SpriteNonant::sprite(unsigned char x, unsigned char y) const
	{
		return sprites[3 * y + x];
	}

	SpriteNonant::SpriteNonant()
		: sprites(9)
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

	void SpriteNonant::draw() const
	{
		if (dirty.grid)
		{
			sync_grid();
			dirty.grid = false;
			dirty.modulation = false; // sync_modulation() is already called in sync_grid()
		}
		else if (dirty.modulation)
		{
			sync_modulation();
			dirty.modulation = false;
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
		nsize = regular_dimensions;

		// set tex_coords
		auto tex_coords = sprite.get_tex_coords();
		regular_uvs = { .x1 = tex_coords.uvs[0].x, .x2 = tex_coords.uvs[1].x, .y1 = tex_coords.uvs[0].y, .y2 = tex_coords.uvs[2].y };

		// set modulation
		regular_modulation = sprite.get_modulation();

		dirty.grid = true;
	}

	void SpriteNonant::set_texture(const std::string& texture_file, unsigned int texture_index)
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_texture(texture_file, texture_index);
		regular_dimensions = context::get_texture_dimensions(texture_file, texture_index);
		nsize = regular_dimensions;
		dirty.grid = true;
	}

	void SpriteNonant::set_texture(const std::string& texture_file, float svg_scale, unsigned int texture_index)
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_texture(texture_file, svg_scale, texture_index);
		regular_dimensions = context::get_texture_dimensions(texture_file, texture_index);
		nsize = regular_dimensions;
		dirty.grid = true;
	}

	void SpriteNonant::set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions)
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_texture(texture, dimensions);
		regular_dimensions = dimensions;
		nsize = regular_dimensions;
		dirty.grid = true;
	}

	void SpriteNonant::set_tex_coords(const math::Rect2D& rect)
	{
		regular_uvs = rect;
		dirty.grid = true;
	}

	void SpriteNonant::set_modulation(const ModulationRect& modulation)
	{
		regular_modulation = modulation;
		dirty.modulation = true;
	}

	void SpriteNonant::set_modulation(glm::vec4 modulation)
	{
		regular_modulation = { modulation, modulation, modulation, modulation };
		dirty.modulation = true;
	}
	
	void SpriteNonant::set_frame_format(const graphics::AnimFrameFormat& anim) const
	{
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_frame_format(anim);
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

	math::Rect2D SpriteNonant::get_tex_coords() const
	{
		return regular_uvs;
	}

	ModulationRect SpriteNonant::get_modulation() const
	{
		return regular_modulation;
	}

	graphics::AnimFrameFormat SpriteNonant::get_frame_format() const
	{
		return sprite(0, 0).get_frame_format();
	}

	void SpriteNonant::set_x_left_offset(float xoff)
	{
		offsets.x_left = xoff;
		dirty.grid = true;
	}
	
	void SpriteNonant::set_x_right_offset(float xoff)
	{
		offsets.x_right = xoff;
		dirty.grid = true;
	}
	
	void SpriteNonant::set_y_bottom_offset(float yoff)
	{
		offsets.y_bottom = yoff;
		dirty.grid = true;
	}
	
	void SpriteNonant::set_y_top_offset(float yoff)
	{
		offsets.y_top = yoff;
		dirty.grid = true;
	}
	
	void SpriteNonant::set_offsets(float x_left, float x_right, float y_bottom, float y_top)
	{
		offsets.x_left = x_left;
		offsets.x_right = x_right;
		offsets.y_bottom = y_bottom;
		offsets.y_top = y_top;
		dirty.grid = true;
	}
	
	void SpriteNonant::get_offsets(float* x_left, float* x_right, float* y_bottom, float* y_top) const
	{
		if (x_left)
			*x_left = offsets.x_left;
		if (x_right)
			*x_right = offsets.x_right;
		if (y_bottom)
			*y_bottom = offsets.y_bottom;
		if (y_top)
			*y_top = offsets.y_top;
	}

	void SpriteNonant::set_width(float w)
	{
		clamp_nsize({ w, nsize.y });
		dirty.grid = true;
	}

	void SpriteNonant::set_height(float h)
	{
		clamp_nsize({ nsize.x, h });
		dirty.grid = true;
	}

	void SpriteNonant::set_size(glm::vec2 size)
	{
		clamp_nsize(size);
		dirty.grid = true;
	}

	void SpriteNonant::setup_nonant(glm::vec2 size, float x_left, float x_right, float y_bottom, float y_top)
	{
		offsets.x_left = x_left;
		offsets.x_right = x_right;
		offsets.y_bottom = y_bottom;
		offsets.y_top = y_top;
		clamp_nsize(size);
		dirty.grid = true;
	}

	void SpriteNonant::setup_nonant(const Sprite& copy, glm::vec2 size, float x_left, float x_right, float y_bottom, float y_top)
	{
		offsets.x_left = x_left;
		offsets.x_right = x_right;
		offsets.y_bottom = y_bottom;
		offsets.y_top = y_top;

		set_local() = copy.get_local();
		set_frame_format(copy.get_frame_format());

		// set texture
		glm::vec2 dimensions;
		auto texture = copy.get_texture(dimensions);
		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
				sprite(x, y).set_texture(texture, dimensions);
		regular_dimensions = dimensions;
		nsize = regular_dimensions;

		// set nsize
		clamp_nsize(size);

		// set tex_coords
		auto tex_coords = copy.get_tex_coords();
		regular_uvs = { .x1 = tex_coords.uvs[0].x, .x2 = tex_coords.uvs[1].x, .y1 = tex_coords.uvs[0].y, .y2 = tex_coords.uvs[2].y };

		// set modulation
		regular_modulation = copy.get_modulation();

		dirty.grid = true;
	}

	void SpriteNonant::clamp_nsize(glm::vec2 size)
	{
		nsize.x = std::max(size.x, offsets.x_left + offsets.x_right);
		nsize.y = std::max(size.y, offsets.y_bottom + offsets.y_top);
	}

	void SpriteNonant::sync_grid() const
	{
		// [2][0] [2][1] [2][2]
		// [1][0] [1][1] [1][2]
		// [0][0] [0][1] [0][2]

		float xuvs[4]{ 0.0f, offsets.x_left / regular_dimensions.x, 1.0f - offsets.x_right / regular_dimensions.x, 1.0f };
		float yuvs[4]{ 0.0f, offsets.y_bottom / regular_dimensions.y, 1.0f - offsets.y_top / regular_dimensions.y, 1.0f };

		for (size_t n = 0; n < 4; ++n)
		{
			xuvs[n] = xuvs[n] * (regular_uvs.x2 - regular_uvs.x1) + regular_uvs.x1;
			yuvs[n] = yuvs[n] * (regular_uvs.y2 - regular_uvs.y1) + regular_uvs.y1;
		}

		float widths[3]{ offsets.x_left, nsize.x - offsets.x_left - offsets.x_right, offsets.x_right };
		float heights[3]{ offsets.y_bottom, nsize.y - offsets.y_bottom - offsets.y_top, offsets.y_top };

		float xpos[3]{ -0.5f * widths[1], 0.0f, 0.5f * widths[1] };
		float ypos[3]{ -0.5f * heights[1], 0.0f, 0.5f * heights[1] };

		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
			{
				sprite(x, y).set_tex_coords(math::Rect2D{.x1 = xuvs[x], .x2 = xuvs[x + 1], .y1 = yuvs[y], .y2 = yuvs[y + 1]});
				glm::vec2 size = { widths[x], heights[y] };
				sprite(x, y).transformer.ref_modifier<PivotTransformModifier2D>().size = size;
				Transform2D& local = sprite(x, y).set_local();
				local.scale = size / regular_dimensions;
				local.position = glm::vec2{ xpos[x], ypos[y] };
			}

		sync_modulation();
	}

	void SpriteNonant::sync_modulation() const
	{
		float xcuvs[4]{ 0.0f, offsets.x_left / nsize.x, 1.0f - offsets.x_right / nsize.x, 1.0f };
		float ycuvs[4]{ 0.0f, offsets.y_bottom / nsize.y, 1.0f - offsets.y_top / nsize.y, 1.0f };

		for (unsigned char x = 0; x < 3; ++x)
			for (unsigned char y = 0; y < 3; ++y)
			{
				ModulationRect mod;
				mod.colors[0] = regular_modulation.mix({ xcuvs[x]    , ycuvs[y]     });
				mod.colors[1] = regular_modulation.mix({ xcuvs[x + 1], ycuvs[y]     });
				mod.colors[2] = regular_modulation.mix({ xcuvs[x + 1], ycuvs[y + 1] });
				mod.colors[3] = regular_modulation.mix({ xcuvs[x]    , ycuvs[y + 1] });
				sprite(x, y).set_modulation(mod);
			}
	}
}
