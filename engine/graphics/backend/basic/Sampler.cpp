#include "Sampler.h"

namespace oly::graphics
{
	Sampler::Sampler()
	{
		glCreateSamplers(1, &id_);
	}

	Sampler::Sampler(Sampler&& other) noexcept
		: id_(other.id_)
	{
		other.id_ = 0;
	}

	Sampler::~Sampler()
	{
		glDeleteSamplers(1, &id_);
	}

	Sampler& Sampler::operator=(Sampler&& other) noexcept
	{
		if (this != &other)
		{
			glDeleteSamplers(1, &id_);
			id_ = other.id_;
			other.id_ = 0;
		}
		return *this;
	}

    Sampler::operator GLuint () const
    {
        return id_;
    }

	void Sampler::set_parameter_i(GLenum param, GLint value) const
	{
		glSamplerParameteri(id_, param, value);
	}

	void Sampler::set_parameter_iv(GLenum param, const GLint* values) const
	{
		glSamplerParameteriv(id_, param, values);
	}

	void Sampler::set_parameter_f(GLenum param, GLfloat value) const
	{
		glSamplerParameterf(id_, param, value);
	}

	void Sampler::set_parameter_fv(GLenum param, const GLfloat* values) const
	{
		glSamplerParameterfv(id_, param, values);
	}
}
