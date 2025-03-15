#pragma once

#include <GL/glew.h>

#include <vector>
#include <array>
#include <memory>

namespace oly
{
	namespace rendering
	{
		class GLBuffer;
		typedef std::shared_ptr<GLBuffer> GLBufferRes;

		class GLBuffer
		{
			GLuint id;

			struct static_id { GLuint id; };
			GLBuffer(static_id sid) : id(sid.id) {}

		public:
			GLBuffer();
			GLBuffer(const GLBuffer&) = delete;
			GLBuffer(GLBuffer&&) noexcept;
			~GLBuffer();
			GLBuffer& operator=(GLBuffer&&) noexcept;

			operator GLuint () const { return id; }
			static GLBufferRes from_id(GLuint id) { return GLBufferRes(new GLBuffer(static_id{ id })); }
		};

		std::vector<GLBufferRes> gen_bulk_buffers(GLsizei n);

		template<GLsizei N>
		std::array<GLBufferRes, N> gen_bulk_buffers()
		{
			std::array<GLuint, N> buffers;
			glGenBuffers(N, buffers.data());
			std::array<GLBufferRes, N> wrapped_buffers;
			for (GLsizei i = 0; i < N; ++i)
				wrapped_buffers[i] = GLBuffer::from_id(buffers[i]);
			return wrapped_buffers;
		}
	}
}
