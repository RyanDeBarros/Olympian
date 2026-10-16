#include "LightweightBuffers.h"

namespace oly::graphics
{
    LightweightBuffer<Mutability::Mutable>::LightweightBuffer(
        GLsizeiptr max_size_in_bytes,
        GLsizeiptr size_in_bytes,
        GLenum usage)
        : max_size_(max_size_in_bytes)
    {
        if (max_size_ < 0)
            throw Error(ErrorCode::InvalidSize);
        
        set_size(size_in_bytes);
        glNamedBufferData(buf_, size_, nullptr, usage);
    }

    GLuint LightweightBuffer<Mutability::Mutable>::buffer() const
    {
        return buf_;
    }

    GLsizeiptr LightweightBuffer<Mutability::Mutable>::get_size() const
    {
        return size_;
    }

    size_t LightweightBuffer<Mutability::Mutable>::get_max_size() const
    {
        return max_size_;
    }

    void LightweightBuffer<Mutability::Mutable>::resize_empty(GLsizeiptr size_in_bytes, GLenum usage) const
    {
        set_size(size_in_bytes);
        glNamedBufferData(buf_, size_, nullptr, usage);
    }

    void LightweightBuffer<Mutability::Mutable>::resize(GLsizeiptr size_in_bytes, GLenum usage)
    {
        if (size_in_bytes != size_)
        {
            GLsizeiptr old_size = size_;
            set_size(size_in_bytes);
            GLBuffer new_buf;
            glNamedBufferData(new_buf, size_, nullptr, usage);
            glCopyNamedBufferSubData(buf_, new_buf, 0, 0, std::min(size_, old_size));
            buf_ = std::move(new_buf);
        }
    }

    void LightweightBuffer<Mutability::Mutable>::set_size(GLsizeiptr sz) const
    {
        if (sz < 0 || sz > max_size_)
            throw Error(ErrorCode::InvalidSize);
        
        size_ = sz;
    }

    void LightweightBuffer<Mutability::Mutable>::assert_in_range(GLintptr pos) const
    {
        if (pos < 0 || pos > size_ - 1)
            throw Error(ErrorCode::IndexOutOfRange);
    }

    LightweightBuffer<Mutability::Immutable>::LightweightBuffer(GLsizeiptr size_in_bytes, GLbitfield flags)
        : size_(size_in_bytes)
    {
        if (size_ <= 0)
            throw Error(ErrorCode::InvalidSize);

        glNamedBufferStorage(buf_, size_, nullptr, flags);
    }

    GLuint LightweightBuffer<Mutability::Immutable>::buffer() const
    {
        return buf_;
    }
    
    GLsizeiptr LightweightBuffer<Mutability::Immutable>::get_size() const
    {
        return size_;
    }

    void LightweightBuffer<Mutability::Immutable>::force_resize_empty(GLsizeiptr size_in_bytes, GLbitfield flags)
    {
        if (size_in_bytes <= 0)
            throw Error(ErrorCode::InvalidSize);

        size_ = size_in_bytes;
        GLBuffer new_buf;
        glNamedBufferStorage(new_buf, size_, nullptr, flags);
        buf_ = std::move(new_buf);
    }

    void LightweightBuffer<Mutability::Immutable>::force_resize(GLsizeiptr size_in_bytes, GLbitfield flags)
    {
        if (size_in_bytes <= 0)
            throw Error(ErrorCode::InvalidSize);

        if (size_in_bytes != size_)
        {
            GLsizeiptr old_size = size_;
            size_ = size_in_bytes;
            GLBuffer new_buf;
            glNamedBufferStorage(new_buf, size_, nullptr, flags);
            glCopyNamedBufferSubData(buf_, new_buf, 0, 0, std::min(size_, old_size));
            buf_ = std::move(new_buf);
        }
    }

    void LightweightBuffer<Mutability::Immutable>::assert_in_range(GLintptr pos) const
    {
        if (pos < 0 || pos > size_ - 1)
            throw Error(ErrorCode::IndexOutOfRange);
    }
}
