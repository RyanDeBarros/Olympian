#pragma once

#include "external/GL.h"

#include <memory>
#include <optional>
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
		
	private:
		RasterTexture(const std::string_view filepath, GLenum min_filter, GLenum mag_filter, bool generate_mipmaps);

	public:
		static std::shared_ptr<RasterTexture> Load(const std::string_view filepath, std::optional<GLenum> min_filter = std::nullopt,
			std::optional<GLenum> mag_filter = std::nullopt, bool generate_mipmaps = false);

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

	private:
		GIFTexture(const std::string_view filepath, GLenum min_filter, GLenum mag_filter, bool generate_mipmaps);

	public:
		static std::shared_ptr<GIFTexture> Load(const std::string_view filepath, std::optional<GLenum> min_filter = std::nullopt,
			std::optional<GLenum> mag_filter = std::nullopt, bool generate_mipmaps = false);

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

	private:
		SVGTexture(const std::string_view filepath, float scale, GLenum min_filter, GLenum mag_filter, bool generate_mipmaps);

	public:
		static std::shared_ptr<SVGTexture> Load(const std::string_view filepath, float scale = 1.f, std::optional<GLenum> min_filter = std::nullopt,
			std::optional<GLenum> mag_filter = std::nullopt, bool generate_mipmaps = false);

		GLuint ID() const;
		float Width() const;
		float Height() const;
	};

	struct Texture
	{
		using Variant = std::variant<std::monostate, std::shared_ptr<RasterTexture>, std::shared_ptr<GIFTexture>, std::shared_ptr<SVGTexture>>;
		Variant v;

		bool Empty() const;
		GIFTexture* GetGIF();
		SVGTexture* GetSVG();
		GLuint ID() const;
		float Width() const;
		float Height() const;
		ImVec2 Size() const;

		static Texture LoadGeneric(const std::string_view filepath, std::optional<GLenum> min_filter = std::nullopt,
			std::optional<GLenum> mag_filter = std::nullopt, float scale = 1.f, bool generate_mipmaps = false);
	};
}
