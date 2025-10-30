#pragma once

#include <vector>
#include <array>
#include <string>
#include <memory>

#include "external/GLM.h"
#include "external/NSVG.h"

#include "core/types/SmartReference.h"
#include "core/types/DeferredFalse.h"
#include "core/math/Shapes.h"
#include "core/util/ResourcePath.h"

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

		void set_parameter(GLenum param, GLint value) const;
	};

	typedef SmartReference<Texture> TextureRef;

	class BindlessTexture
	{
		Texture t;
		GLuint64 handle = 0;

	public:
		BindlessTexture();
		BindlessTexture(GLenum target);
		BindlessTexture(Texture&& texture);
		BindlessTexture(const BindlessTexture&) = delete;
		BindlessTexture(BindlessTexture&&) noexcept;
		~BindlessTexture();
		BindlessTexture& operator=(BindlessTexture&&) noexcept;

		operator GLuint () const { return t; }
		void set_handle();
		void set_handle(GLuint sampler);
		void set_and_use_handle() { set_handle(); use_handle(); }
		void set_and_use_handle(GLuint sampler) { set_handle(sampler); use_handle(); }
		GLuint64 get_handle() const { return handle; }
		void use_handle() const;
		void disuse_handle() const;

		const Texture& texture() const { return t; }
	};

	typedef SmartReference<BindlessTexture> BindlessTextureRef;

	class ScopedPixelAlignment
	{
		int cpp;

	public:
		ScopedPixelAlignment(int cpp);
		ScopedPixelAlignment(const ScopedPixelAlignment&) = delete;
		ScopedPixelAlignment(ScopedPixelAlignment&&) = delete;
		~ScopedPixelAlignment();
	};

	struct ImageDimensions
	{
		int w = 0, h = 0, cpp = 4;

		glm::vec2 dimensions() const { return { w, h }; }
		template<typename Pixel = unsigned char>
		Pixel* pxnew() const { return new Pixel[w * h * cpp]; }
	};

	struct Image3DDimensions
	{
		int w = 0, h = 0, d = 0, cpp = 4;
	};

	namespace tex
	{
		extern GLenum internal_format(int cpp);
		extern GLenum format(int cpp);
		extern GLint mipmap_levels(int w, int h);

		extern void storage_2d(GLuint texture, ImageDimensions dim, GLsizei levels = 1);

		template<typename Pixel = unsigned char>
		inline void subimage_2d(GLuint texture, const Pixel* pixels, math::IArea2D subarea, int cpp, GLint level = 0)
		{
			GLenum data_type;
			if constexpr (std::is_same_v<Pixel, unsigned char>)
				data_type = GL_UNSIGNED_BYTE;
			else if constexpr (std::is_same_v<Pixel, float>)
				data_type = GL_FLOAT;
			else
				static_assert(deferred_false<Pixel>, "Unsupported pixel type in oly::graphics::tex::subimage_2d().");
			ScopedPixelAlignment pixel_align(cpp);
			glTextureSubImage2D(texture, level, subarea.x, subarea.y, subarea.w, subarea.h, format(cpp), data_type, pixels);
		}

		template<typename Pixel = unsigned char>
		inline void image_2d(GLuint texture, const Pixel* pixels, ImageDimensions dim, bool mipmaps)
		{
			storage_2d(texture, dim, mipmaps ? mipmap_levels(dim.w, dim.h) : 1);
			subimage_2d(texture, pixels, { .w = dim.w, .h = dim.h }, dim.cpp);
			if (mipmaps)
				glGenerateTextureMipmap(texture);
		}

		extern void storage_3d(GLuint texture, Image3DDimensions dim, GLsizei levels = 1);

		template<typename Pixel = unsigned char>
		inline void subimage_3d(GLuint texture, const Pixel* pixels, math::IArea3D subarea, int cpp, GLint level = 0)
		{
			GLenum data_type;
			if constexpr (std::is_same_v<Pixel, unsigned char>)
				data_type = GL_UNSIGNED_BYTE;
			else if constexpr (std::is_same_v<Pixel, float>)
				data_type = GL_FLOAT;
			else
				static_assert(deferred_false<Pixel>, "Unsupported pixel type in oly::graphics::tex::subimage_3d().");
			ScopedPixelAlignment pixel_align(cpp);
			glTextureSubImage3D(texture, level, subarea.x, subarea.y, subarea.z, subarea.w, subarea.h, subarea.d, format(cpp), data_type, pixels);
		}
	}

	class Image
	{
		unsigned char* _buf = nullptr;
		ImageDimensions _dim;

	public:
		Image(const ResourcePath& file);
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
	
	inline Texture load_texture_2d(const ResourcePath& file, ImageDimensions& dim, bool generate_mipmaps = false)
	{
		Image image(file);
		dim = image.dim();
		return load_texture_2d(image, generate_mipmaps);
	}

	inline BindlessTexture load_bindless_texture_2d(const Image& image, bool generate_mipmaps = false)
	{
		return BindlessTexture(load_texture_2d(image, generate_mipmaps));
	}

	inline BindlessTexture load_bindless_texture_2d(const ResourcePath& file, ImageDimensions& dim, bool generate_mipmaps = false)
	{
		return BindlessTexture(load_texture_2d(file, dim, generate_mipmaps));
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
		SmartReference<AnimDimensions> _dim = REF_INIT;

	public:
		Anim(const ResourcePath& filepath, SpritesheetOptions options = {});
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
		SmartReference<AnimDimensions> dim() const { return _dim; }
		glm::vec2 dimensions() const { return _dim->dimensions(); }

		void delete_buffer();
	};

	typedef SmartReference<Anim> AnimRef;

	extern Texture load_texture_2d_array(const Anim& anim, bool generate_mipmaps = false);

	inline Texture load_texture_2d_array(const ResourcePath& file, SmartReference<AnimDimensions>& dim, SpritesheetOptions options = {}, bool generate_mipmaps = false)
	{
		Anim anim(file, options);
		auto texture = load_texture_2d_array(anim, generate_mipmaps);
		dim = anim.dim();
		return texture;
	}
	
	inline BindlessTexture load_bindless_texture_2d_array(const Anim& anim, bool generate_mipmaps = false)
	{
		return BindlessTexture(load_texture_2d_array(anim, generate_mipmaps));
	}
	
	inline BindlessTexture load_bindless_texture_2d_array(const ResourcePath& file, SmartReference<AnimDimensions>& dim, SpritesheetOptions options = {}, bool generate_mipmaps = false)
	{
		return BindlessTexture(load_texture_2d_array(file, dim, options, generate_mipmaps));
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
	extern AnimFrameFormat setup_anim_frame_format(const ResourcePath& texture_file, float speed = 1.0f, GLuint starting_frame = 0);
	extern AnimFrameFormat setup_anim_frame_format_single(const AnimDimensions& dim, GLuint frame);
	extern AnimFrameFormat setup_anim_frame_format_single(const ResourcePath& texture_file, GLuint frame);

	class NSVGAbstract
	{
		friend class NSVGContext;
		mutable NSVGimage* i = nullptr;

	public:
		NSVGAbstract(const ResourcePath& file, const char* units = "px", float dpi = 96.0f);
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
		float scale = 1.0f;
	};

	enum class SVGMipmapGenerationMode
	{
		AUTO,
		OFF,
		MANUAL
	};

	extern Texture load_nsvg_texture_2d(const VectorImageRef& image, SVGMipmapGenerationMode generate_mipmaps = SVGMipmapGenerationMode::OFF, const NSVGAbstract* abstract = nullptr);
	
	inline BindlessTexture load_bindless_nsvg_texture_2d(const VectorImageRef& image, SVGMipmapGenerationMode generate_mipmaps = SVGMipmapGenerationMode::OFF, const NSVGAbstract* abstract = nullptr)
	{
		return BindlessTexture(load_nsvg_texture_2d(image, generate_mipmaps, abstract));
	}

	// TODO v6 texture streaming
}
