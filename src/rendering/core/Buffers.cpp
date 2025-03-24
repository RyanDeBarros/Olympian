#include "Buffers.h"

oly::rendering::GLBuffer::GLBuffer()
{
	glCreateBuffers(1, &id);
}

oly::rendering::GLBuffer::GLBuffer(GLBuffer&& other) noexcept
	: id(other.id)
{
	other.id = 0;
}

oly::rendering::GLBuffer::~GLBuffer()
{
	glDeleteBuffers(1, &id);
}

oly::rendering::GLBuffer& oly::rendering::GLBuffer::operator=(GLBuffer&& other) noexcept
{
	if (this != &other)
	{
		glDeleteBuffers(1, &id);
		id = other.id;
		other.id = 0;
	}
	return *this;
}
