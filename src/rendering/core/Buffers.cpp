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

oly::rendering::GLBufferBlock<0>::GLBufferBlock(GLsizei count)
	: ids(new GLuint[count]), count(count)
{
	glCreateBuffers(count, ids);
}

oly::rendering::GLBufferBlock<0>::GLBufferBlock(GLBufferBlock<0>&& other) noexcept
	: ids(other.ids), count(other.count)
{
	other.ids = nullptr;
	other.count = 0;
}

oly::rendering::GLBufferBlock<0>::~GLBufferBlock()
{
	glDeleteBuffers(count, ids);
	delete[] ids;
}

oly::rendering::GLBufferBlock<0>& oly::rendering::GLBufferBlock<0>::operator=(GLBufferBlock<0>&& other) noexcept
{
	if (this != &other)
	{
		glDeleteBuffers(count, ids);
		delete[] ids;
		ids = other.ids;
		count = other.count;
		other.ids = nullptr;
		other.count = 0;
	}
	return *this;
}

GLuint oly::rendering::GLBufferBlock<0>::operator[](GLsizei i) const
{
	OLY_ASSERT(i >= 0 && i < count);
	return ids[i];
}
