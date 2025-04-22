#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <nanosvg/nanosvg.h>
#include <nanosvg/nanosvgrast.h>

#include <vector>
#include <array>
#include <memory>
#include <string>

#include "util/General.h"

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
			Texture t;
			GLuint64 handle = 0;
			GLuint64 _tex_handle = 0;
			std::vector<std::pair<GLuint, GLuint64>> _sampler_handles;

		public:
			BindlessTexture();
			BindlessTexture(GLenum target);
			BindlessTexture(Texture&& texture);
			BindlessTexture(TextureRes&& texture);
			BindlessTexture(const BindlessTexture&) = delete;
			BindlessTexture(BindlessTexture&&) noexcept;
			~BindlessTexture();
			BindlessTexture& operator=(BindlessTexture&&) noexcept;

			operator GLuint () const { return t; }
			void set_handle(); // texture becomes immutable
			void set_handle(GLuint sampler); // texture becomes immutable
			void set_and_use_handle() { set_handle(); use_handle(); }  // texture becomes immutable
			void set_and_use_handle(GLuint sampler) { set_handle(sampler); use_handle(); }  // texture becomes immutable
			GLuint64 get_handle() const { return handle; }
			void use_handle() const;
			void disuse_handle() const;
		};

		typedef std::shared_ptr<BindlessTexture> BindlessTextureRes;

		namespace tex
		{
			extern GLenum internal_format(int cpp);
			extern GLenum format(int cpp);
			extern void pixel_alignment(int cpp);
			inline GLsizei expected_mipmap_levels(float w, float h) { return (GLsizei)glm::floor(glm::log2(max(w, h))) + 1; }
			inline GLsizei expected_mipmap_levels(float w, float h, float d) { return (GLsizei)glm::floor(glm::log2(max(w, h, d))) + 1; }
		}

		struct ImageDimensions
		{
			int w = 0, h = 0, cpp = 4;

			glm::vec2 dimensions() const { return { w, h }; }
		};

		class Image
		{
			unsigned char* _buf = nullptr;
			ImageDimensions _dim;

		public:
			Image(const char* filepath);
			Image(unsigned char* buf, ImageDimensions dim);
			Image(const Image&) = delete;
			Image(Image&&) noexcept;
			~Image();
			Image& operator=(Image&&) noexcept;

			const unsigned char* buf() const { return _buf; }
			unsigned char* buf() { return _buf; }
			ImageDimensions dim() const { return _dim; }

			void delete_buffer();
		};

		typedef std::shared_ptr<Image> ImageRes;

		struct ImageTextureRes
		{
			ImageRes image;
			TextureRes texture;
		};
		struct ImageBindlessTextureRes
		{
			ImageRes image;
			BindlessTextureRes texture;

			ImageBindlessTextureRes() = default;
			ImageBindlessTextureRes(ImageTextureRes&& image) : image(std::move(image.image)), texture(std::make_shared<BindlessTexture>(std::move(image.texture))) {}
		};

		extern ImageTextureRes load_texture_2d(const char* filename, bool generate_mipmaps = false);
		inline ImageTextureRes load_texture_2d(const std::string& filename, bool generate_mipmaps = false) { return load_texture_2d(filename.c_str(), generate_mipmaps); }
		inline ImageBindlessTextureRes load_bindless_texture_2d(const char* filename, bool generate_mipmaps = false) { return ImageBindlessTextureRes(load_texture_2d(filename, generate_mipmaps)); }
		inline ImageBindlessTextureRes load_bindless_texture_2d(const std::string& filename, bool generate_mipmaps = false) { return load_bindless_texture_2d(filename.c_str(), generate_mipmaps); }

		inline TextureRes load_texture_2d(const char* filename, ImageDimensions& dim, bool generate_mipmaps = false)
		{
			auto res = load_texture_2d(filename, generate_mipmaps); dim = res.image->dim(); return res.texture;
		}
		inline TextureRes load_texture_2d(const std::string& filename, ImageDimensions& dim, bool generate_mipmaps = false)
		{
			return load_texture_2d(filename.c_str(), dim, generate_mipmaps);
		}
		inline BindlessTextureRes load_bindless_texture_2d(const char* filename, ImageDimensions& dim, bool generate_mipmaps = false)
		{
			return std::make_shared<BindlessTexture>(load_texture_2d(filename, dim, generate_mipmaps));
		}
		inline BindlessTextureRes load_bindless_texture_2d(const std::string& filename, ImageDimensions& dim, bool generate_mipmaps = false)
		{
			return load_bindless_texture_2d(filename.c_str(), dim, generate_mipmaps);
		}

		inline float gif_delay_epsilon = 1.0f;
		struct GIFDimensions
		{
			int w = 0, h = 0, cpp = 4;

			void set_delays(int* delays, unsigned int num_frames);

		private:
			int _frames = -1;
			std::vector<int> delays;

		public:
			bool uniform() const { return _frames >= 0; }
			GLuint frames() const { return uniform() ? (GLuint)_frames : (GLuint)delays.size(); }
			int delay(unsigned int frame = 0) const;

			glm::vec2 dimensions() const { return { w, h }; }
		};

		class GIF
		{
			unsigned char* _buf = nullptr;
			std::shared_ptr<GIFDimensions> _dim;

		public:
			GIF(const char* filepath);
			GIF(const GIF&) = delete;
			GIF(GIF&&) noexcept;
			~GIF();
			GIF& operator=(GIF&&) noexcept;

			const unsigned char* buf() const { return _buf; }
			unsigned char* buf() { return _buf; }
			std::weak_ptr<GIFDimensions> dim() const { return _dim; }

			void delete_buffer();
		};

		typedef std::shared_ptr<GIF> GIFRes;

		struct GIFTextureRes
		{
			GIFRes gif;
			TextureRes texture;
		};
		struct GIFBindlessTextureRes
		{
			GIFRes gif;
			BindlessTextureRes texture;

			GIFBindlessTextureRes() = default;
			GIFBindlessTextureRes(GIFTextureRes&& gif) : gif(std::move(gif.gif)), texture(std::make_shared<BindlessTexture>(std::move(gif.texture))) {}
		};

		extern GIFTextureRes load_texture_2d_array(const char* filename, bool generate_mipmaps = false);
		inline GIFTextureRes load_texture_2d_array(const std::string& filename, bool generate_mipmaps = false) { return load_texture_2d_array(filename.c_str(), generate_mipmaps); }
		inline GIFBindlessTextureRes load_bindless_texture_2d_array(const char* filename, bool generate_mipmaps = false) { return GIFBindlessTextureRes(load_texture_2d_array(filename, generate_mipmaps)); }
		inline GIFBindlessTextureRes load_bindless_texture_2d_array(const std::string& filename, bool generate_mipmaps = false) { return load_bindless_texture_2d_array(filename.c_str(), generate_mipmaps); }

		inline TextureRes load_texture_2d_array(const char* filename, GIFDimensions& dim, bool generate_mipmaps = false)
		{
			auto res = load_texture_2d_array(filename, generate_mipmaps); dim = std::move(*res.gif->dim().lock()); return res.texture;
		}
		inline TextureRes load_texture_2d_array(const std::string& filename, GIFDimensions& dim, bool generate_mipmaps = false)
		{
			return load_texture_2d_array(filename.c_str(), dim, generate_mipmaps);
		}
		inline BindlessTextureRes load_bindless_texture_2d_array(const char* filename, GIFDimensions& dim, bool generate_mipmaps = false)
		{
			return std::make_shared<BindlessTexture>(load_texture_2d_array(filename, dim, generate_mipmaps));
		}
		inline BindlessTextureRes load_bindless_texture_2d_array(const std::string& filename, GIFDimensions& dim, bool generate_mipmaps = false)
		{
			return load_bindless_texture_2d_array(filename.c_str(), dim, generate_mipmaps);
		}

		struct GIFFrameFormat
		{
			GLuint starting_frame = 0;
			GLuint num_frames = 0;
			float starting_time = 0.0f;
			float delay_seconds = 0.0f;

			bool operator==(const GIFFrameFormat&) const = default;
		};
	}

	class TextureRegistry;
	namespace rendering
	{

		extern GIFFrameFormat setup_gif_frame_format(const GIFDimensions& dim, float speed = 0.1f, GLuint starting_frame = 0);
		extern GIFFrameFormat setup_gif_frame_format(const TextureRegistry* texture_registry, const std::string& texture_name, float speed = 0.1f, GLuint starting_frame = 0);
		extern GIFFrameFormat setup_gif_frame_format_single(const GIFDimensions& dim, GLuint frame);
		extern GIFFrameFormat setup_gif_frame_format_single(const TextureRegistry* texture_registry, const std::string& texture_name, GLuint frame);

		// TODO spritesheets, and automatic UV updating in shaders from spritesheets

		class NSVGAbstract
		{
			friend class NSVGContext;
			mutable NSVGimage* i = nullptr;

		public:
			NSVGAbstract(const char* filepath);
			NSVGAbstract(const std::string& filepath);
			NSVGAbstract(const char* filepath, const char* units, float dpi);
			NSVGAbstract(const std::string& filepath, const char* units, float dpi);
			NSVGAbstract(const NSVGAbstract&) = delete;
			NSVGAbstract(NSVGAbstract&&) noexcept;
			~NSVGAbstract();
			NSVGAbstract& operator=(NSVGAbstract&&) noexcept;

			float width() const { return i ? i->width : 0.0f; }
			float height() const { return i ? i->height : 0.0f; }
		};

		class NSVGContext
		{
			NSVGrasterizer* r = nullptr;

		public:
			NSVGContext();
			NSVGContext(const NSVGContext&) = delete;
			NSVGContext(NSVGContext&&) noexcept;
			~NSVGContext();
			NSVGContext& operator=(NSVGContext&&) noexcept;

			Image rasterize(const NSVGAbstract& abstract, float scale);
		};
	}
}
