#include "Sampler.h"

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
	}
}
