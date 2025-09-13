#include "Sprite.h"

#include "core/context/rendering/Sprites.h"

namespace oly::rendering
{
	StaticSprite::StaticSprite(SpriteBatch* batch)
		: ref(batch)
	{
	}

	StaticSprite::StaticSprite(const StaticSprite& other)
		: ref(other.ref)
	{
		glm::vec2 dim;
		auto tex = other.get_texture(dim);
		set_texture(tex, dim);
		set_tex_coords(other.get_tex_coords());
		set_modulation(other.get_modulation());
		set_frame_format(other.get_frame_format());
	}

	StaticSprite::StaticSprite(StaticSprite&& other) noexcept
		: ref(other.ref)
	{
	}

	StaticSprite& StaticSprite::operator=(const StaticSprite& other)
	{
		if (this != &other)
		{
			ref = other.ref;
			glm::vec2 dim;
			auto tex = other.get_texture(dim);
			set_texture(tex, dim);
			set_tex_coords(other.get_tex_coords());
			set_modulation(other.get_modulation());
			set_frame_format(other.get_frame_format());
		}
		return *this;
	}

	StaticSprite& StaticSprite::operator=(StaticSprite&& other) noexcept
	{
		if (this != &other)
		{
			ref = std::move(other.ref);
			if (&ref.batch != &other.ref.batch)
			{
				glm::vec2 dim;
				auto tex = other.get_texture(dim);
				set_texture(tex, dim);
				set_tex_coords(other.get_tex_coords());
				set_modulation(other.get_modulation());
				set_frame_format(other.get_frame_format());
			}
		}
		return *this;
	}

	StaticSprite::~StaticSprite()
	{
	}

	void StaticSprite::draw() const
	{
		ref.draw_quad();
		if (ref.is_in_context()) [[likely]]
			context::internal::set_sprite_batch_rendering(true);
	}

	Sprite::Sprite(SpriteBatch* batch)
		: ref(batch)
	{
	}

	Sprite::Sprite(const Sprite& other)
		: ref(other.ref), transformer(other.transformer)
	{
		glm::vec2 dim;
		auto tex = other.get_texture(dim);
		set_texture(tex, dim);
		set_tex_coords(other.get_tex_coords());
		set_modulation(other.get_modulation());
		set_frame_format(other.get_frame_format());
	}

	Sprite::Sprite(Sprite&& other) noexcept
		: ref(std::move(other.ref)), transformer(std::move(other.transformer))
	{
	}

	Sprite& Sprite::operator=(const Sprite& other)
	{
		if (this != &other)
		{
			ref = other.ref;
			transformer = other.transformer;
			glm::vec2 dim;
			auto tex = other.get_texture(dim);
			set_texture(tex, dim);
			set_tex_coords(other.get_tex_coords());
			set_modulation(other.get_modulation());
			set_frame_format(other.get_frame_format());
		}
		return *this;
	}

	Sprite& Sprite::operator=(Sprite&& other) noexcept
	{
		if (this != &other)
		{
			ref = std::move(other.ref);
			transformer = std::move(other.transformer);
			if (&ref.batch != &other.ref.batch)
			{
				glm::vec2 dim;
				auto tex = other.get_texture(dim);
				set_texture(tex, dim);
				set_tex_coords(other.get_tex_coords());
				set_modulation(other.get_modulation());
				set_frame_format(other.get_frame_format());
			}
		}
		return *this;
	}

	Sprite::~Sprite()
	{
	}

	void Sprite::draw() const
	{
		if (transformer.flush())
			ref.set_transform(transformer.global());
		ref.draw_quad();
		if (ref.is_in_context()) [[likely]]
			context::internal::set_sprite_batch_rendering(true);
	}
}
