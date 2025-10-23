#pragma once

#include <Olympian.h>

struct SpriteMatch : public oly::rendering::IDrawable
{
	oly::Transformer2D transformer;
	oly::rendering::Drawable<oly::rendering::Sprite> sprite0;
	oly::rendering::Drawable<oly::rendering::Sprite> sprite2;

	SpriteMatch();
	SpriteMatch(const SpriteMatch&) = default;
	SpriteMatch(SpriteMatch&&) = default;
	SpriteMatch& operator=(const SpriteMatch&) = default;
	SpriteMatch& operator=(SpriteMatch&&) = default;

	const oly::Transform2D& get_local() const { return transformer.get_local(); }
	oly::Transform2D& set_local() { return transformer.set_local(); }

	void on_tick() const;
};
