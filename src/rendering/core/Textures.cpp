#include "Textures.h"

#include <stb/stb_image.h>

#include "util/IO.h"
#include "util/Errors.h"
#include "util/Assert.h"
#include "util/Approximate.h"
#include "../../Olympian.h"

namespace oly
{
	namespace rendering
	{
		Sampler::Sampler()
		{
			glCreateSamplers(1, &id);
		}

		Sampler::Sampler(Sampler&& other) noexcept
			: id(other.id)
		{
			other.id = 0;
		}

		Sampler::~Sampler()
		{
			glDeleteSamplers(1, &id);
		}

		Sampler& Sampler::operator=(Sampler&& other) noexcept
		{
			if (this != &other)
			{
				glDeleteSamplers(1, &id);
				id = other.id;
				other.id = 0;
			}
			return *this;
		}

		void Sampler::set_parameter_i(GLenum param, GLint value) const
		{
			glSamplerParameteri(id, param, value);
		}

		void Sampler::set_parameter_iv(GLenum param, const GLint* values) const
		{
			glSamplerParameteriv(id, param, values);
		}

		void Sampler::set_parameter_f(GLenum param, GLfloat value) const
		{
			glSamplerParameterf(id, param, value);
		}

		void Sampler::set_parameter_fv(GLenum param, const GLfloat* values) const
		{
			glSamplerParameterfv(id, param, values);
		}

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

		GLenum tex::internal_format(int cpp)
		{
			return cpp == 1 ? GL_R8
				: cpp == 2 ? GL_RG8
				: cpp == 3 ? GL_RGB8
				: GL_RGBA8;
		}

		GLenum tex::format(int cpp)
		{
			return cpp == 1 ? GL_RED
				: cpp == 2 ? GL_RG
				: cpp == 3 ? GL_RGB
				: GL_RGBA;
		}

		void tex::pixel_alignment(int cpp)
		{
			switch (cpp)
			{
			case 1:
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				break;
			case 2:
				glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
				break;
			case 3:
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				break;
			case 4:
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
				break;
			}
		}

		Image::Image(const char* filepath)
		{
			_buf = stbi_load(filepath, &_dim.w, &_dim.h, &_dim.cpp, _dim.cpp);
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

		TextureRes load_texture_2d(const Image& image, bool generate_mipmaps)
		{
			TextureRes texture = std::make_shared<Texture>(GL_TEXTURE_2D);;
			glBindTexture(GL_TEXTURE_2D, *texture);
			const auto& dim = image.dim();
			tex::pixel_alignment(dim.cpp);
			glTexImage2D(GL_TEXTURE_2D, 0, tex::internal_format(dim.cpp), dim.w, dim.h, 0, tex::format(dim.cpp), GL_UNSIGNED_BYTE, image.buf());
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

		Anim::Anim(const char* filepath)
			: _dim(std::make_shared<AnimDimensions>())
		{
			// TODO if file extension is .gif
			//{
				auto full_content = io::read_file_uc(filepath);
				int* delays;
				int frames;
				_buf = stbi_load_gif_from_memory(full_content.data(), (int)full_content.size(), &delays, &_dim->w, &_dim->h, &frames, &_dim->cpp, _dim->cpp);
				_dim->set_delays(delays, frames);
				stbi_image_free(delays);
			//}
			// TODO else spritesheet
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

		TextureRes load_texture_2d_array(const Anim& anim, bool generate_mipmaps)
		{
			TextureRes texture = std::make_shared<Texture>(GL_TEXTURE_2D_ARRAY);
			glBindTexture(GL_TEXTURE_2D_ARRAY, *texture);
			const auto& dim = anim.dim().lock();
			tex::pixel_alignment(dim->cpp);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, tex::internal_format(dim->cpp), dim->w, dim->h, dim->frames(), 0, tex::format(dim->cpp), GL_UNSIGNED_BYTE, nullptr);
			for (GLuint i = 0; i < dim->frames(); ++i)
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, dim->w, dim->h, 1, tex::format(dim->cpp), GL_UNSIGNED_BYTE, anim.buf() + i * dim->w * dim->h * dim->cpp);
			if (generate_mipmaps)
				glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
			return texture;
		}

		AnimFrameFormat setup_anim_frame_format(const AnimDimensions& dim, float speed, GLuint starting_frame)
		{
			OLY_ASSERT(dim.uniform());
			return { starting_frame, dim.frames(), 0.0f, speed * 0.01f * dim.delay() };
		}

		AnimFrameFormat setup_anim_frame_format(const TextureRegistry* texture_registry, const std::string& texture_name, float speed, GLuint starting_frame)
		{
			return setup_anim_frame_format(*texture_registry->get_anim_dimensions(texture_name).lock(), speed, starting_frame);
		}

		AnimFrameFormat setup_anim_frame_format(const Context* context, const std::string& texture_name, float speed, GLuint starting_frame)
		{
			return setup_anim_frame_format(&context->texture_registry(), texture_name, speed, starting_frame);
		}

		AnimFrameFormat setup_anim_frame_format_single(const AnimDimensions& dim, GLuint frame)
		{
			return { frame, dim.frames(), 0.0f, 0.0f };
		}

		AnimFrameFormat setup_anim_frame_format_single(const TextureRegistry* texture_registry, const std::string& texture_name, GLuint frame)
		{
			return setup_anim_frame_format_single(*texture_registry->get_anim_dimensions(texture_name).lock(), frame);
		}

		AnimFrameFormat setup_anim_frame_format_single(const Context* context, const std::string& texture_name, GLuint frame)
		{
			return setup_anim_frame_format_single(&context->texture_registry(), texture_name, frame);
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

		TextureRes load_nsvg_texture_2d(const VectorImageRes& image, bool generate_mipmaps)
		{
			TextureRes texture = std::make_shared<Texture>(GL_TEXTURE_2D);;
			glBindTexture(GL_TEXTURE_2D, *texture);
			const auto& dim = image.image->dim();
			tex::pixel_alignment(dim.cpp);
			glTexImage2D(GL_TEXTURE_2D, 0, tex::internal_format(dim.cpp), dim.w, dim.h, 0, tex::format(dim.cpp), GL_UNSIGNED_BYTE, image.image->buf());
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
				tex::pixel_alignment(dim.cpp);
				glTexImage2D(GL_TEXTURE_2D, ++level, tex::internal_format(dim.cpp), dim.w, dim.h, 0, tex::format(dim.cpp), GL_UNSIGNED_BYTE, img.buf());
				if (img.dim().w == 1 && img.dim().h == 1)
					break;
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level);
		}
	}
}
