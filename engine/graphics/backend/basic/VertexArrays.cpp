#include "VertexArrays.h"

namespace oly::graphics
{
	VertexArray::VertexArray()
	{
		glGenVertexArrays(1, &id);
	}

	VertexArray::VertexArray(VertexArray&& other) noexcept
		: id(other.id)
	{
		other.id = 0;
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &id);
	}

	VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
	{
		if (this != &other)
		{
			glDeleteVertexArrays(1, &id);
			id = other.id;
			other.id = 0;
		}
		return *this;
	}
}
