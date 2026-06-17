#include "Texture.h"

#include "core/Errors.h"

#include "external/STB.h"
#include "external/NSVG.h"

#include "util/Hash.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace oly::editor
{
	struct RasterTextureConstructor
	{
		std::filesystem::path filepath;
		GLenum min_filter, mag_filter;
		bool generate_mipmaps;

		bool operator==(const RasterTextureConstructor&) const = default;
	};

	struct GIFTextureConstructor
	{
		std::filesystem::path filepath;
		GLenum min_filter, mag_filter;
		bool generate_mipmaps;

		bool operator==(const GIFTextureConstructor&) const = default;
	};

	struct SVGTextureConstructor
	{
		std::filesystem::path filepath;
		GLenum min_filter, mag_filter;
		float scale;
		bool generate_mipmaps;

		bool operator==(const SVGTextureConstructor&) const = default;
	};

	struct TextureConstructorHash
	{
		size_t operator()(const RasterTextureConstructor& tc) const
		{
			return detail::Hasher()
				.with(tc.filepath)
				.with(tc.min_filter)
				.with(tc.mag_filter)
				.with(tc.generate_mipmaps);
		}

		size_t operator()(const GIFTextureConstructor& tc) const
		{
			return detail::Hasher()
				.with(tc.filepath)
				.with(tc.min_filter)
				.with(tc.mag_filter)
				.with(tc.generate_mipmaps);
		}

		size_t operator()(const SVGTextureConstructor& tc) const
		{
			return detail::Hasher()
				.with(tc.filepath)
				.with(tc.min_filter)
				.with(tc.mag_filter)
				.with(tc.scale)
				.with(tc.generate_mipmaps);
		}
	};

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

	RasterTexture::RasterTexture(const std::string_view filepath, GLenum min_filter, GLenum mag_filter, bool generate_mipmaps)
	{
		int channels;
		unsigned char* data = stbi_load(filepath.data(), &width, &height, &channels, 0);
		if (!data || width <= 0 || height <= 0 || channels <= 0)
		{
			stbi_image_free(data);
			BreakoutError::Throw(("Cannot load raster image from file: " + std::string(filepath)).c_str());
		}

		glBindTexture(GL_TEXTURE_2D, id.ID());
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat(channels), width, height, 0, Format(channels), GL_UNSIGNED_BYTE, data);
		if (generate_mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(data);
	}

	static std::unordered_map<RasterTextureConstructor, std::weak_ptr<RasterTexture>, TextureConstructorHash> RASTER_TEXTURES;

	std::shared_ptr<RasterTexture> RasterTexture::Load(const std::string_view filepath, std::optional<GLenum> min_filter, std::optional<GLenum> mag_filter, bool generate_mipmaps)
	{
		RasterTextureConstructor ctor{
			.filepath = filepath,
			.min_filter = min_filter ? *min_filter : GL_NEAREST,
			.mag_filter = mag_filter ? *mag_filter : GL_NEAREST,
			.generate_mipmaps = generate_mipmaps
		};

		auto it = RASTER_TEXTURES.find(ctor);
		if (it != RASTER_TEXTURES.end() && !it->second.expired())
			return it->second.lock();

		std::shared_ptr<RasterTexture> sp(new RasterTexture(filepath, ctor.min_filter, ctor.mag_filter, ctor.generate_mipmaps));
		RASTER_TEXTURES[ctor] = sp;
		return sp;
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

	GIFTexture::GIFTexture(const std::string_view filepath, GLenum min_filter, GLenum mag_filter, bool generate_mipmaps)
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

		ids.resize(frames);
		for (size_t i = 0; i < frames; ++i)
		{
			glBindTexture(GL_TEXTURE_2D, ids[i].ID());
			glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat(channels), width, height, 0, Format(channels), GL_UNSIGNED_BYTE, data + i * area);
			if (generate_mipmaps)
				glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		stbi_image_free(delay_arr);
		stbi_image_free(data);
	}

	static std::unordered_map<GIFTextureConstructor, std::weak_ptr<GIFTexture>, TextureConstructorHash> GIF_TEXTURES;

	std::shared_ptr<GIFTexture> GIFTexture::Load(const std::string_view filepath, std::optional<GLenum> min_filter, std::optional<GLenum> mag_filter, bool generate_mipmaps)
	{
		GIFTextureConstructor ctor{
			.filepath = filepath,
			.min_filter = min_filter ? *min_filter : GL_NEAREST,
			.mag_filter = mag_filter ? *mag_filter : GL_NEAREST,
			.generate_mipmaps = generate_mipmaps
		};

		auto it = GIF_TEXTURES.find(ctor);
		if (it != GIF_TEXTURES.end() && !it->second.expired())
			return it->second.lock();

		std::shared_ptr<GIFTexture> sp(new GIFTexture(filepath, ctor.min_filter, ctor.mag_filter, ctor.generate_mipmaps));
		GIF_TEXTURES.emplace(ctor, sp);
		return sp;
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

	SVGTexture::SVGTexture(const std::string_view filepath, float scale, GLenum min_filter, GLenum mag_filter, bool generate_mipmaps)
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

		glBindTexture(GL_TEXTURE_2D, id.ID());
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat(channels), width, height, 0, Format(channels), GL_UNSIGNED_BYTE, data);
		if (generate_mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		glBindTexture(GL_TEXTURE_2D, 0);

		delete[] data;
		nsvgDeleteRasterizer(rasterizer);
		nsvgDelete(image);
	}

	static std::unordered_map<SVGTextureConstructor, std::weak_ptr<SVGTexture>, TextureConstructorHash> SVG_TEXTURES;

	std::shared_ptr<SVGTexture> SVGTexture::Load(const std::string_view filepath, float scale, std::optional<GLenum> min_filter, std::optional<GLenum> mag_filter, bool generate_mipmaps)
	{
		SVGTextureConstructor ctor{
			.filepath = filepath,
			.min_filter = min_filter ? *min_filter : GL_LINEAR,
			.mag_filter = mag_filter ? *mag_filter : GL_LINEAR,
			.scale = scale,
			.generate_mipmaps = generate_mipmaps
		};

		auto it = SVG_TEXTURES.find(ctor);
		if (it != SVG_TEXTURES.end() && !it->second.expired())
			return it->second.lock();

		std::shared_ptr<SVGTexture> sp(new SVGTexture(filepath, ctor.scale, ctor.min_filter, ctor.mag_filter, ctor.generate_mipmaps));
		SVG_TEXTURES.emplace(ctor, sp);
		return sp;
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

	bool Texture::Empty() const
	{
		return std::get_if<std::monostate>(&v);
	}

	GIFTexture* Texture::GetGIF()
	{
		auto ptr = std::get_if<std::shared_ptr<GIFTexture>>(&v);
		return ptr ? ptr->get() : nullptr;
	}

	SVGTexture* Texture::GetSVG()
	{
		auto ptr = std::get_if<std::shared_ptr<SVGTexture>>(&v);
		return ptr ? ptr->get() : nullptr;
	}

	GLuint Texture::ID() const
	{
		return std::visit([](const auto& t) -> GLuint {
			if constexpr (std::is_same_v<std::decay_t<decltype(t)>, std::monostate>)
				return 0;
			else
				return t->ID();
		}, v);
	}

	float Texture::Width() const
	{
		return std::visit([](const auto& t) {
			if constexpr (std::is_same_v<std::decay_t<decltype(t)>, std::monostate>)
				return 0.f;
			else
				return t->Width();
		}, v);
	}

	float Texture::Height() const
	{
		return std::visit([](const auto& t) {
			if constexpr (std::is_same_v<std::decay_t<decltype(t)>, std::monostate>)
				return 0.f;
			else
				return t->Height();
		}, v);
	}

	ImVec2 Texture::Size() const
	{
		return ImVec2(Width(), Height());
	}

	Texture Texture::LoadGeneric(const std::string_view filepath, std::optional<GLenum> min_filter, std::optional<GLenum> mag_filter, float scale, bool generate_mipmaps)
	{
		Texture t;
		if (filepath.ends_with(".svg"))
			t.v = SVGTexture::Load(filepath, scale, min_filter, mag_filter, generate_mipmaps);
		else if (filepath.ends_with(".gif"))
			t.v = GIFTexture::Load(filepath, min_filter, mag_filter, generate_mipmaps);
		else
			t.v = RasterTexture::Load(filepath, min_filter, mag_filter, generate_mipmaps);
		return t;
	}

	void Texture::Update()
	{
		static const auto Prune = [](auto& textures) {
			for (auto it = textures.begin(); it != textures.end();)
			{
				if (it->second.expired())
					it = textures.erase(it);
				else
					++it;
			}
		};
		
		Prune(RASTER_TEXTURES);
		Prune(GIF_TEXTURES);
		Prune(SVG_TEXTURES);
	}
}
