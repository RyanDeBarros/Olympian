#include "Textures.h"

#include <stb/stb_image.h>

oly::rendering::Sampler::Sampler()
{
	glCreateSamplers(1, &id);
}

oly::rendering::Sampler::Sampler(Sampler&& other) noexcept
	: id(other.id)
{
	other.id = 0;
}

oly::rendering::Sampler::~Sampler()
{
	glDeleteSamplers(1, &id);
}

oly::rendering::Sampler& oly::rendering::Sampler::operator=(Sampler&& other) noexcept
{
	if (this != &other)
	{
		glDeleteSamplers(1, &id);
		id = other.id;
		other.id = 0;
	}
	return *this;
}

void oly::rendering::Sampler::set_parameter_i(GLenum param, GLint value) const
{
	glSamplerParameteri(id, param, value);
}

void oly::rendering::Sampler::set_parameter_iv(GLenum param, const GLint* values) const
{
	glSamplerParameteriv(id, param, values);
}

void oly::rendering::Sampler::set_parameter_f(GLenum param, GLfloat value) const
{
	glSamplerParameterf(id, param, value);
}

void oly::rendering::Sampler::set_parameter_fv(GLenum param, const GLfloat* values) const
{
	glSamplerParameterfv(id, param, values);
}

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

oly::rendering::BindlessTexture::BindlessTexture()
{
	glGenTextures(1, &id);
}

oly::rendering::BindlessTexture::BindlessTexture(GLenum target)
{
	glCreateTextures(target, 1, &id);
}

oly::rendering::BindlessTexture::BindlessTexture(BindlessTexture&& other) noexcept
	: id(other.id), handle(other.handle)
{
	other.id = 0;
	other.handle = 0;
}

oly::rendering::BindlessTexture::~BindlessTexture()
{
	disuse_handle();
	glDeleteTextures(1, &id);
}

oly::rendering::BindlessTexture& oly::rendering::BindlessTexture::operator=(BindlessTexture&& other) noexcept
{
	if (this != &other)
	{
		disuse_handle();
		glDeleteTextures(1, &id);
		id = other.id;
		other.id = 0;
		handle = other.handle;
		other.handle = 0;
	}
	return *this;
}

void oly::rendering::BindlessTexture::set_handle()
{
	disuse_handle();
	if (!_tex_handle)
		_tex_handle = glGetTextureHandleARB(id);
	handle = _tex_handle;
}

void oly::rendering::BindlessTexture::set_handle(GLuint64 sampler)
{
	disuse_handle();
	for (auto iter = _sampler_handles.begin(); iter != _sampler_handles.end(); ++iter)
	{
		if (iter->first == sampler)
		{
			handle = iter->second;
			return;
		}
	}
	handle = glGetTextureSamplerHandleARB(id, sampler);
	_sampler_handles.push_back({ sampler, handle });
}

void oly::rendering::BindlessTexture::use_handle() const
{
	if (handle)
		glMakeTextureHandleResidentARB(handle);
}

void oly::rendering::BindlessTexture::disuse_handle() const
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

void oly::rendering::tex::image_2d(GLenum target, ImageDimensions dim, void* buf, GLenum data_type)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment(dim.cpp));
	glTexImage2D(target, 0, internal_format(dim.cpp), dim.w, dim.h, 0, format(dim.cpp), data_type, buf);
}

oly::rendering::TextureRes oly::rendering::load_static_texture_2d(const char* filename, ImageDimensions& dim)
{
	// TODO PixelBuffer that encapsulates unsigned char* image and ImageDimensions dim
	unsigned char* image = stbi_load(filename, &dim.w, &dim.h, &dim.cpp, 4);
	TextureRes texture = std::make_shared<oly::rendering::Texture>(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, *texture);
	tex::image_2d(GL_TEXTURE_2D, dim, image, GL_UNSIGNED_BYTE);
	stbi_image_free(image);
	return texture;
}

oly::rendering::BindlessTextureRes oly::rendering::load_static_bindless_texture_2d(const char* filename, ImageDimensions& dim)
{
	unsigned char* image = stbi_load(filename, &dim.w, &dim.h, &dim.cpp, 4);
	BindlessTextureRes texture = std::make_shared<oly::rendering::BindlessTexture>(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, *texture);
	tex::image_2d(GL_TEXTURE_2D, dim, image, GL_UNSIGNED_BYTE);
	stbi_image_free(image);
	return texture;
}
