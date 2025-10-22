#pragma once

#include <Olympian.h>

struct SpriteMatch
{
	oly::Transformer2D transformer;
	oly::rendering::SpriteRef sprite0;
	oly::rendering::SpriteRef sprite2;

	SpriteMatch();
	SpriteMatch(const SpriteMatch&) = default;
	SpriteMatch(SpriteMatch&&) = default;
	SpriteMatch& operator=(const SpriteMatch&) = default;
	SpriteMatch& operator=(SpriteMatch&&) = default;

	const oly::Transform2D& get_local() const { return transformer.get_local(); }
	oly::Transform2D& set_local() { return transformer.set_local(); }

	void draw() const;

	void on_tick() const;
};
