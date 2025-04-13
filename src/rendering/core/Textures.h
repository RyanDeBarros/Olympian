#pragma once

#include <GL/glew.h>

#include <vector>
#include <array>
#include <memory>
#include <string>

namespace oly
{
	namespace rendering
	{
		class Sampler
		{
			GLuint id = 0;

		public:
			Sampler();
			Sampler(const Sampler&) = delete;
			Sampler(Sampler&&) noexcept;
			~Sampler();
			Sampler& operator=(Sampler&&) noexcept;

			operator GLuint () const { return id; }

			void set_parameter_i(GLenum param, GLint value) const;
			void set_parameter_iv(GLenum param, const GLint* values) const;
			void set_parameter_f(GLenum param, GLfloat value) const;
			void set_parameter_fv(GLenum param, const GLfloat* values) const;
		};

		typedef std::shared_ptr<Sampler> SamplerRes;

		class Texture
		{
			GLuint id = 0;

		public:
			Texture();
			Texture(GLenum target);
			Texture(const Texture&) = delete;
			Texture(Texture&&) noexcept;
			~Texture();
			Texture& operator=(Texture&&) noexcept;

			operator GLuint () const { return id; }
		};

		typedef std::shared_ptr<Texture> TextureRes;

		class BindlessTexture
		{
			GLuint id = 0;
			GLuint64 handle = 0;
			GLuint64 _tex_handle = 0;
			std::vector<std::pair<GLuint, GLuint64>> _sampler_handles;

		public:
			BindlessTexture();
			BindlessTexture(GLenum target);
			BindlessTexture(const BindlessTexture&) = delete;
			BindlessTexture(BindlessTexture&&) noexcept;
			~BindlessTexture();
			BindlessTexture& operator=(BindlessTexture&&) noexcept;

			operator GLuint () const { return id; }
			void set_handle(); // texture becomes immutable
			void set_handle(GLuint sampler); // texture becomes immutable
			GLuint64 get_handle() const { return handle; }
			void use_handle() const;
			void disuse_handle() const;
		};

		typedef std::shared_ptr<BindlessTexture> BindlessTextureRes;

		struct ImageDimensions
		{
			int w = 0, h = 0, cpp = 4;
		};

		class Image
		{
			unsigned char* _buf = nullptr;
			ImageDimensions _dim;

		public:
			Image(const char* filepath);
			Image(const Image&) = delete;
			Image(Image&&) noexcept;
			~Image();
			Image& operator=(Image&&) noexcept;

			const unsigned char* buf() const { return _buf; }
			unsigned char* buf() { return _buf; }
			ImageDimensions dim() const { return _dim; }
		};

		typedef std::shared_ptr<Image> ImageRes;

		namespace tex
		{
			extern GLenum internal_format(int cpp);
			extern GLenum format(int cpp);
			extern GLint alignment(int cpp);
			
			extern void image_2d(GLenum target, ImageDimensions dim, void* buf, GLenum data_type);
		}

		extern TextureRes load_texture_2d(const char* filename, ImageDimensions& dim);
		inline TextureRes load_texture_2d(const std::string& filename, ImageDimensions& dim) { return load_texture_2d(filename.c_str(), dim); }
		extern BindlessTextureRes load_bindless_texture_2d(const char* filename, ImageDimensions& dim);
		inline BindlessTextureRes load_bindless_texture_2d(const std::string& filename, ImageDimensions& dim) { return load_bindless_texture_2d(filename.c_str(), dim); }

		struct ImageTextureRes
		{
			ImageRes image;
			TextureRes texture;
		};
		struct ImageBindlessTextureRes
		{
			ImageRes image;
			BindlessTextureRes texture;
		};

		extern ImageTextureRes load_texture_2d(const char* filename);
		inline ImageTextureRes load_texture_2d(const std::string& filename) { return load_texture_2d(filename.c_str()); }
		extern ImageBindlessTextureRes load_bindless_texture_2d(const char* filename);
		inline ImageBindlessTextureRes load_bindless_texture_2d(const std::string& filename) { return load_bindless_texture_2d(filename.c_str()); }
	}
}
