#pragma once

#include "external/GL.h"
#include "core/types/Meta.h"
#include "graphics/backend/basic/Buffers.h"
#include "graphics/backend/specialized/Mutability.h"
#include "core/base/Errors.h"

namespace oly::graphics
{
	template<Mutability M>
	class LightweightBuffer;

	template<>
	class LightweightBuffer<Mutability::Mutable>
	{
		GLBuffer buf;
		mutable GLsizeiptr size = 0;
		GLsizeiptr max_size = 0;

	public:
		LightweightBuffer(GLsizeiptr max_size_in_bytes, GLsizeiptr size_in_bytes = 0, GLenum usage = GL_DYNAMIC_DRAW)
			: max_size(max_size_in_bytes)
		{
			if (max_size < 0)
				throw Error(ErrorCode::InvalidSize);
			set_size(size_in_bytes);
			glNamedBufferData(buf, size, nullptr, usage);
		}

		GLuint buffer() const { return buf; }
		
		GLsizeiptr get_size() const { return size; }

		size_t get_max_size() const { return max_size; }
		
	private:
		void set_size(GLsizeiptr sz) const
		{
			if (sz < 0 || sz > max_size)
				throw Error(ErrorCode::InvalidSize);
			size = sz;
		}

		void assert_in_range(GLintptr pos) const
		{
			if (pos < 0 || pos > size - 1)
				throw Error(ErrorCode::IndexOutOfRange);
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

		void resize_empty(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) const
		{
			set_size(size_in_bytes);
			glNamedBufferData(buf, size, nullptr, usage);
		}
		
		void resize(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW)
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

	template<>
	class LightweightBuffer<Mutability::Immutable>
	{
		GLBuffer buf;
		mutable GLsizeiptr size = 0;

	public:
		LightweightBuffer(GLsizeiptr size_in_bytes, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT)
			: size(size_in_bytes)
		{
			if (size <= 0)
				throw Error(ErrorCode::InvalidSize);
			glNamedBufferStorage(buf, size, nullptr, flags);
		}

		GLuint buffer() const { return buf; }

		GLsizeiptr get_size() const { return size; }

	private:
		void assert_in_range(GLintptr pos) const
		{
			if (pos < 0 || pos > size - 1)
				throw Error(ErrorCode::IndexOutOfRange);
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

		void force_resize_empty(GLsizeiptr size_in_bytes, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT)
		{
			if (size_in_bytes <= 0)
				throw Error(ErrorCode::InvalidSize);

			size = size_in_bytes;
			GLBuffer new_buf;
			glNamedBufferStorage(new_buf, size, nullptr, flags);
			buf = std::move(new_buf);
		}

		void force_resize(GLsizeiptr size_in_bytes, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT)
		{
			if (size_in_bytes <= 0)
				throw Error(ErrorCode::InvalidSize);

			if (size_in_bytes != size)
			{
				GLsizeiptr old_size = size;
				size = size_in_bytes;
				GLBuffer new_buf;
				glNamedBufferStorage(new_buf, size, nullptr, flags);
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
