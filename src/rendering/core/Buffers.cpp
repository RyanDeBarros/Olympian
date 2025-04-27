#include "Buffers.h"

void oly::rendering::_::swap_gl_buffer(GLBuffer& buffer, GLuint& id)
{
	std::swap(buffer.id, id);
}

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

void oly::rendering::GLBuffer::mutable_resize(GLsizeiptr new_size, GLenum usage)
{
	GLint old_size;
	glGetNamedBufferParameteriv(id, GL_BUFFER_SIZE, &old_size);
	mutable_resize(new_size, usage, old_size);
}

void oly::rendering::GLBuffer::mutable_resize(GLsizeiptr new_size, GLenum usage, GLsizeiptr old_size)
{
	if (new_size != old_size)
	{
		GLuint newid;
		glCreateBuffers(1, &newid);
		glNamedBufferData(newid, new_size, nullptr, usage);
		glCopyNamedBufferSubData(id, newid, 0, 0, std::min(new_size, old_size));
		glDeleteBuffers(1, &id);
		id = newid;
	}
}

void oly::rendering::GLBuffer::mutable_grow(GLsizeiptr new_size, GLenum usage)
{
	GLint old_size;
	glGetNamedBufferParameteriv(id, GL_BUFFER_SIZE, &old_size);
	mutable_grow(new_size, usage, old_size);
}

void oly::rendering::GLBuffer::mutable_grow(GLsizeiptr new_size, GLenum usage, GLsizeiptr old_size)
{
	if (new_size > old_size)
	{
		GLuint newid;
		glCreateBuffers(1, &newid);
		glNamedBufferData(newid, new_size, nullptr, usage);
		glCopyNamedBufferSubData(id, newid, 0, 0, std::min(new_size, old_size));
		glDeleteBuffers(1, &id);
		id = newid;
	}
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
