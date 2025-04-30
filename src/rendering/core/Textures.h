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
			extern void pixel_alignment_pre_send(int cpp);
			extern void pixel_alignment_post_send(int cpp);
		}

		struct ImageDimensions
		{
			int w = 0, h = 0, cpp = 4;

			glm::vec2 dimensions() const { return { w, h }; }
			unsigned char* pxnew() const { return new unsigned char[w * h * cpp]; }
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

		extern Texture load_texture_2d(const Image& image, bool generate_mipmaps = false);
		inline Texture load_texture_2d(const char* filename, ImageDimensions& dim, bool generate_mipmaps = false)
		{
			Image image(filename);
			dim = image.dim();
			return load_texture_2d(image, generate_mipmaps);
		}
		inline Texture load_texture_2d(const std::string& filename, ImageDimensions& dim, bool generate_mipmaps = false)
		{
			return load_texture_2d(filename.c_str(), dim, generate_mipmaps);
		}
		inline BindlessTexture load_bindless_texture_2d(const Image& image, bool generate_mipmaps = false)
		{
			return BindlessTexture(load_texture_2d(image, generate_mipmaps));
		}
		inline BindlessTexture load_bindless_texture_2d(const char* filename, ImageDimensions& dim, bool generate_mipmaps = false)
		{
			return BindlessTexture(load_texture_2d(filename, dim, generate_mipmaps));
		}
		inline BindlessTexture load_bindless_texture_2d(const std::string& filename, ImageDimensions& dim, bool generate_mipmaps = false)
		{
			return load_bindless_texture_2d(filename.c_str(), dim, generate_mipmaps);
		}

		inline float anim_delay_epsilon = 1.0f;
		struct AnimDimensions
		{
			int w = 0, h = 0, cpp = 4;

			void set_delays(int* delays, unsigned int num_frames);

		private:
			friend class Anim;
			int _frames = -1;
			std::vector<int> delays;

		public:
			bool uniform() const { return _frames >= 0; }
			GLuint frames() const { return uniform() ? (GLuint)_frames : (GLuint)delays.size(); }
			int delay(unsigned int frame = 0) const;

			glm::vec2 dimensions() const { return { w, h }; }
		};

		struct SpritesheetOptions
		{
			GLuint rows = 1, cols = 1;
			GLuint cell_width_override = 0, cell_height_override = 0;
			int delay_cs = 0;
			bool row_major = true, row_up = true;
		};

		class Anim
		{
			unsigned char* _buf = nullptr;
			std::shared_ptr<AnimDimensions> _dim;

		public:
			Anim(const char* filepath, SpritesheetOptions options = {});
			Anim(const Anim&) = delete;
			Anim(Anim&&) noexcept;
			~Anim();
			Anim& operator=(Anim&&) noexcept;

			const unsigned char* buf() const { return _buf; }
			unsigned char* buf() { return _buf; }
			std::weak_ptr<AnimDimensions> dim() const { return _dim; }

			void delete_buffer();
		};

		typedef std::shared_ptr<Anim> AnimRes;

		extern Texture load_texture_2d_array(const Anim& anim, bool generate_mipmaps = false);
		inline Texture load_texture_2d_array(const char* filename, AnimDimensions& dim, SpritesheetOptions options = {}, bool generate_mipmaps = false)
		{
			Anim anim(filename, options);
			auto texture = load_texture_2d_array(anim, generate_mipmaps);
			dim = std::move(*anim.dim().lock());
			return texture;
		}
		inline Texture load_texture_2d_array(const std::string& filename, AnimDimensions& dim, SpritesheetOptions options = {}, bool generate_mipmaps = false)
		{
			return load_texture_2d_array(filename.c_str(), dim, options, generate_mipmaps);
		}
		inline BindlessTexture load_bindless_texture_2d_array(const Anim& anim, bool generate_mipmaps = false)
		{
			return BindlessTexture(load_texture_2d_array(anim, generate_mipmaps));
		}
		inline BindlessTexture load_bindless_texture_2d_array(const char* filename, AnimDimensions& dim, SpritesheetOptions options = {}, bool generate_mipmaps = false)
		{
			return BindlessTexture(load_texture_2d_array(filename, dim, options, generate_mipmaps));
		}
		inline BindlessTexture load_bindless_texture_2d_array(const std::string& filename, AnimDimensions& dim, SpritesheetOptions options = {}, bool generate_mipmaps = false)
		{
			return load_bindless_texture_2d_array(filename.c_str(), dim, options, generate_mipmaps);
		}

		struct AnimFrameFormat
		{
			GLuint starting_frame = 0;
			GLuint num_frames = 0;
			float starting_time = 0.0f;
			float delay_seconds = 0.0f;

			bool operator==(const AnimFrameFormat&) const = default;
		};
	}

	class TextureRegistry;
	class Context;
	namespace rendering
	{
		extern AnimFrameFormat setup_anim_frame_format(const AnimDimensions& dim, float speed = 1.0f, GLuint starting_frame = 0);
		extern AnimFrameFormat setup_anim_frame_format(const TextureRegistry& texture_registry, const std::string& texture_name, float speed = 1.0f, GLuint starting_frame = 0);
		extern AnimFrameFormat setup_anim_frame_format(const Context& context, const std::string& texture_name, float speed = 1.0f, GLuint starting_frame = 0);
		extern AnimFrameFormat setup_anim_frame_format_single(const AnimDimensions& dim, GLuint frame);
		extern AnimFrameFormat setup_anim_frame_format_single(const TextureRegistry& texture_registry, const std::string& texture_name, GLuint frame);
		extern AnimFrameFormat setup_anim_frame_format_single(const Context& context, const std::string& texture_name, GLuint frame);

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
			mutable NSVGrasterizer* r = nullptr;

		public:
			NSVGContext();
			NSVGContext(const NSVGContext&) = delete;
			NSVGContext(NSVGContext&&) noexcept;
			~NSVGContext();
			NSVGContext& operator=(NSVGContext&&) noexcept;

			Image rasterize(const NSVGAbstract& abstract, float scale) const;
			ImageRes rasterize_res(const NSVGAbstract& abstract, float scale) const;

		private:
			void rasterize_unsafe(const NSVGAbstract& abstract, float scale, unsigned char*& buf, ImageDimensions& dim) const;
		};

		struct VectorImageRes
		{
			ImageRes image;
			float scale;
		};

		extern Texture load_nsvg_texture_2d(const VectorImageRes& image, bool generate_mipmaps = false);
		inline BindlessTexture load_bindless_nsvg_texture_2d(const VectorImageRes& image, bool generate_mipmaps = false)
		{
			return BindlessTexture(load_nsvg_texture_2d(image, generate_mipmaps));
		}
		// texture needs to be bound to GL_TEXTURE_2D before calling nsvg_manually_generate_mipmaps()
		extern void nsvg_manually_generate_mipmaps(const VectorImageRes& image, const NSVGAbstract& abstract, const NSVGContext& context);
	}
}
