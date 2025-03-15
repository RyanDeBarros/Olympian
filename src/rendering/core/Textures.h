#pragma once

#include <GL/glew.h>

#include <vector>
#include <array>
#include <memory>

namespace oly
{
	namespace rendering
	{
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
			static std::shared_ptr<Texture> from_id(GLuint id) { return std::shared_ptr<Texture>(new Texture(static_id{ id })); }
		};

		std::vector<std::shared_ptr<Texture>> gen_bulk_textures(GLsizei n);

		template<GLsizei N>
		std::array<std::shared_ptr<Texture>, N> gen_bulk_textures()
		{
			std::array<GLuint, N> textures;
			glGenTextures(N, textures.data());
			std::array<std::shared_ptr<Texture>, N> wrapped_textures;
			for (GLsizei i = 0; i < N; ++i)
				wrapped_textures[i] = Texture::from_id(textures[i]);
			return wrapped_textures;
		}

		std::vector<std::shared_ptr<Texture>> create_bulk_textures(GLsizei n, GLenum target);

		template<GLsizei N>
		std::array<std::shared_ptr<Texture>, N> create_bulk_textures(GLenum target)
		{
			std::array<GLuint, N> textures;
			glCreateTextures(target, N, textures.data());
			std::array<std::shared_ptr<Texture>, N> wrapped_textures;
			for (GLsizei i = 0; i < N; ++i)
				wrapped_textures[i] = Texture::from_id(textures[i]);
			return wrapped_textures;
		}
	}
}
