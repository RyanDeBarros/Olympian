#include "Sprite.h"

#include "core/context/rendering/Sprites.h"

namespace oly::rendering
{
	void StaticSprite::draw() const
	{
		ref.draw_quad();
		if (ref.is_in_context()) [[likely]]
			context::internal::set_sprite_batch_rendering(true);
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
