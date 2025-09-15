#include "Textures.h"

#include "core/types/Meta.h"

#include "core/base/Assert.h"

namespace oly::graphics::textures
{
	// TODO v4 load these from actual image files in internal/textures. Allow assets to access them using a special filepath format - instead of RES://, use some other prefix. Also, when loading textures in general, use these prefixes.
	BindlessTextureRef white1x1;

	void internal::load()
	{
		{
			ImageDimensions dim{ .w = 1, .h = 1, .cpp = 4 };
			unsigned char* buf = dim.pxnew();
			memcpy(buf, std::array<unsigned char, 4>{ 255, 255, 255, 255 }.data(), 4 * sizeof(unsigned char));
			white1x1 = graphics::BindlessTextureRef(load_bindless_texture_2d(Image(buf, dim)));
			white1x1->set_and_use_handle();
		}
	}

	void internal::unload()
	{
		white1x1.invalidate();
	}

	BindlessTextureRef mod2x2(glm::vec<4, unsigned char> c1, glm::vec<4, unsigned char> c2, glm::vec<4, unsigned char> c3, glm::vec<4, unsigned char> c4)
	{
		ImageDimensions dim{ .w = 2, .h = 2, .cpp = 4 };
		unsigned char* buf = dim.pxnew();
		memcpy(buf + 4 * 0, glm::value_ptr(c1), 4 * sizeof(unsigned char));
		memcpy(buf + 4 * 1, glm::value_ptr(c2), 4 * sizeof(unsigned char));
		memcpy(buf + 4 * 2, glm::value_ptr(c4), 4 * sizeof(unsigned char));
		memcpy(buf + 4 * 3, glm::value_ptr(c3), 4 * sizeof(unsigned char));
		auto tex = graphics::BindlessTextureRef(load_bindless_texture_2d(Image(buf, dim)));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		tex->set_and_use_handle();
		return tex;
	}

	// TODO v4 maybe float version can send a texture that uses floats instead of unsigned char for pixel data type.
	BindlessTextureRef mod2x2(glm::vec4 c1, glm::vec4 c2, glm::vec4 c3, glm::vec4 c4)
	{
		return mod2x2(glm::vec<4, unsigned char>(round(c1 * 255.0f)),
			glm::vec<4, unsigned char>(round(c2 * 255.0f)),
			glm::vec<4, unsigned char>(round(c3 * 255.0f)),
			glm::vec<4, unsigned char>(round(c4 * 255.0f)));
	}
}
