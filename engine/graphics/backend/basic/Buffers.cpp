#include "Buffers.h"

namespace oly::graphics
{
    void internal::swap_gl_buffer(GLBuffer& buffer, GLuint& id)
    {
        std::swap(buffer.id_, id);
    }

    GLBuffer::GLBuffer()
    {
        glCreateBuffers(1, &id_);
    }

    GLBuffer::GLBuffer(GLBuffer&& o) noexcept
        : id_(o.id_)
    {
        o.id_ = 0;
    }

    GLBuffer::~GLBuffer()
    {
        glDeleteBuffers(1, &id_);
    }

    GLBuffer& GLBuffer::operator=(GLBuffer&& o) noexcept
    {
        if (this != &o)
        {
            glDeleteBuffers(1, &id_);
            id_ = o.id_;
            o.id_ = 0;
        }
        return *this;
    }

    GLBuffer::operator GLuint () const
    {
        return id_;
    }

    GLBufferBlock<0>::GLBufferBlock(GLsizei count)
        : ids_(new GLuint[count]), count_(count)
    {
        glCreateBuffers(count_, ids_);
    }

    GLBufferBlock<0>::GLBufferBlock(GLBufferBlock<0>&& o) noexcept
        : ids_(o.ids_), count_(o.count_)
    {
        o.ids_ = nullptr;
        o.count_ = 0;
    }

    GLBufferBlock<0>::~GLBufferBlock()
    {
        glDeleteBuffers(count_, ids_);
        delete[] ids_;
    }

    GLBufferBlock<0>& GLBufferBlock<0>::operator=(GLBufferBlock<0>&& o) noexcept
    {
        if (this != &o)
        {
            glDeleteBuffers(count_, ids_);
            delete[] ids_;
            ids_ = o.ids_;
            count_ = o.count_;
            o.ids_ = nullptr;
            o.count_ = 0;
        }
        return *this;
    }

    GLuint GLBufferBlock<0>::operator[](size_t i) const
    {
        if (i >= count_)
            throw Error(ErrorCode::IndexOutOfRange);

        return ids_[i];
    }

    GLsizei GLBufferBlock<0>::get_count() const
    {
        return count_;
    }
}
