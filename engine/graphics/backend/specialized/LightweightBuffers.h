#pragma once

#include "external/GL.h"
#include "core/types/Meta.h"
#include "graphics/backend/basic/Buffers.h"
#include "graphics/backend/specialized/Mutability.h"

namespace oly::graphics
{
	template<Mutability M>
	class LightweightBuffer
	{
		GLBuffer buf;
		mutable GLsizeiptr size = 0;
		std::conditional_t<M == Mutability::MUTABLE, GLsizeiptr, Empty> max_size = {};

	public:
		LightweightBuffer(GLsizeiptr size_in_bytes, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT) requires (M == Mutability::IMMUTABLE)
			: size(size_in_bytes)
		{
			if (size < 0)
				throw Error(ErrorCode::INVALID_SIZE);
			glNamedBufferStorage(buf, size, nullptr, flags);
		}

		LightweightBuffer(GLsizeiptr size_in_bytes, GLsizeiptr max_size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) requires (M == Mutability::MUTABLE)
			: max_size(max_size_in_bytes)
		{
			if (max_size < 0)
				throw Error(ErrorCode::INVALID_SIZE);
			set_size(size_in_bytes);
			glNamedBufferData(buf, size, nullptr, usage);
		}

		GLuint buffer() const { return buf; }
		
		GLsizeiptr get_size() const { return size; }

		size_t get_max_size() const { return max_size; }
		
	private:
		void set_size(GLsizeiptr sz) requires (M == Mutability::MUTABLE)
		{
			if (sz < 0 || sz > max_size)
				throw Error(ErrorCode::INVALID_SIZE);
			size = sz;
		}

		void assert_in_range(GLintptr pos) const
		{
			if (pos < 0 || pos > size - 1)
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}

	public:
		template<typename StructType>
		void send(GLintptr pos, const StructType& obj) const
		{
			assert_in_range(pos);
			glNamedBufferSubData(buf, pos * sizeof(StructType), sizeof(StructType), &obj);
		}
		
		template<typename StructType, typename MemberType>
		void send(GLintptr pos, MemberType StructType::* member, const MemberType& obj) const
		{
			assert_in_range(pos);
			glNamedBufferSubData(buf, pos * sizeof(StructType) + member_offset(member), sizeof(MemberType), &obj);
		}
		
		template<typename StructType>
		StructType receive(GLintptr pos) const
		{
			assert_in_range(pos);
			StructType obj;
			glGetNamedBufferSubData(buf, pos * sizeof(StructType), sizeof(StructType), &obj);
			return obj;
		}

		template<typename StructType, typename MemberType>
		MemberType receive(GLintptr pos, MemberType StructType::* member) const
		{
			assert_in_range(pos);
			MemberType obj;
			glGetNamedBufferSubData(buf, pos * sizeof(StructType) + member_offset(member), sizeof(MemberType), &obj);
			return obj;
		}

		void resize_empty(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) const requires (M == Mutability::MUTABLE)
		{
			set_size(size_in_bytes);
			glNamedBufferData(buf, size, nullptr, usage);
		}
		
		void resize(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) requires (M == Mutability::MUTABLE)
		{
			if (size_in_bytes != size)
			{
				GLsizeiptr old_size = size;
				set_size(size_in_bytes);
				GLBuffer new_buf;
				glNamedBufferData(new_buf, size, nullptr, usage);
				glCopyNamedBufferSubData(buf, new_buf, 0, 0, std::min(size, old_size));
				buf = std::move(new_buf);
			}
		}
	};

	constexpr GLsizeiptr SHADER_STORAGE_MAX_BUFFER_SIZE = 134'217'728; // 128 MiB

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
