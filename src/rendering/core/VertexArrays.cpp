#include "VertexArrays.h"

oly::rendering::VertexArray::VertexArray()
{
	glGenVertexArrays(1, &id);
}

oly::rendering::VertexArray::VertexArray(VertexArray&& other) noexcept
	: id(other.id)
{
	other.id = 0;
}

oly::rendering::VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &id);
}

oly::rendering::VertexArray& oly::rendering::VertexArray::operator=(VertexArray&& other) noexcept
{
	if (this != &other)
	{
		glDeleteVertexArrays(1, &id);
		id = other.id;
		other.id = 0;
	}
	return *this;
}

void oly::rendering::VAODescriptor::gen_ebo()
{
	ebo = std::make_shared<GLBuffer>(); glBindVertexArray(vao); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
}
