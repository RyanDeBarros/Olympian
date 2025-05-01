#include "PixelBuffers.h"

#include "Assert.h"

namespace oly
{
	void flip_pixel_buffer(unsigned char* buf, int w, int h, int cpp)
	{
		OLY_ASSERT(w > 0 && h > 0 && cpp > 0);
		int stride = w * cpp;
		unsigned char* temp = new unsigned char[stride];
		for (int row = 0; row < h / 2; ++row)
		{
			unsigned char* lower = buf + row * stride;
			unsigned char* upper = buf + (h - 1 - row) * stride;
			memcpy(temp, lower, stride);
			memcpy(lower, upper, stride);
			memcpy(upper, temp, stride);
		}
		delete[] temp;
	}
}
