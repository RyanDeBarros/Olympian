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
			GLBuffer(const GLBuffer&) = delete;
			GLBuffer(GLBuffer&&) noexcept;
			~GLBuffer();
			GLBuffer& operator=(GLBuffer&&) noexcept;

			operator GLuint () const { return id; }
		};

		typedef std::shared_ptr<GLBuffer> GLBufferRes;

		// TODO GLbufferBlock
	}
}
