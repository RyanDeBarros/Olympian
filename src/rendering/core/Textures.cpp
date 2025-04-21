#include "Textures.h"

#include <stb/stb_image.h>

#include "util/IO.h"
#include "util/Errors.h"
#include "util/Assert.h"

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

		ImageTextureRes load_texture_2d(const char* filename, bool generate_mipmaps)
		{
			ImageTextureRes img;
			img.image = std::make_shared<Image>(filename);
			img.texture = std::make_shared<Texture>(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, *img.texture);
			const auto& dim = img.image->dim();
			tex::pixel_alignment(dim.cpp);
			glTexImage2D(GL_TEXTURE_2D, 0, tex::internal_format(dim.cpp), dim.w, dim.h, 0, tex::format(dim.cpp), GL_UNSIGNED_BYTE, img.image->buf());
			if (generate_mipmaps)
				glGenerateMipmap(GL_TEXTURE_2D);
			return img;
		}

		void GIFDimensions::set_delays(int* new_delays, unsigned int num_frames)
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
				if (std::abs(delays[i] - delay) > gif_delay_epsilon)
					single = false;
			}
			if (single)
			{
				delays.resize(1);
				_frames = num_frames;
			}
		}

		int GIFDimensions::delay(unsigned int frame) const
		{
			if (frame >= frames())
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return uniform() ? delays[0] : delays[frame];
		}

		GIF::GIF(const char* filepath)
		{
			auto full_content = io::read_file_uc(filepath);
			int* delays;
			int frames;
			_buf = stbi_load_gif_from_memory(full_content.data(), (int)full_content.size(), &delays, &_dim.w, &_dim.h, &frames, &_dim.cpp, _dim.cpp);
			_dim.set_delays(delays, frames);
			stbi_image_free(delays);
		}

		GIF::GIF(GIF&& other) noexcept
			: _buf(other._buf), _dim(std::move(other._dim))
		{
			other._buf = nullptr;
		}

		GIF::~GIF()
		{
			stbi_image_free(_buf);
		}

		GIF& GIF::operator=(GIF&& other) noexcept
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

		GIFTextureRes load_texture_2d_array(const char* filename, bool generate_mipmaps)
		{
			GIFTextureRes gif;
			gif.gif = std::make_shared<GIF>(filename);
			gif.texture = std::make_shared<Texture>(GL_TEXTURE_2D_ARRAY);
			glBindTexture(GL_TEXTURE_2D_ARRAY, *gif.texture);
			const auto& dim = gif.gif->dim();
			tex::pixel_alignment(dim.cpp);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, tex::internal_format(dim.cpp), dim.w, dim.h, dim.frames(), 0, tex::format(dim.cpp), GL_UNSIGNED_BYTE, nullptr);
			for (GLuint i = 0; i < dim.frames(); ++i)
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, dim.w, dim.h, 1, tex::format(dim.cpp), GL_UNSIGNED_BYTE, gif.gif->buf() + i * dim.w * dim.h * dim.cpp);
			if (generate_mipmaps)
				glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
			return gif;
		}

		GIFFrameFormat setup_gif_frame_format(const GIFDimensions& dim, float speed, GLuint starting_frame)
		{
			OLY_ASSERT(dim.uniform());
			return { starting_frame, dim.frames(), 0.0f, speed * 0.01f * dim.delay() };
		}

		GIFFrameFormat setup_gif_frame_format_single(const GIFDimensions& dim, GLuint frame)
		{
			return { frame, dim.frames(), 0.0f, 0.0f };
		}
	}
}
