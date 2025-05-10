#pragma once

#include "Olympian.h"

namespace oly::gen
{
	struct SpriteMatch
	{
		rendering::Sprite sprite0, sprite2;

		SpriteMatch();

		void draw(bool flush_sprites) const;
	};
}
