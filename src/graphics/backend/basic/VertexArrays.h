#pragma once

#include "Buffers.h"

namespace oly
{
	namespace rendering
	{
		class VertexArray
		{
			GLuint id;

		public:
			VertexArray();
			VertexArray(const VertexArray&) = delete;
			VertexArray(VertexArray&&) noexcept;
			~VertexArray();
			VertexArray& operator=(VertexArray&&) noexcept;

			operator GLuint () const { return id; }
		};
	}
}
