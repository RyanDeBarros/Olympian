#pragma once

#include "graphics/backend/basic/Buffers.h"

#include "graphics/backend/specialized/Mutability.h"

#include <imp/fixed_vector.hpp>

#include <set>

namespace oly::graphics
{
    enum class BufferSendType
    {
        SubData,
        Map
    };

    template<typename Iterator, std::unsigned_integral IndexType>
    inline void batch_send(
        Iterator dirty_begin,
        Iterator dirty_end,
        GLuint buf,
        const void* cpudata_,
        GLsizeiptr struct_size,
        IndexType pos_end,
        BufferSendType send_type = BufferSendType::SubData)
    {
        const std::byte* data = (const std::byte*)cpudata_;
        switch (send_type)
        {
        case BufferSendType::SubData:
        {
            GLintptr offset = 0;
            GLsizeiptr size = 0;

            for (auto iter = dirty_begin; iter != dirty_end; ++iter)
            {
                if (*iter >= pos_end)
                    continue;
                if (*iter * struct_size == offset + size)
                    size += struct_size;
                else
                {
                    glNamedBufferSubData(buf, offset, size, data + offset);
                    offset = *iter * struct_size;
                    size = struct_size;
                }
            }
            
            glNamedBufferSubData(buf, offset, size, data + offset);
            break;
        }
        case BufferSendType::Map:
        {
            std::byte* gpu_buf = (std::byte*)glMapNamedBuffer(buf, GL_WRITE_ONLY);
            GLintptr offset = 0;
            GLsizeiptr size = 0;
            
            for (auto iter = dirty_begin; iter != dirty_end; ++iter)
            {
                if (*iter >= pos_end)
                    continue;
                if (*iter * struct_size == offset + size)
                    size += struct_size;
                else
                {
                    memcpy(gpu_buf + offset, data + offset, size);
                    offset = *iter * struct_size;
                    size = struct_size;
                }
            }

            memcpy(gpu_buf + offset, data + offset, size);
            glUnmapNamedBuffer(buf);
            break;
        }
        }
    }

    template<typename Iterator, typename StructType>
    inline void batch_send(
        Iterator dirty_begin,
        Iterator dirty_end,
        GLuint buf_,
        const imp::fixed_vector<StructType>& cpudata_,
        BufferSendType send_type = BufferSendType::SubData)
    {
        batch_send(dirty_begin, dirty_end, buf_, cpudata_.data(), sizeof(StructType), cpudata_.size(), send_type);
    }

    template<typename StructType, Mutability M>
    class CPUSideBuffer
    {
    public:
        using VectorAlias = std::conditional_t<
            M == Mutability::Immutable,
            imp::fixed_vector<StructType>,
            std::vector<StructType>>;

        using StructAlias = StructType;

    protected:
        VectorAlias cpudata_;
        GLBuffer buf_;

    public:
        CPUSideBuffer(size_t size = 0, const StructType& default_value = StructType())
            : cpudata_(size, default_value)
        {
        }

        const VectorAlias& vector() const
        {
            return cpudata_;
        }
        
        VectorAlias& vector()
        {
            return cpudata_;
        }
        
        GLuint buffer() const
        {
            return buf_;
        }

        void init(GLbitfield flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT) const
            requires (M == Mutability::Immutable)
        {
            glNamedBufferStorage(
                buf_,
                cpudata_.size() * sizeof(StructType),
                cpudata_.data(),
                flags);
        }

        void init(GLenum usage = GL_DYNAMIC_DRAW) const requires (M == Mutability::Mutable)
        {
            glNamedBufferData(
                buf_,
                cpudata_.size() * sizeof(StructType),
                cpudata_.data(),
                usage);
        }
    };

    template<typename StructType, std::unsigned_integral IndexType, Mutability M>
    class LazyBuffer
        : public CPUSideBuffer<StructType, M>
    {
        mutable std::set<IndexType> dirty_;

    public:
        using CPUSideBuffer<StructType, M>::CPUSideBuffer;

        void lazy_send(IndexType pos) const
        {
            dirty_.insert(pos);
        }

        void clear_dirty() const
        {
            dirty_.clear();
        }

        void flush(BufferSendType send_type = BufferSendType::SubData) const
        {
            batch_send(
                dirty_.begin(),
                dirty_.end(),
                this->buf_,
                this->cpudata_.data(),
                sizeof(StructType),
                static_cast<IndexType>(this->cpudata_.size()),
                send_type);

            dirty_.clear();
        }
    };
}
