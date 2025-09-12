#include "Textures.h"

#include "core/types/Meta.h"

namespace oly::graphics::textures
{
	// TODO v4 load these from actual image files in internal/textures. Allow assets to access them using a special filepath format - instead of RES://, use some other prefix. Also, when loading textures in general, use these prefixes.
	BindlessTextureRef white1x1;

	void internal::load()
	{
		{
			ImageDimensions dim{ .w = 1, .h = 1, .cpp = 1 };
			unsigned char* buf = dim.pxnew();
			buf[0] = 255;
			white1x1 = graphics::BindlessTextureRef(load_bindless_texture_2d(Image(buf, dim)));
			white1x1->set_and_use_handle();
		}
	}

	void internal::unload()
	{
		white1x1.invalidate();
	}
}
