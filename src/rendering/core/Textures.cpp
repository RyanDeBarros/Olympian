#include "Textures.h"

#include <stb/stb_image.h>

oly::rendering::Texture::Texture()
{
	glGenTextures(1, &id);
}

oly::rendering::Texture::Texture(GLenum target)
{
	glCreateTextures(target, 1, &id);
}

oly::rendering::Texture::Texture(Texture&& other) noexcept
	: id(other.id)
{
	other.id = 0;
}

oly::rendering::Texture::~Texture()
{
	glDeleteTextures(1, &id);
}

oly::rendering::Texture& oly::rendering::Texture::operator=(Texture&& other) noexcept
{
	if (this != &other)
	{
		glDeleteTextures(1, &id);
		id = other.id;
		other.id = 0;
	}
	return *this;
}

std::vector<std::shared_ptr<oly::rendering::Texture>> oly::rendering::gen_bulk_textures(GLsizei n)
{
	std::vector<GLuint> textures;
	textures.resize(n);
	glGenTextures(n, textures.data());
	std::vector<TextureRes> wrapped_textures;
	wrapped_textures.reserve(n);
	for (GLuint id : textures)
		wrapped_textures.push_back(Texture::from_id(id));
	return wrapped_textures;
}

std::vector<std::shared_ptr<oly::rendering::Texture>> oly::rendering::create_bulk_textures(GLsizei n, GLenum target)
{
	std::vector<GLuint> textures;
	textures.resize(n);
	glCreateTextures(target, n, textures.data());
	std::vector<TextureRes> wrapped_textures;
	wrapped_textures.reserve(n);
	for (GLuint id : textures)
		wrapped_textures.push_back(Texture::from_id(id));
	return wrapped_textures;
}

oly::rendering::BindlessTextureHandle::~BindlessTextureHandle()
{
	disuse();
}

void oly::rendering::BindlessTextureHandle::use(GLuint texture) const
{
	disuse();
	handle = glGetTextureHandleARB(texture);
	glMakeTextureHandleResidentARB(handle);
}

void oly::rendering::BindlessTextureHandle::disuse() const
{
	if (handle)
		glMakeTextureHandleNonResidentARB(handle);
}

GLenum oly::rendering::tex::internal_format(int cpp)
{
	return cpp == 1 ? GL_R8
		: cpp == 2 ? GL_RG8
		: cpp == 3 ? GL_RGB8
		: GL_RGBA8;
}

GLenum oly::rendering::tex::format(int cpp)
{
	return cpp == 1 ? GL_RED
		: cpp == 2 ? GL_RG
		: cpp == 3 ? GL_RGB
		: GL_RGBA;
}

GLint oly::rendering::tex::alignment(int cpp)
{
	return cpp == 1 ? 1
		: cpp == 2 ? 2
		: cpp == 3 ? 1
		: 4;
}

oly::rendering::TextureRes oly::rendering::load_static_texture_2d(const char* filename, ImageDimensions& dim)
{
	unsigned char* image = stbi_load(filename, &dim.w, &dim.h, &dim.cpp, 4);
	auto texture = std::make_shared<oly::rendering::Texture>(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, oly::rendering::tex::alignment(dim.cpp));
	glTexImage2D(GL_TEXTURE_2D, 0, oly::rendering::tex::internal_format(dim.cpp), dim.w, dim.h, 0, oly::rendering::tex::format(dim.cpp), GL_UNSIGNED_BYTE, image);
	stbi_image_free(image);
	return texture;
}
