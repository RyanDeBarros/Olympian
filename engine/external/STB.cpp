#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#define STBI_MALLOC(sz) (new unsigned char[sz])
#define STBI_FREE(p) (delete[] (unsigned char*)p)
inline static void* stbi_realloc_sized(void* p, auto oldsz, auto newsz)
{
	unsigned char* newp = new unsigned char[newsz];
	if (p)
	{
		memcpy(newp, p, std::min(size_t(oldsz), size_t(newsz)));
		delete[](unsigned char*)p;
	}
	return newp;
}
#define STBI_REALLOC_SIZED(p, oldsz, newsz) stbi_realloc_sized(p, oldsz, newsz)
#include <stb/stb_image.h>
#pragma warning(push)
#pragma warning(disable : 4996)
#include <stb/stb_image_write.h>
#pragma warning(pop)
#include <stb/stb_truetype.h>
