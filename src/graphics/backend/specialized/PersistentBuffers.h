#pragma once

#include "external/GL.h"
#include "core/base/Errors.h"
#include "core/containers/Ranges.h"
#include "graphics/backend/basic/Buffers.h"
#include "graphics/backend/basic/FenceSync.h"

namespace oly::graphics
{
	namespace internal
	{
		struct PersistentOptions
		{
			GLuint64 timeout_ns = 10'000'000; // 10ms
			GLuint max_timeout_tries = 100; // 1s total
			float grow_multiplier = 1.8f;
		};
	}

	template<typename Struct, internal::PersistentOptions options = internal::PersistentOptions{} >
	class PersistentGPUBuffer
	{
		GLBuffer buf;
		mutable bool accessible = true;
		GLuint size = 0;
		void* data = nullptr;

	public:
		using StructAlias = Struct;

		PersistentGPUBuffer(GLuint size)
			: size(size)
		{
			glNamedBufferStorage(buf, (GLsizeiptr)(size * sizeof(Struct)), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
			data = glMapNamedBufferRange(buf, 0, (GLsizeiptr)(size * sizeof(Struct)), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
		}
		PersistentGPUBuffer(const PersistentGPUBuffer&) = delete;

		GLuint get_size() const { return size; }
		GLuint get_buffer() const { return buf; }
		bool is_accessible() const { return accessible; }

		const Struct& operator[](GLuint i) const
		{
			if (!accessible)
				throw Error(ErrorCode::INACCESSIBLE);
			if (i >= size)
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return reinterpret_cast<const Struct*>(data)[i];
		}

		Struct& operator[](GLuint i)
		{
			if (!accessible)
				throw Error(ErrorCode::INACCESSIBLE);
			if (i >= size)
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return reinterpret_cast<Struct*>(data)[i];
		}

		const Struct* arr(GLuint offset, GLuint length) const
		{
			if (!accessible)
				throw Error(ErrorCode::INACCESSIBLE);
			if (offset + length > size)
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return reinterpret_cast<const Struct*>(data) + offset;
		}

		Struct* arr(GLuint offset, GLuint length)
		{
			if (!accessible)
				throw Error(ErrorCode::INACCESSIBLE);
			if (offset + length > size)
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return reinterpret_cast<Struct*>(data) + offset;
		}

		void grow()
		{
			grow((GLuint)(options.grow_multiplier * size));
		}

		void grow(GLuint new_size)
		{
			if (!accessible)
				throw Error(ErrorCode::INACCESSIBLE);
			if (new_size <= size)
				return;

			GLuint old_size = size;
			size = new_size;

			GLBuffer old_buf;
			std::swap(buf, old_buf);
			glNamedBufferStorage(buf, (GLsizeiptr)(size * sizeof(Struct)), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
			glCopyNamedBufferSubData(old_buf, buf, 0, 0, old_size * sizeof(Struct));
			data = glMapNamedBufferRange(buf, 0, (GLsizeiptr)(size * sizeof(Struct)), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
			// no need to unmap old data, since it uses persistent bit
		}

		void pre_draw(GLuint offset, GLuint length) const
		{
			if (offset + length > size)
			{
				if (offset >= size)
					throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
				else
					length = size - 1 - offset;
			}
			glFlushMappedNamedBufferRange(buf, (GLintptr)(offset * sizeof(Struct)), (GLsizeiptr)(length * sizeof(Struct)));
		}

		void pre_draw() const
		{
			glFlushMappedNamedBufferRange(buf, (GLintptr)0, (GLsizeiptr)(size * sizeof(Struct)));
		}

		void post_draw() const
		{
			FenceSync sync;
			if (sync.signaled())
			{
				accessible = true;
				return;
			}
			for (GLuint i = 0; i < options.max_timeout_tries; ++i)
			{
				if (sync.wait(options.timeout_ns))
				{
					accessible = true;
					return;
				}
				LOG << LOG.begin_temp(LOG.error) << LOG.start << "Timeout in persistent buffer sync" << LOG.end_temp << LOG.nl;
			}
			accessible = false;
			throw Error(ErrorCode::OUT_OF_TIME);
		}
	};

	template<typename... Structs>
	class PersistentGPUBufferBlock
	{
	public:
		static constexpr size_t N = std::tuple_size_v<std::tuple<Structs...>>;
		template<size_t n>
		using StructAlias = std::tuple_element_t<n, std::tuple<Structs...>>;

	private:
		static constexpr internal::PersistentOptions options = {};
		GLBufferBlock<N> buf;
		mutable std::array<bool, N> accessible;
		std::array<GLuint, N> size;
		std::array<void*, N> data;

	public:
		PersistentGPUBufferBlock(GLuint size)
		{
			accessible.fill(true);
			this->size.fill(size);
			data.fill(nullptr);
			init(std::make_index_sequence<N>{});
		}
		PersistentGPUBufferBlock(const std::array<GLuint, N>& sizes)
			: size(sizes)
		{
			accessible.fill(true);
			data.fill(nullptr);
			init(std::make_index_sequence<N>{});
		}
		PersistentGPUBufferBlock(const PersistentGPUBufferBlock&) = delete;

		template<size_t n>
		GLuint get_size() const { return size[n]; }
		template<size_t n>
		GLuint get_buffer() const { return buf[n]; }
		template<size_t n>
		bool is_accessible() const { return accessible[n]; }

		template<size_t n>
		const StructAlias<n>& at(GLuint i) const
		{
			static_assert(n < N);
			if (!accessible[n])
				throw Error(ErrorCode::INACCESSIBLE);
			if (i >= size[n])
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return reinterpret_cast<const StructAlias<n>*>(data[n])[i];
		}

		template<size_t n>
		StructAlias<n>& at(GLuint i)
		{
			static_assert(n < N);
			if (!accessible[n])
				throw Error(ErrorCode::INACCESSIBLE);
			if (i >= size[n])
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return reinterpret_cast<StructAlias<n>*>(data[n])[i];
		}

		template<size_t n>
		const StructAlias<n>* arr(GLuint offset, GLuint length) const
		{
			static_assert(n < N);
			if (!accessible[n])
				throw Error(ErrorCode::INACCESSIBLE);
			if (offset + length > size[n])
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return reinterpret_cast<const StructAlias<n>*>(data[n]) + offset;
		}

		template<size_t n>
		StructAlias<n>* arr(GLuint offset, GLuint length)
		{
			static_assert(n < N);
			if (!accessible[n])
				throw Error(ErrorCode::INACCESSIBLE);
			if (offset + length > size[n])
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return reinterpret_cast<StructAlias<n>*>(data[n]) + offset;
		}

		void grow_all()
		{
			grow_impl(std::make_index_sequence<N>{});
		}

	private:
		template<size_t... Indices>
		void grow_impl(std::index_sequence<Indices...>) const
		{
			(grow<Indices>(), ...);
		}

	public:
		template<size_t n>
		void grow()
		{
			static_assert(n < N);
			grow<n>((GLuint)(options.grow_multiplier * size[n]));
		}

		template<size_t n>
		void grow(GLuint new_size)
		{
			static_assert(n < N);
			if (!accessible[n])
				throw Error(ErrorCode::INACCESSIBLE);
			if (new_size <= size[n])
				return;

			GLuint old_size = size[n];
			size[n] = new_size;

			GLBuffer old_buf;
			buf.swap<n>(old_buf);
			glNamedBufferStorage(buf[n], (GLsizeiptr)(size[n] * sizeof(StructAlias<n>)), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
			glCopyNamedBufferSubData(old_buf, buf[n], 0, 0, old_size * sizeof(StructAlias<n>));
			data[n] = glMapNamedBufferRange(buf[n], 0, (GLsizeiptr)(size[n] * sizeof(StructAlias<n>)), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
			// no need to unmap old data, since it uses persistent bit
		}

		template<size_t n>
		void pre_draw(GLuint offset, GLuint length) const
		{
			static_assert(n < N);
			if (offset + length > size[n])
			{
				if (offset >= size[n])
					throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
				else
					length = size[n] - 1 - offset;
			}
			glFlushMappedNamedBufferRange(buf[n], (GLintptr)(offset * sizeof(StructAlias<n>)), (GLsizeiptr)(length * sizeof(StructAlias<n>)));
		}

		template<size_t n>
		void pre_draw() const
		{
			static_assert(n < N);
			glFlushMappedNamedBufferRange(buf[n], (GLintptr)0, (GLsizeiptr)(size[n] * sizeof(StructAlias<n>)));
		}

		void pre_draw_all() const
		{
			pre_draw_impl(std::make_index_sequence<N>{});
		}

	private:
		template<size_t... Indices>
		void pre_draw_impl(std::index_sequence<Indices...>) const
		{
			(pre_draw<Indices>(), ...);
		}

	public:
		template<size_t n>
		void post_draw() const
		{
			static_assert(n < N);
			FenceSync sync;
			if (sync.signaled())
			{
				accessible[n] = true;
				return;
			}
			for (GLuint i = 0; i < options.max_timeout_tries; ++i)
			{
				if (sync.wait(options.timeout_ns))
				{
					accessible[n] = true;
					return;
				}
				LOG << LOG.begin_temp(LOG.error) << LOG.start << "Timeout in persistent buffer sync" << LOG.end_temp << LOG.nl;
			}
			accessible[n] = false;
			throw Error(ErrorCode::OUT_OF_TIME);
		}

		void post_draw_all() const
		{
			post_draw_impl(std::make_index_sequence<N>{});
		}

	private:
		template<size_t... Indices>
		void post_draw_impl(std::index_sequence<Indices...>) const
		{
			(post_draw<Indices>(), ...);
		}

		template<size_t... Indices>
		void init(std::index_sequence<Indices...>)
		{
			((
				glNamedBufferStorage(buf[Indices], (GLsizeiptr)(size[Indices] * sizeof(StructAlias<Indices>)), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT),
				data[Indices] = glMapNamedBufferRange(buf[Indices], 0, (GLsizeiptr)(size[Indices] * sizeof(StructAlias<Indices>)), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT)
				), ...);
		}
	};

	template<typename Struct, internal::PersistentOptions options = internal::PersistentOptions{} >
	struct LazyPersistentGPUBuffer
	{
		PersistentGPUBuffer<Struct, options> buf;

	private:
		mutable RangeMerger<GLuint> dirty;

	public:
		LazyPersistentGPUBuffer(GLuint size) : buf(size) {}

		void flag(GLuint pos) const { dirty.insert({ pos, 1 }); }
		void pre_draw() const
		{
			for (Range<GLuint> range : dirty)
			{
				try
				{
					buf.pre_draw(range.initial, range.length);
				}
				catch (Error e)
				{
					if (e.code == ErrorCode::INDEX_OUT_OF_RANGE)
						; // silently ignore
					else
						throw e;
				}
			}
			dirty.clear();
		}
		void post_draw() const { buf.post_draw(); }
		void grow() { buf.grow(); buf.pre_draw(); dirty.clear(); }

		const Struct& get(GLuint i) const
		{
			return buf[i];
		}

		Struct& set(GLuint i)
		{
			while (i >= buf.get_size())
				grow();
			flag(i);
			return buf[i];
		}

		const Struct* get(GLuint offset, GLuint length) const
		{
			return buf.arr(offset, length);
		}

		Struct* set(GLuint offset, GLuint length)
		{
			while (offset + length > buf.get_size())
				grow();
			for (GLuint i = 0; i < length; ++i)
				flag(offset + i);
			return buf.arr(offset, length);
		}
	};

	template<typename... Structs>
	struct LazyPersistentGPUBufferBlock
	{
		PersistentGPUBufferBlock<Structs...> buf;
		static constexpr size_t N = PersistentGPUBufferBlock<Structs...>::N;
		template<size_t n>
		using StructAlias = typename PersistentGPUBufferBlock<Structs...>::template StructAlias<n>;

	private:
		mutable std::array<RangeMerger<GLuint>, N> dirty;

	public:
		LazyPersistentGPUBufferBlock(GLuint size) : buf(size) {}
		LazyPersistentGPUBufferBlock(const std::array<GLuint, N>& sizes) : buf(sizes) {}

		template<size_t n>
		void flag(GLuint pos) const
		{
			static_assert(n < N);
			dirty[n].insert({ pos, 1 });
		}
		template<size_t n>
		void pre_draw() const
		{
			static_assert(n < N);
			for (Range<GLuint> range : dirty[n])
			{
				try
				{
					buf.pre_draw<n>(range.initial, range.length);
				}
				catch (Error e)
				{
					if (e.code == ErrorCode::INDEX_OUT_OF_RANGE)
						; // silently ignore
					else
						throw e;
				}
			}
			dirty[n].clear();
		}
		void pre_draw_all() const { pre_draw_impl(std::make_index_sequence<N>{}); }

	private:
		template<size_t... Indices>
		void pre_draw_impl(std::index_sequence<Indices...>) const { (pre_draw<Indices>(), ...); }

	public:
		template<size_t n>
		void post_draw() const { static_assert(n < N); buf.post_draw<n>(); }
		void post_draw_all() const { post_draw_impl(std::make_index_sequence<N>{}); }

	private:
		template<size_t... Indices>
		void post_draw_impl(std::index_sequence<Indices...>) const { (post_draw<Indices>(), ...); }

	public:
		template<size_t n>
		void grow() { static_assert(n < N); buf.grow<n>(); buf.pre_draw<n>(); dirty[n].clear(); }
		void grow_all() const { grow_impl(std::make_index_sequence<N>{}); }

	private:
		template<size_t... Indices>
		void grow_impl(std::index_sequence<Indices...>) const { (grow<Indices>(), ...); }

	public:
		template<size_t n>
		const StructAlias<n>& get(GLuint i) const
		{
			static_assert(n < N);
			return buf.at<n>(i);
		}

		template<size_t n>
		StructAlias<n>& set(GLuint i, bool* grown = nullptr)
		{
			static_assert(n < N);
			while (i >= buf.get_size<n>())
			{
				grow<n>();
				if (grown)
					*grown = true;
			}
			flag<n>(i);
			return buf.at<n>(i);
		}

		template<size_t n>
		const StructAlias<n>* get(GLuint offset, GLuint length) const
		{
			static_assert(n < N);
			return buf.arr<n>(offset, length);
		}

		template<size_t n>
		StructAlias<n>* set(GLuint offset, GLuint length, bool* grown = nullptr)
		{
			static_assert(n < N);
			while (offset + length > buf.get_size<n>())
			{
				grow<n>();
				if (grown)
					*grown = true;
			}
			for (GLuint i = 0; i < length; ++i)
				flag<n>(offset + i);
			return buf.arr<n>(offset, length);
		}
	};
}
