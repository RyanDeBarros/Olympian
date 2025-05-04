#include "Textures.h"

#include "core/types/Meta.h"

namespace oly::graphics::textures
{
	BindlessTextureRes white1x1_1;

	void internal::load()
	{
		{
			ImageDimensions dim{ 1, 1, 1 };
			unsigned char* buf = dim.pxnew();
			buf[0] = 255;
			Image image(buf, dim);
			white1x1_1 = move_shared(load_bindless_texture_2d(image));
			white1x1_1->set_and_use_handle();
		}
	}

	void internal::unload()
	{
		white1x1_1.reset();
	}
}
