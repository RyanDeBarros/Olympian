#pragma once

#include "graphics/primitives/Sprites.h"

namespace oly::rendering
{
	class SpriteNonant
	{
		mutable Sprite sprites[3][3];

		glm::vec2 nsize{};
		glm::vec2 regular_dimensions{};
		math::Rect2D regular_uvs{ .x1 = 0.0f, .x2 = 1.0f, .y1 = 0.0f, .y2 = 1.0f };
		ModulationRect regular_modulation{};

		struct
		{
			float x_left = 0.0f, x_right = 0.0f, y_bottom = 0.0f, y_top = 0.0f;
		} offsets;
		
		struct
		{
			bool grid = false;
			bool modulation = false;
		} mutable dirty;

	public:
		Transformer2D transformer; // TODO public transformers in extensions like SpriteNonant and TileMap should not have their children public

		SpriteNonant();
		SpriteNonant(const SpriteNonant&);
		SpriteNonant& operator=(const SpriteNonant&);

		void draw() const;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void copy_sprite_attributes(const Sprite& sprite);
		void set_texture(const std::string& texture_file, unsigned int texture_index = 0);
		void set_texture(const std::string& texture_file, float svg_scale, unsigned int texture_index = 0);
		void set_texture(const graphics::BindlessTextureRes& texture, glm::vec2 dimensions);
		void set_tex_coords(const math::Rect2D& rect);
		void set_modulation(const ModulationRect& modulation);
		void set_modulation(glm::vec4 modulation);
		void set_frame_format(const graphics::AnimFrameFormat& anim) const;

		graphics::BindlessTextureRes get_texture() const;
		graphics::BindlessTextureRes get_texture(glm::vec2& dimensions) const;
		math::Rect2D get_tex_coords() const;
		ModulationRect get_modulation() const;
		graphics::AnimFrameFormat get_frame_format() const;

		void set_x_left_offset(float xoff);
		void set_x_right_offset(float xoff);
		void set_y_bottom_offset(float yoff);
		void set_y_top_offset(float yoff);
		void set_offsets(float x_left, float x_right, float y_bottom, float y_top);

		float get_x_left_offset() const { return offsets.x_left; }
		float get_x_right_offset() const { return offsets.x_right; }
		float get_y_bottom_offset() const { return offsets.y_bottom; }
		float get_y_top_offset() const { return offsets.y_top; }
		void get_offsets(float* x_left, float* x_right, float* y_bottom, float* y_top) const;

		float width() const { return nsize.x; }
		float height() const { return nsize.y; }
		glm::vec2 size() const { return nsize; }
		void set_width(float w);
		void set_height(float h);
		void set_size(glm::vec2 size);

		void setup_nonant(glm::vec2 nsize, float x_left, float x_right, float y_bottom, float y_top);
		void setup_nonant(const Sprite& copy, glm::vec2 nsize, float x_left, float x_right, float y_bottom, float y_top);

		glm::vec2 get_dimensions() const { return regular_dimensions; }

	private:
		void clamp_nsize(glm::vec2 nsize);
		void sync_grid() const;
		void sync_modulation() const;
	};
}
