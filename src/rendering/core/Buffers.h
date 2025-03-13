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

		public:
			GLBuffer();
			GLBuffer(GLuint id);
			GLBuffer(const GLBuffer&) = delete;
			GLBuffer(GLBuffer&&) noexcept;
			~GLBuffer();
			GLBuffer& operator=(GLBuffer&&) noexcept;

			operator GLuint () const { return id; }
			GLuint release() { GLuint old_id = id; id = 0; return old_id; }
		};

		std::vector<std::shared_ptr<GLBuffer>> gen_bulk_buffers(size_t n);

		template<size_t N>
		std::array<std::shared_ptr<GLBuffer>, N> gen_bulk_buffers()
		{
			std::array<GLuint, N> buffers;
			glGenBuffers(N, buffers.data());
			std::array<std::shared_ptr<GLBuffer>, N> wrapped_buffers;
			for (size_t i = 0; i < N; ++i)
				wrapped_buffers[i] = std::make_shared<GLBuffer>(buffers[i]);
			return wrapped_buffers;
		}
	}
}
