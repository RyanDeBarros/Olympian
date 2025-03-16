#pragma once

#include <GL/glew.h>

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

		struct VAODescriptor
		{
			VertexArray vao;
			std::vector<GLBufferRes> vbos = { nullptr };
			GLBufferRes ebo = nullptr;

			void gen_vbos(GLsizei n) { vbos = oly::rendering::gen_bulk_buffers(n); }
			void gen_ebo();
			GLuint get_vbo(GLsizei i = 0) const { return *vbos[i]; }
			GLuint get_ebo() const { return *ebo; }
		};
	}
}
