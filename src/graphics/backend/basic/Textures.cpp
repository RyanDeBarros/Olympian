#include "Textures.h"

#include "external/STB.h"
#include "core/base/Context.h"
#include "core/base/Errors.h"
#include "core/base/Assert.h"
#include "core/util/IO.h"
#include "core/types/Approximate.h"

namespace oly::graphics
{
	Texture::Texture()
	{
		glGenTextures(1, &id);
	}

	Texture::Texture(GLenum target)
	{
		glCreateTextures(target, 1, &id);
	}

	Texture::Texture(Texture&& other) noexcept
		: id(other.id)
	{
		other.id = 0;
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &id);
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		if (this != &other)
		{
			glDeleteTextures(1, &id);
			id = other.id;
			other.id = 0;
		}
		return *this;
	}

	BindlessTexture::BindlessTexture()
	{
	}

	BindlessTexture::BindlessTexture(GLenum target)
		: t(target)
	{
	}

	BindlessTexture::BindlessTexture(Texture&& texture)
		: t(std::move(texture))
	{
	}

	BindlessTexture::BindlessTexture(TextureRes&& texture)
		: t(std::move(*texture))
	{
		texture.reset();
	}

	BindlessTexture::BindlessTexture(BindlessTexture&& other) noexcept
		: t(std::move(other.t)), handle(other.handle)
	{
		other.handle = 0;
	}

	BindlessTexture::~BindlessTexture()
	{
		disuse_handle();
	}

	BindlessTexture& BindlessTexture::operator=(BindlessTexture&& other) noexcept
	{
		if (this != &other)
		{
			disuse_handle();
			t = std::move(other.t);
			handle = other.handle;
			other.handle = 0;
		}
		return *this;
	}

	void BindlessTexture::set_handle()
	{
		disuse_handle();
		if (!_tex_handle)
			_tex_handle = glGetTextureHandleARB(t);
		handle = _tex_handle;
	}

	void BindlessTexture::set_handle(GLuint sampler)
	{
		disuse_handle();
		for (auto iter = _sampler_handles.begin(); iter != _sampler_handles.end(); ++iter)
		{
			if (iter->first == sampler)
			{
				handle = iter->second;
				return;
			}
		}
		handle = glGetTextureSamplerHandleARB(t, sampler);
		_sampler_handles.push_back({ sampler, handle });
	}

	void BindlessTexture::use_handle() const
	{
		if (handle)
			glMakeTextureHandleResidentARB(handle);
	}

	void BindlessTexture::disuse_handle() const
	{
		if (handle)
			glMakeTextureHandleNonResidentARB(handle);
	}

	GLenum texture_internal_format(int cpp)
	{
		return cpp == 1 ? GL_R8
			: cpp == 2 ? GL_RG8
			: cpp == 3 ? GL_RGB8
			: GL_RGBA8;
	}

	GLenum texture_format(int cpp)
	{
		return cpp == 1 ? GL_RED
			: cpp == 2 ? GL_RG
			: cpp == 3 ? GL_RGB
			: GL_RGBA;
	}

	void pixel_alignment_pre_send(int cpp)
	{
		if (cpp == 1 || cpp == 3)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		else if (cpp == 2)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	}

	void pixel_alignment_post_send(int cpp)
	{
		if (cpp != 4)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}

	Image::Image(const char* filepath)
	{
		_buf = stbi_load(filepath, &_dim.w, &_dim.h, &_dim.cpp, 0);
	}

	Image::Image(unsigned char* buf, ImageDimensions dim)
		: _buf(buf), _dim(dim)
	{
	}

	Image::Image(Image&& other) noexcept
		: _buf(other._buf)
	{
		other._buf = nullptr;
	}

	Image::~Image()
	{
		stbi_image_free(_buf);
	}

	Image& Image::operator=(Image&& other) noexcept
	{
		if (this != &other)
		{
			stbi_image_free(_buf);
			_buf = other._buf;
			_dim = other._dim;
			other._buf = nullptr;
		}
		return *this;
	}

	void Image::delete_buffer()
	{
		stbi_image_free(_buf);
		_buf = nullptr;
	}

	Texture load_texture_2d(const Image& image, bool generate_mipmaps)
	{
		Texture texture(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
		const auto& dim = image.dim();
		pixel_alignment_pre_send(dim.cpp);
		glTexImage2D(GL_TEXTURE_2D, 0, texture_internal_format(dim.cpp), dim.w, dim.h, 0, texture_format(dim.cpp), GL_UNSIGNED_BYTE, image.buf());
		pixel_alignment_post_send(dim.cpp);
		if (generate_mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);
		return texture;
	}

	void AnimDimensions::set_delays(int* new_delays, unsigned int num_frames)
	{
		delays.clear();
		_frames = -1;
		if (num_frames == 0)
			return;
		delays.resize(num_frames);
		bool single = true;
		int delay = new_delays[0];
		delays[0] = delay;
		for (unsigned int i = 1; i < num_frames; ++i)
		{
			delays[i] = new_delays[i];
			if (std::abs(delays[i] - delay) > anim_delay_epsilon)
				single = false;
		}
		if (single)
		{
			delays.resize(1);
			_frames = num_frames;
		}
	}

	int AnimDimensions::delay(unsigned int frame) const
	{
		if (frame >= frames())
			throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		return uniform() ? delays[0] : delays[frame];
	}

	Anim::Anim(const char* filepath, SpritesheetOptions options)
		: _dim(std::make_shared<AnimDimensions>())
	{
		if (io::file_extension(filepath) == ".gif")
		{
			auto full_content = io::read_file_uc(filepath);
			int* delays;
			int frames;
			_buf = stbi_load_gif_from_memory(full_content.data(), (int)full_content.size(), &delays, &_dim->w, &_dim->h, &frames, &_dim->cpp, 0);
			_dim->set_delays(delays, frames);
			stbi_image_free(delays);
		}
		else
		{
			_dim->delays = { options.delay_cs };
			Image image(filepath);
			auto idim = image.dim();
			_dim->cpp = idim.cpp;

			if (options.cols > (GLuint)idim.w)
				options.cols = (GLuint)idim.w;
			else if (options.cols == 0)
				options.cols = 1;
			if (options.cell_width_override == 0)
				options.cell_width_override = (int)(idim.w / options.cols);
			else if (options.cell_width_override * options.cols > (GLuint)idim.w)
				options.cols = (int)(idim.w / options.cell_width_override);

			if (options.rows > (GLuint)idim.h)
				options.rows = (GLuint)idim.h;
			else if (options.rows == 0)
				options.rows = 1;
			if (options.cell_height_override == 0)
				options.cell_height_override = (int)(idim.h / options.rows);
			else if (options.cell_height_override * options.rows > (GLuint)idim.h)
				options.rows = (int)(idim.h / options.cell_height_override);

			_dim->w = options.cell_width_override;
			_dim->h = options.cell_height_override;
			_dim->_frames = options.rows * options.cols;

			GLuint minor_stride = options.cell_width_override * idim.cpp;
			GLuint major_stride = minor_stride * options.cols;
			GLuint image_major_stride = idim.w * idim.cpp;
			GLuint minor_height = options.cell_height_override;
			GLuint major_height = minor_height * options.rows;
			GLuint minor_area = minor_stride * minor_height;
			_buf = new unsigned char[major_stride * major_height];
			const auto cpy = [this, minor_height, minor_stride, ibuf = image.buf(), minor_area, image_major_stride](GLuint i, GLuint j, GLuint k) {
				for (GLuint r = 0; r < minor_height; ++r)
					memcpy(_buf + k * minor_area + r * minor_stride, ibuf + j * minor_stride + (i * minor_height + r) * image_major_stride, minor_stride);
				};
			GLuint k = 0;
			if (options.row_major)
			{
				if (options.row_up)
				{
					for (GLuint i = 0; i < options.rows; ++i)
						for (GLuint j = 0; j < options.cols; ++j)
							cpy(i, j, k++);
				}
				else
				{
					for (int i = options.rows - 1; i >= 0; --i)
						for (GLuint j = 0; j < options.cols; ++j)
							cpy((GLuint)i, j, k++);
				}
			}
			else
			{
				if (options.row_up)
				{
					for (GLuint j = 0; j < options.cols; ++j)
						for (GLuint i = 0; i < options.rows; ++i)
							cpy(i, j, k++);
				}
				else
				{
					for (GLuint j = 0; j < options.cols; ++j)
						for (int i = options.rows - 1; i >= 0; --i)
							cpy((GLuint)i, j, k++);
				}
			}
		}
	}

	Anim::Anim(Anim&& other) noexcept
		: _buf(other._buf), _dim(std::move(other._dim))
	{
		other._buf = nullptr;
	}

	Anim::~Anim()
	{
		stbi_image_free(_buf);
	}

	Anim& Anim::operator=(Anim&& other) noexcept
	{
		if (this != &other)
		{
			stbi_image_free(_buf);
			_buf = other._buf;
			_dim = std::move(other._dim);
			other._buf = nullptr;
		}
		return *this;
	}

	void Anim::delete_buffer()
	{
		stbi_image_free(_buf);
		_buf = nullptr;
	}

	Texture load_texture_2d_array(const Anim& anim, bool generate_mipmaps)
	{
		Texture texture(GL_TEXTURE_2D_ARRAY);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
		const auto& dim = anim.dim().lock();
		pixel_alignment_pre_send(dim->cpp);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, texture_internal_format(dim->cpp), dim->w, dim->h, dim->frames(), 0, texture_format(dim->cpp), GL_UNSIGNED_BYTE, nullptr);
		for (GLuint i = 0; i < dim->frames(); ++i)
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, dim->w, dim->h, 1, texture_format(dim->cpp), GL_UNSIGNED_BYTE, anim.buf() + i * dim->w * dim->h * dim->cpp);
		pixel_alignment_post_send(dim->cpp);
		if (generate_mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
		return texture;
	}

	AnimFrameFormat setup_anim_frame_format(const AnimDimensions& dim, float speed, GLuint starting_frame)
	{
		OLY_ASSERT(dim.uniform());
		return { starting_frame, dim.frames(), 0.0f, 0.01f * dim.delay() / speed };
	}

	AnimFrameFormat setup_anim_frame_format(const std::string& texture_name, float speed, GLuint starting_frame)
	{
		return setup_anim_frame_format(*context::texture_registry().get_anim_dimensions(texture_name).lock(), speed, starting_frame);
	}

	AnimFrameFormat setup_anim_frame_format_single(const AnimDimensions& dim, GLuint frame)
	{
		return { frame, dim.frames(), 0.0f, 0.0f };
	}

	AnimFrameFormat setup_anim_frame_format_single(const std::string& texture_name, GLuint frame)
	{
		return setup_anim_frame_format_single(*context::texture_registry().get_anim_dimensions(texture_name).lock(), frame);
	}

	NSVGAbstract::NSVGAbstract(const char* filepath)
		: i(nsvgParseFromFile(filepath, "px", 96))
	{
		if (!i) throw Error(ErrorCode::NSVG_PARSING);
	}

	NSVGAbstract::NSVGAbstract(const std::string& filepath)
		: i(nsvgParseFromFile(filepath.c_str(), "px", 96))
	{
		if (!i) throw Error(ErrorCode::NSVG_PARSING);
	}
		
	NSVGAbstract::NSVGAbstract(const char* filepath, const char* units, float dpi)
		: i(nsvgParseFromFile(filepath, units, dpi))
	{
		if (!i) throw Error(ErrorCode::NSVG_PARSING);
	}

	NSVGAbstract::NSVGAbstract(const std::string& filepath, const char* units, float dpi)
		: i(nsvgParseFromFile(filepath.c_str(), units, dpi))
	{
		if (!i) throw Error(ErrorCode::NSVG_PARSING);
	}

	NSVGAbstract::NSVGAbstract(NSVGAbstract&& other) noexcept
		: i(other.i)
	{
		other.i = nullptr;
	}
		
	NSVGAbstract::~NSVGAbstract()
	{
		nsvgDelete(i);
	}
		
	NSVGAbstract& NSVGAbstract::operator=(NSVGAbstract&& other) noexcept
	{
		if (this != &other)
		{
			nsvgDelete(i);
			i = other.i;
			other.i = nullptr;
		}
		return *this;
	}

	NSVGContext::NSVGContext()
		: r(nsvgCreateRasterizer())
	{
	}

	NSVGContext::NSVGContext(NSVGContext&& other) noexcept
		: r(other.r)
	{
		other.r = nullptr;
	}
		
	NSVGContext::~NSVGContext()
	{
		nsvgDeleteRasterizer(r);
	}
		
	NSVGContext& NSVGContext::operator=(NSVGContext&& other) noexcept
	{
		if (this != &other)
		{
			nsvgDeleteRasterizer(r);
			r = other.r;
			other.r = nullptr;
		}
		return *this;
	}

	Image NSVGContext::rasterize(const NSVGAbstract& abstract, float scale) const
	{
		unsigned char* buf;
		ImageDimensions dim;
		rasterize_unsafe(abstract, scale, buf, dim);
		return Image(buf, dim);
	}

	ImageRes NSVGContext::rasterize_res(const NSVGAbstract& abstract, float scale) const
	{
		unsigned char* buf;
		ImageDimensions dim;
		rasterize_unsafe(abstract, scale, buf, dim);
		return std::make_shared<Image>(buf, dim);
	}

	void NSVGContext::rasterize_unsafe(const NSVGAbstract& abstract, float scale, unsigned char*& buf, ImageDimensions& dim) const
	{
		dim.w = std::max((int)(scale * abstract.width()), 1);
		dim.h = std::max((int)(scale * abstract.height()), 1);
		dim.cpp = 4;
		int stride = dim.w * dim.cpp;
		buf = new unsigned char[stride * dim.h];
		nsvgRasterize(r, abstract.i, 0.0f, 0.0f, scale, buf, dim.w, dim.h, stride);

		unsigned char* temp = new unsigned char[stride];
		for (int i = 0; i < dim.h / 2; ++i)
		{
			memcpy(temp, buf + i * stride, stride);
			memcpy(buf + i * stride, buf + (dim.h - 1 - i) * stride, stride);
			memcpy(buf + (dim.h - 1 - i) * stride, temp, stride);
		}
		delete[] temp;
	}

	Texture load_nsvg_texture_2d(const VectorImageRes& image, bool generate_mipmaps)
	{
		Texture texture(GL_TEXTURE_2D);;
		glBindTexture(GL_TEXTURE_2D, texture);
		const auto& dim = image.image->dim();
		pixel_alignment_pre_send(dim.cpp);
		glTexImage2D(GL_TEXTURE_2D, 0, texture_internal_format(dim.cpp), dim.w, dim.h, 0, texture_format(dim.cpp), GL_UNSIGNED_BYTE, image.image->buf());
		pixel_alignment_post_send(dim.cpp);
		if (generate_mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);
		return texture;
	}

	void nsvg_manually_generate_mipmaps(const VectorImageRes& image, const NSVGAbstract& abstract, const NSVGContext& context)
	{
		if (image.image->dim().w <= 1 && image.image->dim().h <= 1)
			return;
		float scale = image.scale;
		GLint level = 0;
		pixel_alignment_pre_send(image.image->dim().cpp);
		while (true)
		{
			{
				float new_scale = scale / 2;
				if (approx(new_scale, scale))
					break;
				scale = new_scale;
			}

			auto img = context.rasterize(abstract, scale);
			const auto& dim = img.dim();
			glTexImage2D(GL_TEXTURE_2D, ++level, texture_internal_format(dim.cpp), dim.w, dim.h, 0, texture_format(dim.cpp), GL_UNSIGNED_BYTE, img.buf());
			if (img.dim().w == 1 && img.dim().h == 1)
				break;
		}
		pixel_alignment_post_send(image.image->dim().cpp);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level);
	}
}
