#pragma once

#include <GL/glew.h>

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
		};

		class GLBufferBlock
		{
			GLuint* ids;
			GLsizei count;

		public:
			GLBufferBlock(GLsizei count);
			GLBufferBlock(const GLBufferBlock&) = delete;
			GLBufferBlock(GLBufferBlock&&) noexcept;
			~GLBufferBlock();
			GLBufferBlock& operator=(GLBufferBlock&&) noexcept;

			GLuint operator[](GLsizei i) const;
			GLsizei get_count() const { return count; }
		};
	}
}
