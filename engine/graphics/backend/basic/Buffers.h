#pragma once

#include "core/base/Errors.h"

#include "external/GL.h"

#include <array>

namespace oly::graphics
{
    class GLBuffer;

    namespace internal
    {
        extern void swap_gl_buffer(GLBuffer& buffer, GLuint& id);
    }

    class GLBuffer
    {
        friend void internal::swap_gl_buffer(GLBuffer&, GLuint&);

    private:
        GLuint id_;

    public:
        GLBuffer();
        GLBuffer(const GLBuffer&) = delete;
        GLBuffer(GLBuffer&&) noexcept;
        ~GLBuffer();
        GLBuffer& operator=(GLBuffer&&) noexcept;

        operator GLuint () const;
    };

    template<size_t N>
    class GLBufferBlock
    {
        static_assert(N > 0);

        std::array<GLuint, N> ids_;

    public:
        GLBufferBlock()
        {
            glCreateBuffers(N, ids_.data());
        }

        GLBufferBlock(const GLBufferBlock&) = delete;

        GLBufferBlock(GLBufferBlock&& o) noexcept
            : ids_(o.ids_)
        {
            o.ids_[0] = 0;
        }

        ~GLBufferBlock()
        {
            if (ids_[0])
                glDeleteBuffers(N, ids_.data());
        }

        GLBufferBlock& operator=(GLBufferBlock&& o) noexcept
        {
            if (this != &o)
            {
                if (ids_[0])
                    glDeleteBuffers(N, ids_.data());

                ids_ = o.ids_;
                o.ids_[0] = 0;
            }

            return *this;
        }

        constexpr GLuint operator[](size_t i) const
        {
            if (i >= N)
                throw Error(ErrorCode::IndexOutOfRange);

            return ids_[i];
        }

        template<size_t i>
        void swap(GLBuffer& Single)
        {
            static_assert(i < N);
            internal::swap_gl_buffer(Single, ids_[i]);
        }
    };

    template<>
    class GLBufferBlock<0>
    {
        GLuint* ids_;
        GLsizei count_;

    public:
        GLBufferBlock(GLsizei count);
        GLBufferBlock(const GLBufferBlock&) = delete;
        GLBufferBlock(GLBufferBlock<0>&&) noexcept;
        ~GLBufferBlock();
        GLBufferBlock<0>& operator=(GLBufferBlock<0>&&) noexcept;

        GLuint operator[](size_t i) const;
        GLsizei get_count() const;
    };
}
