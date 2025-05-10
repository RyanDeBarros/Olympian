#pragma once

#include <array>

#include "external/GL.h"
#include "core/base/Assert.h"

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
		GLuint id;

	public:
		GLBuffer();
		GLBuffer(const GLBuffer&) = delete;
		GLBuffer(GLBuffer&&) noexcept;
		~GLBuffer();
		GLBuffer& operator=(GLBuffer&&) noexcept;

		operator GLuint () const { return id; }

		void mutable_resize(GLsizeiptr new_size, GLenum usage);
		void mutable_resize(GLsizeiptr new_size, GLenum usage, GLsizeiptr old_size);
		void mutable_grow(GLsizeiptr new_size, GLenum usage);
		void mutable_grow(GLsizeiptr new_size, GLenum usage, GLsizeiptr old_size);
	};

	template<size_t N>
	class GLBufferBlock
	{
		bool active = true;
		std::array<GLuint, N> ids;

	public:
		GLBufferBlock()
		{
			glCreateBuffers(N, ids.data());
		}

		GLBufferBlock(const GLBufferBlock&) = delete;

		GLBufferBlock(GLBufferBlock&& other) noexcept
			: ids(other.ids)
		{
			other.active = false; // TODO use ids[0] instead of active?
		}

		~GLBufferBlock()
		{
			if (active)
				glDeleteBuffers(N, ids.data());
		}

		GLBufferBlock& operator=(GLBufferBlock&& other) noexcept
		{
			if (this != &other)
			{
				if (active)
					glDeleteBuffers(N, ids.data());
				ids = other.ids;
				other.active = false;
			}
			return *this;
		}

		constexpr GLuint operator[](GLsizei i) const
		{
			OLY_ASSERT(i >= 0 && i < N);
			return ids[i];
		}

		template<size_t i>
		void swap(GLBuffer& single)
		{
			static_assert(i < N);
			internal::swap_gl_buffer(single, ids[i]);
		}
	};

	template<>
	class GLBufferBlock<0>
	{
		GLuint* ids;
		GLsizei count;

	public:
		GLBufferBlock(GLsizei count);
		GLBufferBlock(const GLBufferBlock&) = delete;
		GLBufferBlock(GLBufferBlock<0>&&) noexcept;
		~GLBufferBlock();
		GLBufferBlock<0>& operator=(GLBufferBlock<0>&&) noexcept;

		GLuint operator[](GLsizei i) const;
		GLsizei get_count() const { return count; }
	};
}
