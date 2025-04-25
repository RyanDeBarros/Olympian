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
			ImageBindlessTextureRes(const ImageRes& image, const BindlessTextureRes& texture) : image(image), texture(texture) {}
			ImageBindlessTextureRes(const ImageRes& image, BindlessTextureRes&& texture) : image(image), texture(std::move(texture)) {}
		};

		extern TextureRes load_texture_2d(const Image& image, bool generate_mipmaps = false);
		inline BindlessTextureRes load_bindless_texture_2d(const Image& image, bool generate_mipmaps = false)
		{
			return std::make_shared<BindlessTexture>(load_texture_2d(image, generate_mipmaps));
		}
		inline ImageTextureRes load_texture_2d(const char* filename, bool generate_mipmaps = false)
		{
			ImageTextureRes img;
			img.image = std::make_shared<Image>(filename);
			img.texture = load_texture_2d(*img.image, generate_mipmaps);
			return img;
		}
		inline ImageTextureRes load_texture_2d(const std::string& filename, bool generate_mipmaps = false)
		{
			return load_texture_2d(filename.c_str(), generate_mipmaps);
		}
		inline ImageBindlessTextureRes load_bindless_texture_2d(const char* filename, bool generate_mipmaps = false)
		{
			return ImageBindlessTextureRes(load_texture_2d(filename, generate_mipmaps));
		}
		inline ImageBindlessTextureRes load_bindless_texture_2d(const std::string& filename, bool generate_mipmaps = false)
		{
			return load_bindless_texture_2d(filename.c_str(), generate_mipmaps);
		}
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

		struct AnimTextureRes
		{
			AnimRes anim;
			TextureRes texture;
		};
		struct AnimBindlessTextureRes
		{
			AnimRes anim;
			BindlessTextureRes texture;

			AnimBindlessTextureRes() = default;
			AnimBindlessTextureRes(AnimTextureRes&& anim) : anim(std::move(anim.anim)), texture(std::make_shared<BindlessTexture>(std::move(anim.texture))) {}
			AnimBindlessTextureRes(const AnimRes& anim, const BindlessTextureRes& texture) : anim(anim), texture(texture) {}
			AnimBindlessTextureRes(const AnimRes& anim, BindlessTextureRes&& texture) : anim(anim), texture(std::move(texture)) {}
		};

		extern TextureRes load_texture_2d_array(const Anim& anim, bool generate_mipmaps = false);
		inline BindlessTextureRes load_bindless_texture_2d_array(const Anim& anim, bool generate_mipmaps = false)
		{
			return std::make_shared<BindlessTexture>(load_texture_2d_array(anim, generate_mipmaps));
		}
		inline AnimTextureRes load_texture_2d_array(const char* filename, SpritesheetOptions options = {}, bool generate_mipmaps = false)
		{
			AnimTextureRes anim;
			anim.anim = std::make_shared<Anim>(filename, options);
			anim.texture = load_texture_2d_array(*anim.anim, generate_mipmaps);
			return anim;
		}
		inline AnimTextureRes load_texture_2d_array(const std::string& filename, SpritesheetOptions options = {}, bool generate_mipmaps = false)
		{
			return load_texture_2d_array(filename.c_str(), options, generate_mipmaps);
		}
		inline AnimBindlessTextureRes load_bindless_texture_2d_array(const char* filename, SpritesheetOptions options = {}, bool generate_mipmaps = false)
		{
			return AnimBindlessTextureRes(load_texture_2d_array(filename, options, generate_mipmaps));
		}
		inline AnimBindlessTextureRes load_bindless_texture_2d_array(const std::string& filename, SpritesheetOptions options = {}, bool generate_mipmaps = false)
		{
			return load_bindless_texture_2d_array(filename.c_str(), options, generate_mipmaps);
		}
		inline TextureRes load_texture_2d_array(const char* filename, AnimDimensions& dim, SpritesheetOptions options = {}, bool generate_mipmaps = false)
		{
			auto res = load_texture_2d_array(filename, options, generate_mipmaps); dim = std::move(*res.anim->dim().lock()); return res.texture;
		}
		inline TextureRes load_texture_2d_array(const std::string& filename, AnimDimensions& dim, SpritesheetOptions options = {}, bool generate_mipmaps = false)
		{
			return load_texture_2d_array(filename.c_str(), dim, options, generate_mipmaps);
		}
		inline BindlessTextureRes load_bindless_texture_2d_array(const char* filename, AnimDimensions& dim, SpritesheetOptions options = {}, bool generate_mipmaps = false)
		{
			return std::make_shared<BindlessTexture>(load_texture_2d_array(filename, dim, options, generate_mipmaps));
		}
		inline BindlessTextureRes load_bindless_texture_2d_array(const std::string& filename, AnimDimensions& dim, SpritesheetOptions options = {}, bool generate_mipmaps = false)
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
		extern AnimFrameFormat setup_anim_frame_format(const TextureRegistry* texture_registry, const std::string& texture_name, float speed = 1.0f, GLuint starting_frame = 0);
		extern AnimFrameFormat setup_anim_frame_format(const Context* context, const std::string& texture_name, float speed = 1.0f, GLuint starting_frame = 0);
		extern AnimFrameFormat setup_anim_frame_format_single(const AnimDimensions& dim, GLuint frame);
		extern AnimFrameFormat setup_anim_frame_format_single(const TextureRegistry* texture_registry, const std::string& texture_name, GLuint frame);
		extern AnimFrameFormat setup_anim_frame_format_single(const Context* context, const std::string& texture_name, GLuint frame);

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

		struct VectorTextureRes
		{
			VectorImageRes image;
			TextureRes texture;
		};
		struct VectorBindlessTextureRes
		{
			VectorImageRes image;
			BindlessTextureRes texture;

			VectorBindlessTextureRes() = default;
			VectorBindlessTextureRes(VectorTextureRes&& image) : image(std::move(image.image)), texture(std::make_shared<BindlessTexture>(std::move(image.texture))) {}
			VectorBindlessTextureRes(const VectorImageRes& image, const BindlessTextureRes& texture) : image(image), texture(texture) {}
			VectorBindlessTextureRes(const VectorImageRes& image, BindlessTextureRes&& texture) : image(image), texture(std::move(texture)) {}
		};

		extern TextureRes load_nsvg_texture_2d(const VectorImageRes& image, bool generate_mipmaps = false);
		inline BindlessTextureRes load_bindless_nsvg_texture_2d(const VectorImageRes& image, bool generate_mipmaps = false)
		{
			return std::make_shared<BindlessTexture>(load_nsvg_texture_2d(image, generate_mipmaps));
		}
		// texture needs to be bound to GL_TEXTURE_2D before calling nsvg_manually_generate_mipmaps()
		extern void nsvg_manually_generate_mipmaps(const VectorImageRes& image, const NSVGAbstract& abstract, const NSVGContext& context);
	}
}
