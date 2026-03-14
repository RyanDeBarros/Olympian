#pragma once

#include "external/GL.h"
#include "core/types/Variant.h"

namespace oly::graphics
{
	enum class VertexAttributeType
	{
		Float,
		Int,
		Double
	};

	template<VertexAttributeType T>
	struct VertexAttribute
	{
		static_assert(deferred_false<T>);
	};

	template<>
	struct VertexAttribute<VertexAttributeType::Float>
	{
		GLuint index = 0;
		GLint size = 0;
		GLenum type = GL_FLOAT;
		GLint cols = 1;
		GLboolean normalized = GL_FALSE;
		GLsizei stride = 0;
		GLsizei offset = 0;
		GLsizei col_stride = 0;
		GLuint divisor = 0;
		GLbitfield storage_flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;

		void setup() const
		{
			for (GLint i = 0; i < cols; ++i)
			{
#pragma warning(suppress : 4312)
				glVertexAttribPointer(index + i, size, type, normalized, stride, (void*)(offset + i * col_stride));
				glEnableVertexAttribArray(index + i);
				if (divisor > 0)
					glVertexAttribDivisor(index + i, divisor);
			}
		}
	};

	template<>
	struct VertexAttribute<VertexAttributeType::Int>
	{
		GLuint index = 0;
		GLint size = 0;
		GLenum type = GL_UNSIGNED_INT;
		GLint cols = 1;
		GLsizei stride = 0;
		GLsizei offset = 0;
		GLsizei col_stride = 0;
		GLuint divisor = 0;
		GLbitfield storage_flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;

		void setup() const
		{
			for (GLint i = 0; i < cols; ++i)
			{
#pragma warning(suppress : 4312)
				glVertexAttribIPointer(index + i, size, type, stride, (void*)(offset + i * col_stride));
				glEnableVertexAttribArray(index + i);
				if (divisor > 0)
					glVertexAttribDivisor(index + i, divisor);
			}
		}
	};

	template<>
	struct VertexAttribute<VertexAttributeType::Double>
	{
		GLuint index = 0;
		GLint size = 0;
		GLint cols = 1;
		GLsizei stride = 0;
		GLsizei offset = 0;
		GLsizei col_stride = 0;
		GLuint divisor = 0;
		GLbitfield storage_flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;

		void setup() const
		{
			for (GLint i = 0; i < cols; ++i)
			{
#pragma warning(suppress : 4312)
				glVertexAttribLPointer(index + i, size, GL_DOUBLE, stride, (void*)(offset + i * col_stride));
				glEnableVertexAttribArray(index + i);
				if (divisor > 0)
					glVertexAttribDivisor(index + i, divisor);
			}
		}
	};

	typedef Variant<VertexAttribute<VertexAttributeType::Float>, VertexAttribute<VertexAttributeType::Int>, VertexAttribute<VertexAttributeType::Double>> VertexAttributeVariant;
}
