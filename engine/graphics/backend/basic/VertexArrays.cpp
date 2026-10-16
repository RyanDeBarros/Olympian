#include "VertexArrays.h"

namespace oly::graphics
{
	VertexArray::VertexArray()
	{
		glGenVertexArrays(1, &id_);
	}

	VertexArray::VertexArray(VertexArray&& o) noexcept
		: id_(o.id_)
	{
		o.id_ = 0;
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &id_);
	}

	VertexArray& VertexArray::operator=(VertexArray&& o) noexcept
	{
		if (this != &o)
		{
			glDeleteVertexArrays(1, &id_);
			id_ = o.id_;
			o.id_ = 0;
		}
		return *this;
	}

    VertexArray::operator GLuint () const
    {
        return id_;
    }
}
