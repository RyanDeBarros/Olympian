#pragma once

#include "graphics/backend/basic/VertexArrays.h"
#include "graphics/backend/specialized/LazyBuffers.h"
#include "graphics/backend/specialized/PersistentBuffers.h"

namespace oly::graphics
{
	template<std::unsigned_integral IndexType, size_t Size>
	struct IndexLayout
	{
		IndexType data[Size];

		using ElementAlias = std::conditional_t<Size != 1, IndexLayout<IndexType, Size>, IndexType>;
		static constexpr size_t SizeAlias = Size;
	};

	template<std::unsigned_integral IndexType, Mutability M, size_t LayoutSize = 1>
	class CPUSideEBO : public LazyBuffer<typename IndexLayout<IndexType, LayoutSize>::ElementAlias, IndexType, M>
	{
		using ElementAlias = typename IndexLayout<IndexType, LayoutSize>::ElementAlias;

		struct
		{
			IndexType first = 0;
			IndexType count = 0;
			IndexType offset = 0;
		} draw_spec;

	public:
		CPUSideEBO(IndexType size) : LazyBuffer<ElementAlias, IndexType, M>(size, {}) { set_draw_spec(0, size); }

		void get_draw_spec(IndexType& first, IndexType& count) const { first = draw_spec.first; count = draw_spec.count; }
		void set_draw_spec(IndexType first, IndexType count)
		{
			if (first < this->cpudata.size())
				draw_spec.first = first;
			draw_spec.count = LayoutSize * std::min(count, IndexType(this->cpudata.size() - draw_spec.first));
			draw_spec.offset = (IndexType)(draw_spec.first * sizeof(ElementAlias));
		}

		void bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buf); }
		void draw(GLenum mode, GLenum type) const
		{
#pragma warning(suppress : 4312)
			glDrawElements(mode, (GLsizei)draw_spec.count, type, (void*)(draw_spec.offset));
		}
	};

	template<size_t PrimitiveIndices = 6>
	class PersistentEBO
	{
		mutable LazyPersistentGPUBuffer<std::array<GLuint, PrimitiveIndices>> ebo;
		mutable GLuint draw_count = 0;
		GLuint vao = 0;

	public:
		GLuint offset = 0;

		PersistentEBO(const VertexArray& vao, GLuint primitives)
			: ebo(primitives), vao(vao)
		{
			bind_to_vao();
		}

		void set_vao(const VertexArray& vao)
		{
			this->vao = vao;
			bind_to_vao();
		}

		void grow() const
		{
			ebo.grow();
			bind_to_vao();
		}

	private:
		void bind_to_vao() const
		{
			glBindVertexArray(vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.buf.get_buffer());
			glBindVertexArray(0);
		}

	public:
		std::array<GLuint, PrimitiveIndices>& draw_primitive() const
		{
			GLuint primitive = offset + draw_count++;
			while (primitive >= ebo.buf.get_size())
				grow();
			ebo.flag(primitive);
			return ebo.buf[primitive];
		}

		void render_elements(GLenum mode) const
		{
			ebo.pre_draw();
			glDrawElements(mode, draw_count * PrimitiveIndices, GL_UNSIGNED_INT, (void*)(offset * PrimitiveIndices * sizeof(GLuint)));
			draw_count = 0;
			ebo.post_draw();
		}
	};

	template<std::integral Type>
	constexpr std::array<Type, 6> quad_indices(Type quad_index)
	{
		return std::array<Type, 6>{ Type(0 + 4 * quad_index), Type(1 + 4 * quad_index), Type(2 + 4 * quad_index), Type(2 + 4 * quad_index), Type(3 + 4 * quad_index), Type(0 + 4 * quad_index) };
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
