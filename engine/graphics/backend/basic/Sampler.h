#pragma once

#include "core/base/SmartReference.h"

#include "external/GL.h"

namespace oly::graphics
{
    class Sampler
    {
        GLuint id_ = 0;

    public:
        Sampler();
        Sampler(const Sampler&) = delete;
        Sampler(Sampler&&) noexcept;
        ~Sampler();
        Sampler& operator=(Sampler&&) noexcept;

        operator GLuint () const;

        void set_parameter_i(GLenum param, GLint value) const;
        void set_parameter_iv(GLenum param, const GLint* values) const;
        void set_parameter_f(GLenum param, GLfloat value) const;
        void set_parameter_fv(GLenum param, const GLfloat* values) const;
    };

    using SamplerRef = SmartReference<Sampler>;
}
