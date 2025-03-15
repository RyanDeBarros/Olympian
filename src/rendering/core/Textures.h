#pragma once

#include <GL/glew.h>

#include <vector>
#include <array>
#include <memory>

namespace oly
{
	namespace rendering
	{
		class Texture;
		typedef std::shared_ptr<Texture> TextureRes;

		class Texture
		{
			GLuint id;

			struct static_id { GLuint id; };
			Texture(static_id sid) : id(sid.id) {}

		public:
			Texture();
			Texture(GLenum target);
			Texture(const Texture&) = delete;
			Texture(Texture&&) noexcept;
			~Texture();
			Texture& operator=(Texture&&) noexcept;

			operator GLuint () const { return id; }
			static TextureRes from_id(GLuint id) { return TextureRes(new Texture(static_id{ id })); }
		};

		std::vector<TextureRes> gen_bulk_textures(GLsizei n);

		template<GLsizei N>
		std::array<TextureRes, N> gen_bulk_textures()
		{
			std::array<GLuint, N> textures;
			glGenTextures(N, textures.data());
			std::array<TextureRes, N> wrapped_textures;
			for (GLsizei i = 0; i < N; ++i)
				wrapped_textures[i] = Texture::from_id(textures[i]);
			return wrapped_textures;
		}

		std::vector<TextureRes> create_bulk_textures(GLsizei n, GLenum target);

		template<GLsizei N>
		std::array<TextureRes, N> create_bulk_textures(GLenum target)
		{
			std::array<GLuint, N> textures;
			glCreateTextures(target, N, textures.data());
			std::array<TextureRes, N> wrapped_textures;
			for (GLsizei i = 0; i < N; ++i)
				wrapped_textures[i] = Texture::from_id(textures[i]);
			return wrapped_textures;
		}

		class BindlessTextureHandle
		{
			mutable GLuint64 handle = 0;

		public:
			BindlessTextureHandle() = default;
			BindlessTextureHandle(GLuint texture);
			~BindlessTextureHandle();

			void refresh(GLuint texture) const;
			void use() const;
			void disuse() const;
			operator GLuint64 () const { return handle; }
		};

		namespace tex
		{
			extern GLenum internal_format(int cpp);
			extern GLenum format(int cpp);
			extern GLint alignment(int cpp);
		}

		struct ImageDimensions
		{
			int w, h, cpp;
		};

		extern TextureRes load_static_texture_2d(const char* filename, ImageDimensions& dim);
	}
}
