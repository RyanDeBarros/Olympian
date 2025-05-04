#include "Textures.h"

#include "core/types/Meta.h"

namespace oly
{
	namespace textures
	{
		rendering::BindlessTextureRes white1x1_1;

		void load()
		{
			{
				rendering::ImageDimensions dim{ 1, 1, 1 };
				unsigned char* buf = dim.pxnew();
				buf[0] = 255;
				rendering::Image image(buf, dim);
				white1x1_1 = move_shared(rendering::load_bindless_texture_2d(image));
				white1x1_1->set_and_use_handle();
			}
		}

		void unload()
		{
			white1x1_1.reset();
		}
	}
}
