#pragma once

#include <GL/glew.h>

#include <array>

#include "util/Assert.h"

namespace oly
{
	namespace rendering
	{
		class GLBuffer
		{
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
			GLBufferBlock();
			GLBufferBlock(const GLBufferBlock&) = delete;
			GLBufferBlock(GLBufferBlock&&) noexcept;
			~GLBufferBlock();
			GLBufferBlock& operator=(GLBufferBlock&&) noexcept;

			constexpr GLuint operator[](GLsizei i) const;
			static constexpr size_t count = N;
		};

		template<size_t N>
		oly::rendering::GLBufferBlock<N>::GLBufferBlock()
		{
			glCreateBuffers(N, ids.data());
		}

		template<size_t N>
		oly::rendering::GLBufferBlock<N>::GLBufferBlock(GLBufferBlock<N>&& other) noexcept
			: ids(other.ids)
		{
			other.active = false;
		}

		template<size_t N>
		oly::rendering::GLBufferBlock<N>::~GLBufferBlock()
		{
			if (active)
				glDeleteBuffers(N, ids.data());
		}

		template<size_t N>
		oly::rendering::GLBufferBlock<N>& oly::rendering::GLBufferBlock<N>::operator=(GLBufferBlock<N>&& other) noexcept
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

		template<size_t N>
		constexpr GLuint oly::rendering::GLBufferBlock<N>::operator[](GLsizei i) const
		{
			OLY_ASSERT(i >= 0 && i < count);
			return ids[i];
		}
		
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
}
