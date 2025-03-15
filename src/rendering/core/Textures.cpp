#include "Textures.h"

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
	std::vector<std::shared_ptr<Texture>> wrapped_textures;
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
	std::vector<std::shared_ptr<Texture>> wrapped_textures;
	wrapped_textures.reserve(n);
	for (GLuint id : textures)
		wrapped_textures.push_back(Texture::from_id(id));
	return wrapped_textures;
}
