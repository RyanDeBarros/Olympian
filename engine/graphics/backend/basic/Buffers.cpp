#include "Buffers.h"

namespace oly::graphics
{
	void internal::swap_gl_buffer(GLBuffer& buffer, GLuint& id)
	{
		std::swap(buffer.id, id);
	}

	GLBuffer::GLBuffer()
	{
		glCreateBuffers(1, &id);
	}

	GLBuffer::GLBuffer(GLBuffer&& other) noexcept
		: id(other.id)
	{
		other.id = 0;
	}

	GLBuffer::~GLBuffer()
	{
		glDeleteBuffers(1, &id);
	}

	GLBuffer& GLBuffer::operator=(GLBuffer&& other) noexcept
	{
		if (this != &other)
		{
			glDeleteBuffers(1, &id);
			id = other.id;
			other.id = 0;
		}
		return *this;
	}


	GLBufferBlock<0>::GLBufferBlock(GLsizei count)
		: ids(new GLuint[count]), count(count)
	{
		glCreateBuffers(count, ids);
	}

	GLBufferBlock<0>::GLBufferBlock(GLBufferBlock<0>&& other) noexcept
		: ids(other.ids), count(other.count)
	{
		other.ids = nullptr;
		other.count = 0;
	}

	GLBufferBlock<0>::~GLBufferBlock()
	{
		glDeleteBuffers(count, ids);
		delete[] ids;
	}

	GLBufferBlock<0>& GLBufferBlock<0>::operator=(GLBufferBlock<0>&& other) noexcept
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

	GLuint GLBufferBlock<0>::operator[](GLsizei i) const
	{
		OLY_ASSERT(i >= 0 && i < count);
		return ids[i];
	}
}
