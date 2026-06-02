#pragma once

#include "external/GL.h"

#include <variant>
#include <vector>

namespace oly::editor
{
	class TextureID
	{
		GLuint _id = 0;

	public:
		TextureID();
		TextureID(const TextureID&) = delete;
		TextureID(TextureID&&) noexcept;
		~TextureID();
		TextureID& operator=(TextureID&&) noexcept;

		GLuint ID() const;
	};

	struct RasterTexture
	{
		TextureID id;
		int width = 0, height = 0;
		
		RasterTexture(const char* filepath, GLenum min_filter = GL_LINEAR, GLenum mag_filter = GL_LINEAR);
		GLuint ID() const;
	};

	struct GIFTexture
	{
		std::vector<TextureID> ids;
		std::vector<float> delays;
		int width = 0, height = 0;
		int index = 0;
		float timer = 0.f;

		GIFTexture(const char* filepath, GLenum min_filter = GL_LINEAR, GLenum mag_filter = GL_LINEAR);

		void Update(float delta_seconds);
		GLuint ID() const;
	};

	struct SVGTexture
	{
		TextureID id;
		int width = 0, height = 0;

		SVGTexture(const char* filepath, float scale = 1.f, GLenum min_filter = GL_LINEAR, GLenum mag_filter = GL_LINEAR);
		GLuint ID() const;
	};

	struct Texture
	{
		using Variant = std::variant<std::monostate, RasterTexture, GIFTexture, SVGTexture>;
		Variant v;

		void Update(float delta_seconds);
		GLuint ID() const;
		int Width() const;
		int Height() const;
	};
}
