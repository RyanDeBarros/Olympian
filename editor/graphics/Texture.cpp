#include "Texture.h"

#include "external/STB.h"
#include "external/NSVG.h"

#include <fstream>
#include <vector>

namespace oly::editor
{
	TextureID::TextureID()
	{
		glGenTextures(1, &_id);
	}

	TextureID::TextureID(TextureID&& other) noexcept
		: _id(other._id)
	{
		other._id = 0;
	}
	
	TextureID::~TextureID()
	{
		glDeleteTextures(1, &_id);
	}
	
	TextureID& TextureID::operator=(TextureID&& other) noexcept
	{
		if (this != &other)
		{
			glDeleteTextures(1, &_id);
			_id = other._id;
			other._id = 0;
		}
		return *this;
	}

	GLuint TextureID::ID() const
	{
		return _id;
	}

	// TODO v7 error handling - be mindful of memory leaks with early exits

	static GLenum InternalFormat(int channels)
	{
		return channels == 1 ? GL_R8
			: channels == 2 ? GL_RG8
			: channels == 3 ? GL_RGB8
			: GL_RGBA8;
	}

	static GLenum Format(int channels)
	{
		return channels == 1 ? GL_RED
			: channels == 2 ? GL_RG
			: channels == 3 ? GL_RGB
			: GL_RGBA;
	}

	RasterTexture::RasterTexture(const char* filepath, GLenum min_filter, GLenum mag_filter)
	{
		int channels;
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 0);
		glBindTexture(GL_TEXTURE_2D, id.ID());
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat(channels), width, height, 0, Format(channels), GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(data);
	}

	GLuint RasterTexture::ID() const
	{
		return id.ID();
	}

	float RasterTexture::Width() const
	{
		return width;
	}

	float RasterTexture::Height() const
	{
		return height;
	}

	GIFTexture::GIFTexture(const char* filepath, GLenum min_filter, GLenum mag_filter)
	{
		std::ifstream file(filepath, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<unsigned char> pixels(size);
		file.read(reinterpret_cast<char*>(pixels.data()), size);

		int channels;
		int* delay_arr;
		int frames;
		unsigned char* data = stbi_load_gif_from_memory(pixels.data(), static_cast<int>(pixels.size()), &delay_arr, &width, &height, &frames, &channels, 0);

		ids.resize(frames);
		delays.resize(frames);

		const int area = sizeof(unsigned char) * width * height * channels;
		for (int i = 0; i < frames; ++i)
		{
			delays[i] = 0.01f * delay_arr[i];

			glBindTexture(GL_TEXTURE_2D, ids[i].ID());
			glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat(channels), width, height, 0, Format(channels), GL_UNSIGNED_BYTE, data + i * area);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		stbi_image_free(delay_arr);
		stbi_image_free(data);
	}

	void GIFTexture::Update(float delta_seconds)
	{
		const int frames = static_cast<int>(delays.size());
		timer += delta_seconds * speed;
		while (timer > std::max(delays[index], 0.001f))
		{
			timer -= std::max(delays[index], 0.001f);
			index = (index + 1) % frames;
		}
		while (timer < 0.f)
		{
			timer += std::max(delays[index], 0.001f);
			index = (index - 1 + frames) % frames;
		}
	}

	GLuint GIFTexture::ID() const
	{
		return ids[index].ID();
	}

	float GIFTexture::Width() const
	{
		return width;
	}
	
	float GIFTexture::Height() const
	{
		return height;
	}

	SVGTexture::SVGTexture(const char* filepath, float scale, GLenum min_filter, GLenum mag_filter)
	{
		NSVGimage* image = nsvgParseFromFile(filepath, "px", 96.f);
		NSVGrasterizer* rasterizer = nsvgCreateRasterizer();

		width = std::max(static_cast<int>(scale * image->width), 1);
		height = std::max(static_cast<int>(scale * image->height), 1);
		const int channels = 4;
		const int stride = width * channels;
		unsigned char* data = new unsigned char[stride * height];
		nsvgRasterize(rasterizer, image, 0.0f, 0.0f, scale, data, width, height, stride);

		glBindTexture(GL_TEXTURE_2D, id.ID());
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat(channels), width, height, 0, Format(channels), GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		glBindTexture(GL_TEXTURE_2D, 0);

		delete[] data;

		nsvgDeleteRasterizer(rasterizer);
		nsvgDelete(image);
	}
	
	GLuint SVGTexture::ID() const
	{
		return id.ID();
	}

	float SVGTexture::Width() const
	{
		return width * preview_scale;
	}

	float SVGTexture::Height() const
	{
		return height * preview_scale;
	}

	GIFTexture* Texture::GetGIF()
	{
		return std::get_if<GIFTexture>(&v);
	}

	SVGTexture* Texture::GetSVG()
	{
		return std::get_if<SVGTexture>(&v);
	}

	GLuint Texture::ID() const
	{
		return std::visit([](const auto& t) -> GLuint {
			if constexpr (std::is_same_v<std::decay_t<decltype(t)>, std::monostate>)
				return 0;
			else
				return t.ID();
		}, v);
	}

	float Texture::Width() const
	{
		return std::visit([](const auto& t) {
			if constexpr (std::is_same_v<std::decay_t<decltype(t)>, std::monostate>)
				return 0.f;
			else
				return t.Width();
		}, v);
	}

	float Texture::Height() const
	{
		return std::visit([](const auto& t) {
			if constexpr (std::is_same_v<std::decay_t<decltype(t)>, std::monostate>)
				return 0.f;
			else
				return t.Height();
		}, v);
	}

	ImVec2 Texture::Size() const
	{
		return ImVec2(Width(), Height());
	}
}
