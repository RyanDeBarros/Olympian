#pragma once

#include "Buffers.h"

namespace oly::graphics
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
