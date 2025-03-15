#pragma once

#include <GL/glew.h>

#include <vector>
#include <array>
#include <memory>

namespace oly
{
	namespace rendering
	{
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
			static std::shared_ptr<GLBuffer> from_id(GLuint id) { return std::shared_ptr<GLBuffer>(new GLBuffer(static_id{ id })); }
		};

		std::vector<std::shared_ptr<GLBuffer>> gen_bulk_buffers(GLsizei n);

		template<GLsizei N>
		std::array<std::shared_ptr<GLBuffer>, N> gen_bulk_buffers()
		{
			std::array<GLuint, N> buffers;
			glGenBuffers(N, buffers.data());
			std::array<std::shared_ptr<GLBuffer>, N> wrapped_buffers;
			for (GLsizei i = 0; i < N; ++i)
				wrapped_buffers[i] = GLBuffer::from_id(buffers[i]);
			return wrapped_buffers;
		}
	}
}
