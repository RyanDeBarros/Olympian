#pragma once

#include "core/base/Errors.h"

#include "graphics/backend/basic/Buffers.h"

#include "graphics/backend/specialized/Mutability.h"

#include <imp/util.hpp>

namespace oly::graphics
{
    template<Mutability M>
    class LightweightBuffer;

    template<>
    class LightweightBuffer<Mutability::Mutable>
    {
        GLBuffer buf_;
        mutable GLsizeiptr size_ = 0;
        GLsizeiptr max_size_ = 0;

    public:
        LightweightBuffer(GLsizeiptr max_size_in_bytes, GLsizeiptr size_in_bytes = 0, GLenum usage = GL_DYNAMIC_DRAW);

    public:
        GLuint buffer() const;
        GLsizeiptr get_size() const;
        size_t get_max_size() const;

        template<typename StructType>
        void send(GLintptr pos, const StructType& obj) const
        {
            assert_in_range(pos);
            glNamedBufferSubData(
                buf_,
                pos * sizeof(StructType),
                sizeof(StructType),
                &obj);
        }

        template<typename StructType, typename MemberType>
        void send(GLintptr pos, MemberType StructType::* member, const MemberType& obj) const
        {
            assert_in_range(pos);
            glNamedBufferSubData(
                buf_,
                pos * sizeof(StructType) + imp::member_offset(member),
                sizeof(MemberType),
                &obj);
        }

        template<typename StructType>
        StructType receive(GLintptr pos) const
        {
            assert_in_range(pos);
            StructType obj;
            glGetNamedBufferSubData(
                buf_,
                pos * sizeof(StructType),
                sizeof(StructType),
                &obj);
            return obj;
        }

        template<typename StructType, typename MemberType>
        MemberType receive(GLintptr pos, MemberType StructType::* member) const
        {
            assert_in_range(pos);
            MemberType obj;
            glGetNamedBufferSubData(
                buf_,
                pos * sizeof(StructType) + imp::member_offset(member),
                sizeof(MemberType),
                &obj);
            return obj;
        }

        void resize_empty(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) const;
        void resize(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW);

    private:
        void set_size(GLsizeiptr sz) const;
        void assert_in_range(GLintptr pos) const;
    };

    template<>
    class LightweightBuffer<Mutability::Immutable>
    {
        GLBuffer buf_;
        mutable GLsizeiptr size_ = 0;

    public:
        LightweightBuffer(GLsizeiptr size_in_bytes, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT);

    public:
        GLuint buffer() const;
        GLsizeiptr get_size() const;

        template<typename StructType>
        void send(GLintptr pos, const StructType& obj) const
        {
            assert_in_range(pos);
            glNamedBufferSubData(
                buf_,
                pos * sizeof(StructType),
                sizeof(StructType),
                &obj);
        }

        template<typename StructType, typename MemberType>
        void send(GLintptr pos, MemberType StructType::* member, const MemberType& obj) const
        {
            assert_in_range(pos);
            glNamedBufferSubData(
                buf_,
                pos * sizeof(StructType) + imp::member_offset(member),
                sizeof(MemberType),
                &obj);
        }

        template<typename StructType>
        StructType receive(GLintptr pos) const
        {
            assert_in_range(pos);
            StructType obj;
            glGetNamedBufferSubData(
                buf_,
                pos * sizeof(StructType),
                sizeof(StructType),
                &obj);
            return obj;
        }

        template<typename StructType, typename MemberType>
        MemberType receive(GLintptr pos, MemberType StructType::* member) const
        {
            assert_in_range(pos);
            MemberType obj;
            glGetNamedBufferSubData(
                buf_,
                pos * sizeof(StructType) + imp::member_offset(member),
                sizeof(MemberType),
                &obj);
            return obj;
        }

        void force_resize_empty(GLsizeiptr size_in_bytes, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT);
        void force_resize(GLsizeiptr size_in_bytes, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT);

    private:
        void assert_in_range(GLintptr pos) const;
    };

    constexpr GLsizeiptr kShaderStorageMaxBufferSize = 134'217'728; // 128 MiB

    template<Mutability M>
    struct LightweightSSBO
        : public LightweightBuffer<M>
    {
        using LightweightBuffer<M>::LightweightBuffer;

        void bind_base(GLuint index) const { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, this->buffer()); }
    };

    template<Mutability M>
    struct LightweightUBO
        : public LightweightBuffer<M>
    {
        using LightweightBuffer<M>::LightweightBuffer;

        void bind_base(GLuint index) const { glBindBufferBase(GL_UNIFORM_BUFFER, index, this->buffer()); }
    };
}
