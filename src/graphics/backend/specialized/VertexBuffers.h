#pragma once

#include "graphics/backend/specialized/PersistentBuffers.h"
#include "graphics/backend/specialized/VertexAttributes.h"

namespace oly::graphics
{
	template<typename... Structs>
	class PersistentVertexBufferBlock
	{
		LazyPersistentGPUBufferBlock<Structs...> buf;
		GLuint vao;

	public:
		template<size_t n>
		using StructAlias = typename LazyPersistentGPUBufferBlock<Structs...>::template StructAlias<n>;
		static constexpr size_t N = LazyPersistentGPUBufferBlock<Structs...>::N;

		std::array<VertexAttributeVariant, N> attributes;

		PersistentVertexBufferBlock(GLuint vao, GLuint size) : vao(vao), buf(size) {}
		PersistentVertexBufferBlock(GLuint vao, const std::array<GLuint, N>& sizes) : vao(vao), buf(sizes) {}

		void setup()
		{
			glBindVertexArray(vao);
			setup_impl(std::make_index_sequence<N>{});
			glBindVertexArray(0);
		}

		void set_vao(const VertexArray& vao)
		{
			this->vao = vao;
			setup();
		}

	private:
		template<size_t... Indices>
		void setup_impl(std::index_sequence<Indices...>)
		{
			(setup<Indices>(), ...);
		}

		template<size_t n>
		void setup()
		{
			static_assert(n < N);
			glBindBuffer(GL_ARRAY_BUFFER, buffer<n>());
			std::visit([](auto&& attribute) { attribute.setup(); }, attributes[n]);
		}

		template<size_t n>
		void setup_single()
		{
			static_assert(n < N);
			glBindVertexArray(vao);
			setup<n>();
			glBindVertexArray(0);
		}

	public:
		template<size_t n>
		GLuint buffer() const { return buf.buf.get_buffer<n>(); }
		template<size_t n>
		GLuint size() const { return buf.buf.get_size<n>(); }
		template<size_t n>
		void pre_draw() const { buf.pre_draw<n>(); }
		void pre_draw_all() const { buf.pre_draw_all(); }
		template<size_t n>
		void post_draw() const { buf.post_draw<n>(); }
		void post_draw_all() const { buf.post_draw_all(); }
		template<size_t n>
		void grow() { buf.grow<n>(); setup_single<n>(); }
		void grow_all() const { buf.grow_all(); setup(); }
		template<size_t n>
		const StructAlias<n>& get(GLuint i) const { return buf.get<n>(i); }
		template<size_t n>
		StructAlias<n>& set(GLuint i)
		{
			bool grown = false;
			StructAlias<n>& el = buf.set<n>(i, &grown);
			if (grown)
				setup_single<n>();
			return el;
		}
		template<size_t n>
		const StructAlias<n>* get(GLuint offset, GLuint length) const { return buf.get<n>(offset, length); }
		template<size_t n>
		StructAlias<n>* set(GLuint offset, GLuint length)
		{
			bool grown = false;
			StructAlias<n>* arr = buf.set<n>(offset, length, &grown);
			if (grown)
				setup_single<n>();
			return arr;
		}
	};
}
