#include "Buffers.h"

oly::rendering::GLBuffer::GLBuffer()
{
	glGenBuffers(1, &id);
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

std::vector<std::shared_ptr<oly::rendering::GLBuffer>> oly::rendering::gen_bulk_buffers(GLsizei n)
{
	std::vector<GLuint> buffers;
	buffers.resize(n);
	glGenBuffers(n, buffers.data());
	std::vector<std::shared_ptr<GLBuffer>> wrapped_buffers;
	wrapped_buffers.reserve(n);
	for (GLuint id : buffers)
		wrapped_buffers.push_back(GLBuffer::from_id(id));
	return wrapped_buffers;
}
