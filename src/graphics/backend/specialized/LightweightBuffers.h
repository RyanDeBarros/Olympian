#pragma once

#include "external/GL.h"
#include "core/types/Meta.h"
#include "graphics/backend/basic/Buffers.h"
#include "graphics/backend/specialized/Mutability.h"

namespace oly
{
	namespace rendering
	{
		template<Mutability M>
		class LightweightBuffer
		{
			GLBuffer buf;
			mutable GLsizeiptr size = 0;

		public:
			LightweightBuffer(GLsizeiptr size_in_bytes, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT) requires (M == Mutability::IMMUTABLE)
				: size(size_in_bytes) {
				glNamedBufferStorage(buf, size, nullptr, flags);
			}
			LightweightBuffer(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) requires (M == Mutability::MUTABLE)
				: size(size_in_bytes) {
				glNamedBufferData(buf, size, nullptr, usage);
			}

			GLuint buffer() const { return buf; }
			GLsizeiptr get_size() const { return size; }
			template<typename StructType>
			void send(GLintptr pos, const StructType& obj) const
			{
				glNamedBufferSubData(buf, pos * sizeof(StructType), sizeof(StructType), &obj);
			}
			template<typename StructType, typename MemberType>
			void send(GLintptr pos, MemberType StructType::* member, const MemberType& obj) const
			{
				glNamedBufferSubData(buf, pos * sizeof(StructType) + member_offset(member), sizeof(MemberType), &obj);
			}
			template<typename StructType>
			StructType receive(GLintptr pos) const
			{
				StructType obj;
				glGetNamedBufferSubData(buf, pos * sizeof(StructType), sizeof(StructType), &obj);
				return obj;
			}
			template<typename StructType, typename MemberType>
			MemberType receive(GLintptr pos, MemberType StructType::* member) const
			{
				MemberType obj;
				glGetNamedBufferSubData(buf, pos * sizeof(StructType) + member_offset(member), sizeof(MemberType), &obj);
				return obj;
			}
			void resize_empty(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) const requires (M == Mutability::MUTABLE) { size = size_in_bytes; glNamedBufferData(buf, size, nullptr, usage); }
			void resize(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) requires (M == Mutability::MUTABLE) { buf.mutable_resize(size_in_bytes, usage, size); size = size_in_bytes; }
			void grow(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) requires (M == Mutability::MUTABLE) { if (size_in_bytes > size) { buf.mutable_grow(size_in_bytes, usage, size); size = size_in_bytes; } }
		};

		template<Mutability M>
		struct LightweightSSBO : public LightweightBuffer<M>
		{
			using LightweightBuffer<M>::LightweightBuffer;

			void bind_base(GLuint index) const { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, this->buffer()); }
		};

		template<Mutability M>
		struct LightweightUBO : public LightweightBuffer<M>
		{
			using LightweightBuffer<M>::LightweightBuffer;

			void bind_base(GLuint index) const { glBindBufferBase(GL_UNIFORM_BUFFER, index, this->buffer()); }
		};
	}
}
