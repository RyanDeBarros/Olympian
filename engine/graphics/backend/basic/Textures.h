#pragma once

#include <vector>
#include <array>
#include <string>
#include <memory>

#include "external/GLM.h"
#include "external/NSVG.h"

#include "core/types/SmartReference.h"
#include "core/types/DeferredFalse.h"

#include "graphics/backend/basic/Sampler.h"

namespace oly::graphics
{
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

	typedef SmartReference<Texture> TextureRef;

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

	typedef SmartReference<BindlessTexture> BindlessTextureRef;

	extern GLenum texture_internal_format(int cpp);
	extern GLenum texture_format(int cpp);
	extern void pixel_alignment_pre_send(int cpp);
	extern void pixel_alignment_post_send(int cpp);

	struct ImageDimensions
	{
		int w = 0, h = 0, cpp = 4;

		glm::vec2 dimensions() const { return { w, h }; }
		unsigned char* pxnew() const { return new unsigned char[w * h * cpp]; }
	};

	// TODO v5 support for other pixel types

	template<typename Pixel = unsigned char>
	inline void tex_image_2d(GLenum target, const ImageDimensions& dim, const Pixel* pixels, GLint level = 0)
	{
		GLenum data_type;
		if constexpr (std::is_same_v<Pixel, unsigned char>)
			data_type = GL_UNSIGNED_BYTE;
		else
			static_assert(deferred_false<Pixel>, "Unsupported pixel type in tex_image_2d().");
		glTexImage2D(target, level, texture_internal_format(dim.cpp), dim.w, dim.h, 0, texture_format(dim.cpp), data_type, pixels);
	}

	template<typename Pixel = unsigned char>
	inline void tex_image_2d(GLenum target, const ImageDimensions& dim, GLint level = 0)
	{
		GLenum data_type;
		if constexpr (std::is_same_v<Pixel, unsigned char>)
			data_type = GL_UNSIGNED_BYTE;
		else
			static_assert(deferred_false<Pixel>, "Unsupported pixel type in tex_image_2d().");
		glTexImage2D(target, level, texture_internal_format(dim.cpp), dim.w, dim.h, 0, texture_format(dim.cpp), data_type, nullptr);
	}

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

	typedef SmartReference<Image> ImageRef;

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

	class NSVGAbstract;
	class Anim
	{
		unsigned char* _buf = nullptr;
		std::shared_ptr<AnimDimensions> _dim;

	public:
		Anim(const char* filepath, SpritesheetOptions options = {});
		Anim(const NSVGAbstract& svg_abstract, float scale, SpritesheetOptions options = {});

	private:
		void parse_sprite_sheet(const Image& image, SpritesheetOptions options);

	public:
		Anim(const Anim&) = delete;
		Anim(Anim&&) noexcept;
		~Anim();
		Anim& operator=(Anim&&) noexcept;

		const unsigned char* buf() const { return _buf; }
		unsigned char* buf() { return _buf; }
		std::weak_ptr<AnimDimensions> dim() const { return _dim; }
		glm::vec2 dimensions() const { return _dim->dimensions(); }

		void delete_buffer();
	};

	typedef SmartReference<Anim> AnimRef;

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

	extern AnimFrameFormat setup_anim_frame_format(const AnimDimensions& dim, float speed = 1.0f, GLuint starting_frame = 0);
	extern AnimFrameFormat setup_anim_frame_format(const std::string& texture_file, float speed = 1.0f, GLuint starting_frame = 0);
	extern AnimFrameFormat setup_anim_frame_format_single(const AnimDimensions& dim, GLuint frame);
	extern AnimFrameFormat setup_anim_frame_format_single(const std::string& texture_file, GLuint frame);

	class NSVGAbstract
	{
		friend class NSVGContext;
		mutable NSVGimage* i = nullptr;

	public:
		NSVGAbstract(const char* filepath, const char* units = "px", float dpi = 96.0f);
		NSVGAbstract(const std::string& filepath, const char* units = "px", float dpi = 96.0f);
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
		ImageRef rasterize_res(const NSVGAbstract& abstract, float scale) const;

	private:
		void rasterize_unsafe(const NSVGAbstract& abstract, float scale, unsigned char*& buf, ImageDimensions& dim) const;
	};

	struct VectorImageRef
	{
		ImageRef image;
		float scale;
	};

	extern Texture load_nsvg_texture_2d(const VectorImageRef& image, bool generate_mipmaps = false);
	inline BindlessTexture load_bindless_nsvg_texture_2d(const VectorImageRef& image, bool generate_mipmaps = false)
	{
		return BindlessTexture(load_nsvg_texture_2d(image, generate_mipmaps));
	}
	// texture needs to be bound to GL_TEXTURE_2D before calling nsvg_manually_generate_mipmaps()
	extern void nsvg_manually_generate_mipmaps(const VectorImageRef& image, const NSVGAbstract& abstract, const NSVGContext& context);
}
