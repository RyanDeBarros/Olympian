#include "VertexAttributes.h"

namespace oly::graphics
{
    void VertexAttribute<VertexAttributeType::Float>::setup() const
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

    void VertexAttribute<VertexAttributeType::Int>::setup() const
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

    void VertexAttribute<VertexAttributeType::Double>::setup() const
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
}
