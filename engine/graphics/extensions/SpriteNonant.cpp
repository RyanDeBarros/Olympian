#include "SpriteNonant.h"

#include "core/base/Context.h"

namespace oly::rendering
{
	SpriteNonant::SpriteNonant()
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
			{
				sprites[i][j].transformer.attach_parent(&transformer);
				PivotTransformModifier2D modifier;
				modifier.pivot.x = -0.5f * j + 1.0f;
				modifier.pivot.y = -0.5f * i + 1.0f;
				sprites[i][j].transformer.modifier = move_unique(std::move(modifier));
			}
	}

	SpriteNonant::SpriteNonant(const SpriteNonant& other)
		: transformer(other.transformer), regular_dimensions(other.regular_dimensions), regular_uvs(other.regular_uvs),
		offsets(other.offsets), regular_modulation(other.regular_modulation)
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				sprites[i][j] = other.sprites[i][j];
	}

	SpriteNonant& SpriteNonant::operator=(const SpriteNonant& other)
	{
		if (this != &other)
		{
			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
					sprites[i][j] = other.sprites[i][j];

			transformer = other.transformer;
			regular_dimensions = other.regular_dimensions;
			regular_uvs = other.regular_uvs;
			offsets = other.offsets;
			regular_modulation = other.regular_modulation;
		}
		return *this;
	}

	void SpriteNonant::draw() const
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				sprites[i][j].draw();
	}

	void SpriteNonant::set_texture(const std::string& texture_file, unsigned int texture_index)
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				sprites[i][j].set_texture(texture_file, texture_index);
		regular_dimensions = context::get_texture_dimensions(texture_file, texture_index);
		nsize = regular_dimensions;
		sync_grid();
	}

	void SpriteNonant::set_texture(const std::string& texture_file, float svg_scale, unsigned int texture_index)
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				sprites[i][j].set_texture(texture_file, svg_scale, texture_index);
		regular_dimensions = context::get_texture_dimensions(texture_file, texture_index);
		nsize = regular_dimensions;
		sync_grid();
	}

	void SpriteNonant::set_texture(const graphics::BindlessTextureRes& texture, glm::vec2 dimensions)
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				sprites[i][j].set_texture(texture, dimensions);
		regular_dimensions = dimensions;
		nsize = regular_dimensions;
		sync_grid();
	}

	void SpriteNonant::set_tex_coords(const math::Rect2D& rect)
	{
		regular_uvs = rect;
		sync_grid();
	}

	void SpriteNonant::set_modulation(const ModulationRect& modulation)
	{
		regular_modulation = modulation;
		sync_modulation();
	}

	void SpriteNonant::set_modulation(glm::vec4 modulation)
	{
		regular_modulation = { modulation, modulation, modulation, modulation };
		sync_modulation();
	}
	
	void SpriteNonant::set_frame_format(const graphics::AnimFrameFormat& anim) const
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				sprites[i][j].set_frame_format(anim);
	}

	graphics::BindlessTextureRes SpriteNonant::get_texture() const
	{
		return sprites[0][0].get_texture();
	}

	graphics::BindlessTextureRes SpriteNonant::get_texture(glm::vec2& dimensions) const
	{
		dimensions = regular_dimensions;
		return sprites[0][0].get_texture();
	}

	math::Rect2D SpriteNonant::get_tex_coords() const
	{
		return regular_uvs;
	}

	ModulationRect SpriteNonant::get_modulation() const
	{
		// TODO
		return ModulationRect();
	}

	graphics::AnimFrameFormat SpriteNonant::get_frame_format() const
	{
		return sprites[0][0].get_frame_format();
	}

	void SpriteNonant::set_x_left_offset(float xoff)
	{
		offsets.x_left = xoff;
		sync_grid();
	}
	
	void SpriteNonant::set_x_right_offset(float xoff)
	{
		offsets.x_right = xoff;
		sync_grid();
	}
	
	void SpriteNonant::set_y_bottom_offset(float yoff)
	{
		offsets.y_bottom = yoff;
		sync_grid();
	}
	
	void SpriteNonant::set_y_top_offset(float yoff)
	{
		offsets.y_top = yoff;
		sync_grid();
	}
	
	void SpriteNonant::set_offsets(float x_left, float x_right, float y_bottom, float y_top)
	{
		offsets.x_left = x_left;
		offsets.x_right = x_right;
		offsets.y_bottom = y_bottom;
		offsets.y_top = y_top;
		sync_grid();
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
		nsize.x = w;
		sync_grid();
	}

	void SpriteNonant::set_height(float h)
	{
		nsize.y = h;
		sync_grid();
	}

	void SpriteNonant::set_size(glm::vec2 size)
	{
		nsize = size;
		sync_grid();
	}

	void SpriteNonant::sync_grid()
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

		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
			{
				sprites[i][j].set_tex_coords(math::Rect2D{ .x1 = xuvs[j], .x2 = xuvs[j + 1], .y1 = yuvs[i], .y2 = yuvs[i + 1] });
				glm::vec2 size = { widths[j], heights[i] };
				sprites[i][j].transformer.get_modifier<PivotTransformModifier2D>().size = size;
				Transform2D& local = sprites[i][j].set_local();
				local.scale = size / regular_dimensions;
				local.position = glm::vec2{ xpos[j], ypos[i] };
			}

		sync_modulation();
	}

	void SpriteNonant::sync_modulation()
	{
		// TODO
	}
}
