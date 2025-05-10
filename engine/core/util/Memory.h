#pragma once

#include <memory>

namespace oly
{
	template<typename T>
	inline void swap_adjacent_subbuffers(T* buf, size_t offset1, size_t count1, size_t offset2, size_t count2)
	{
		if (count1 < count2)
		{
			std::swap(count1, count2);
			std::swap(offset1, offset2);
		}

		T* temp = new T[count1];
		memcpy(temp, buf + offset1, count1 * sizeof(T));
		if (offset1 < offset2)
		{
			memcpy(buf + offset1, buf + offset2, count2 * sizeof(T));
			memcpy(buf + offset2 + count2 - count1, temp, count1 * sizeof(T));
		}
		else
		{
			memcpy(buf + offset1 + count1 - count2, buf + offset2, count2 * sizeof(T));
			memcpy(buf + offset2, temp, count1 * sizeof(T));
		}
		delete[] temp;
	}

	bool flip_pixel_buffer(unsigned char* buf, int w, int h, int cpp)
	{
		if (w <= 0 || h <= 0 || cpp <= 0)
			return false;
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
		return true;
	}
}
