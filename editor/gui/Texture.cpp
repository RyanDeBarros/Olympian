#include "Texture.h"

#include "core/Errors.h"

#include "external/STB.h"
#include "external/NSVG.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace oly::editor
{
	struct TextureConstructor
	{
		std::filesystem::path filepath;
		float scale;
		int frame_index;
		GLenum min_filter, mag_filter;

		bool operator==(const TextureConstructor&) const = default;
	};

	struct TextureConstructorHash
	{
		size_t operator()(const TextureConstructor& tc) const
		{
			// TODO v8 put hash combine in util/Hash.h
			size_t h = std::hash<std::string>{}(tc.filepath.generic_string());
			h ^= std::hash<float>{}(tc.scale) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
			h ^= std::hash<GLenum>{}(tc.min_filter) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
			h ^= std::hash<GLenum>{}(tc.mag_filter) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
			return h;
		}
	};

	static TextureIDRef GetTextureIDRef(const std::string_view filepath, float scale, int frame_index, GLenum min_filter, GLenum mag_filter, bool& initialized)
	{
		static std::unordered_map<TextureConstructor, TextureIDRef, TextureConstructorHash> TEXTURE_REGISTRY;

		TextureConstructor ctor{
			.filepath = filepath,
			.scale = scale,
			.frame_index = frame_index,
			.min_filter = min_filter,
			.mag_filter = mag_filter
		};

		auto it = TEXTURE_REGISTRY.find(ctor);
		if (it != TEXTURE_REGISTRY.end())
		{
			initialized = true;
			return it->second;
		}
		else
		{
			initialized = false;
			auto ref = std::make_shared<TextureID>();
			TEXTURE_REGISTRY[ctor] = ref;
			return ref;
		}
	}

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

	RasterTexture::RasterTexture(const std::string_view filepath, GLenum min_filter, GLenum mag_filter)
	{
		bool initialized;
		id = GetTextureIDRef(filepath, 0.f, -1, min_filter, mag_filter, initialized);
		
		if (!initialized)
		{
			int channels;
			unsigned char* data = stbi_load(filepath.data(), &width, &height, &channels, 0);
			if (!data || width <= 0 || height <= 0 || channels <= 0)
			{
				stbi_image_free(data);
				BreakoutError::Throw(("Cannot load raster image from file: " + std::string(filepath)).c_str());
			}

			glBindTexture(GL_TEXTURE_2D, id->ID());
			glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat(channels), width, height, 0, Format(channels), GL_UNSIGNED_BYTE, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
			glBindTexture(GL_TEXTURE_2D, 0);

			stbi_image_free(data);
		}
	}

	GLuint RasterTexture::ID() const
	{
		return id->ID();
	}

	float RasterTexture::Width() const
	{
		return width;
	}

	float RasterTexture::Height() const
	{
		return height;
	}

	GIFTexture::GIFTexture(const std::string_view filepath, GLenum min_filter, GLenum mag_filter)
	{
		std::ifstream file(filepath.data(), std::ios::binary | std::ios::ate);
		if (!file.is_open() || file.fail())
			BreakoutError::Throw(("Cannot open file for reading: " + std::string(filepath)).c_str());

		std::streamsize size = file.tellg();
		if (size <= 0)
			BreakoutError::Throw(("File has bad size (bytes): " + std::string(filepath)).c_str());

		file.seekg(0, std::ios::beg);

		std::vector<unsigned char> pixels(size);
		file.read(reinterpret_cast<char*>(pixels.data()), size);

		int channels;
		int* delay_arr;
		int frames;
		unsigned char* data = stbi_load_gif_from_memory(pixels.data(), static_cast<int>(pixels.size()), &delay_arr, &width, &height, &frames, &channels, 0);
		if (!data || !delay_arr || frames <= 0 || width <= 0 || height <= 0 || channels <= 0)
		{
			stbi_image_free(delay_arr);
			stbi_image_free(data);
			BreakoutError::Throw(("Cannot load gif from file: " + std::string(filepath)).c_str());
		}

		const int area = sizeof(unsigned char) * width * height * channels;
		delays.resize(frames);
		for (int i = 0; i < frames; ++i)
			delays[i] = 0.01f * delay_arr[i];

		ids.reserve(frames);
		for (size_t i = 0; i < frames; ++i)
		{
			bool initialized;
			ids.push_back(GetTextureIDRef(filepath, 0.f, i, min_filter, mag_filter, initialized));
			if (!initialized)
			{
				glBindTexture(GL_TEXTURE_2D, ids[i]->ID());
				glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat(channels), width, height, 0, Format(channels), GL_UNSIGNED_BYTE, data + i * area);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
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
		return ids[index]->ID();
	}

	float GIFTexture::Width() const
	{
		return width;
	}
	
	float GIFTexture::Height() const
	{
		return height;
	}

	SVGTexture::SVGTexture(const std::string_view filepath, float scale, GLenum min_filter, GLenum mag_filter)
	{
		bool initialized;
		id = GetTextureIDRef(filepath, scale, -1, min_filter, mag_filter, initialized);
		if (!initialized)
		{
			NSVGimage* image = nsvgParseFromFile(filepath.data(), "px", 96.f);
			if (!image)
				BreakoutError::Throw(("Cannot parse svg from file: " + std::string(filepath)).c_str());

			NSVGrasterizer* rasterizer = nsvgCreateRasterizer();
			if (!rasterizer)
			{
				nsvgDelete(image);
				BreakoutError::Throw("Failed to create svg rasterizer");
			}

			width = std::max(static_cast<int>(scale * image->width), 1);
			height = std::max(static_cast<int>(scale * image->height), 1);
			const int channels = 4;
			const int stride = width * channels;
			unsigned char* data = new (std::nothrow) unsigned char[stride * height];
			if (!data)
			{
				nsvgDeleteRasterizer(rasterizer);
				nsvgDelete(image);
				BreakoutError::Throw("Bad alloc");
			}

			nsvgRasterize(rasterizer, image, 0.0f, 0.0f, scale, data, width, height, stride);

			glBindTexture(GL_TEXTURE_2D, id->ID());
			glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat(channels), width, height, 0, Format(channels), GL_UNSIGNED_BYTE, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
			glBindTexture(GL_TEXTURE_2D, 0);

			delete[] data;
			nsvgDeleteRasterizer(rasterizer);
			nsvgDelete(image);
		}
	}
	
	GLuint SVGTexture::ID() const
	{
		return id->ID();
	}

	float SVGTexture::Width() const
	{
		return width * preview_scale;
	}

	float SVGTexture::Height() const
	{
		return height * preview_scale;
	}

	bool Texture::Empty() const
	{
		return std::get_if<std::monostate>(&v);
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

	void Texture::LoadGeneric(const std::string_view filepath)
	{
		if (filepath.ends_with(".svg"))
			v = SVGTexture(filepath);
		else if (filepath.ends_with(".gif"))
			v = GIFTexture(filepath);
		else
			v = RasterTexture(filepath);
	}
}
