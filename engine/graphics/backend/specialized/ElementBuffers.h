#pragma once

#include "graphics/backend/basic/VertexArrays.h"

#include "graphics/backend/specialized/LazyBuffers.h"
#include "graphics/backend/specialized/PersistentBuffers.h"

namespace oly::graphics
{
    template<std::unsigned_integral IndexType, size_t Size>
    struct IndexLayout
    {
        using ElementAlias = std::conditional_t<Size != 1, IndexLayout<IndexType, Size>, IndexType>;

        static constexpr size_t SizeAlias = Size;

        IndexType data[Size];
    };

    template<std::unsigned_integral IndexType, Mutability M, size_t LayoutSize = 1>
    class CPUSideEBO
        : public LazyBuffer<typename IndexLayout<IndexType, LayoutSize>::ElementAlias, IndexType, M>
    {
        using ElementAlias = typename IndexLayout<IndexType, LayoutSize>::ElementAlias;

        struct
        {
            IndexType first = 0;
            IndexType count = 0;
            IndexType offset = 0;
        } draw_spec_;

    public:
        CPUSideEBO(IndexType size = 0)
            : LazyBuffer<ElementAlias, IndexType, M>(size, {})
        {
            set_draw_spec(0, size);
        }

        void get_draw_spec(IndexType& first, IndexType& count) const
        {
            first = draw_spec_.first;
            count = draw_spec_.count;
        }

        void set_draw_spec(IndexType first, IndexType count)
        {
            if (first < this->cpudata_.size())
                draw_spec_.first = first;

            draw_spec_.count = LayoutSize * std::min(count, IndexType(this->cpudata_.size() - draw_spec_.first));
            draw_spec_.offset = static_cast<IndexType>(draw_spec_.first * sizeof(ElementAlias));
        }

        void bind() const
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buf_);
        }

        void draw(GLenum mode, GLenum type) const
        {
#pragma warning(suppress : 4312)
            glDrawElements(
                mode,
                static_cast<GLsizei>(draw_spec_.count),
                type,
                reinterpret_cast<void*>(draw_spec_.offset));
        }
    };

    template<size_t PrimitiveIndices>
    class PersistentEBO
    {
        mutable LazyPersistentGPUBuffer<std::array<GLuint, PrimitiveIndices>> ebo_;
        mutable GLuint draw_count_ = 0;
        GLuint vao_ = 0;

    public:
        GLuint offset = 0;

        PersistentEBO(const VertexArray& vao, GLuint primitives = 0)
            : ebo_(primitives), vao_(vao)
        {
            bind_to_vao();
        }

        void set_vao(const VertexArray& vao)
        {
            vao_ = vao;
            bind_to_vao();
        }

        void grow() const
        {
            ebo_.grow();
            bind_to_vao();
        }

    private:
        void bind_to_vao() const
        {
            glBindVertexArray(vao_);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_.buf.get_buffer());
            glBindVertexArray(0);
        }

    public:
        std::array<GLuint, PrimitiveIndices>& draw_primitive() const
        {
            GLuint primitive = offset + draw_count_++;
            while (primitive >= ebo_.buf.get_size())
                grow();

            ebo_.flag(primitive);
            return ebo_.buf[primitive];
        }

        void render_elements(GLenum mode) const
        {
            ebo_.pre_draw();
            glDrawElements(
                mode,
                draw_count_ * PrimitiveIndices,
                GL_UNSIGNED_INT,
                reinterpret_cast<void*>(offset * PrimitiveIndices * sizeof(GLuint)));

            draw_count_ = 0;
            ebo_.post_draw();
        }

        bool empty() const
        {
            return draw_count_ == 0;
        }
    };

    template<std::integral Type>
    constexpr std::array<Type, 6> quad_indices(Type quad_index)
    {
        return std::array<Type, 6>{
            Type(0 + 4 * quad_index),
                Type(1 + 4 * quad_index),
                Type(2 + 4 * quad_index),
                Type(2 + 4 * quad_index),
                Type(3 + 4 * quad_index),
                Type(0 + 4 * quad_index)
        };
    }

    template<std::integral Type>
    constexpr void quad_indices(Type* indices, Type quad_index)
    {
        indices[0] = Type(0 + 4 * quad_index);
        indices[1] = Type(1 + 4 * quad_index);
        indices[2] = Type(2 + 4 * quad_index);
        indices[3] = Type(2 + 4 * quad_index);
        indices[4] = Type(3 + 4 * quad_index);
        indices[5] = Type(0 + 4 * quad_index);
    }
}
