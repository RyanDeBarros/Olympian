#pragma once

#include "external/GL.h"

#include <string_view>
#include <variant>
#include <vector>

#include <imgui.h>

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
		
		RasterTexture(const std::string_view filepath, GLenum min_filter = GL_NEAREST, GLenum mag_filter = GL_NEAREST);
		GLuint ID() const;
		float Width() const;
		float Height() const;
	};

	struct GIFTexture
	{
		std::vector<TextureID> ids;
		std::vector<float> delays;
		int width = 0, height = 0;
		int index = 0;
		float timer = 0.f;
		float speed = 1.f;

		GIFTexture(const std::string_view filepath, GLenum min_filter = GL_NEAREST, GLenum mag_filter = GL_NEAREST);

		void Update(float delta_seconds);
		GLuint ID() const;
		float Width() const;
		float Height() const;
	};

	struct SVGTexture
	{
		TextureID id;
		int width = 0, height = 0;
		float preview_scale = 1.f;

		SVGTexture(const std::string_view filepath, float scale = 1.f, GLenum min_filter = GL_LINEAR, GLenum mag_filter = GL_LINEAR);
		GLuint ID() const;
		float Width() const;
		float Height() const;
	};

	struct Texture
	{
		using Variant = std::variant<std::monostate, RasterTexture, GIFTexture, SVGTexture>;
		Variant v;

		bool Empty() const;
		GIFTexture* GetGIF();
		SVGTexture* GetSVG();
		GLuint ID() const;
		float Width() const;
		float Height() const;
		ImVec2 Size() const;

		void LoadGeneric(const std::string_view filepath);
	};
}
