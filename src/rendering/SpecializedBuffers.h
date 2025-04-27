#pragma once

#include <set>

#include "core/Core.h"
#include "util/FixedVector.h"
#include "util/General.h"
#include "util/RangeMerger.h"

namespace oly
{
	namespace rendering
	{
		enum class BufferSendType
		{
			SUBDATA,
			MAP
		};

		template<typename Iterator, std::unsigned_integral IndexType>
		inline void batch_send(Iterator dirty_begin, Iterator dirty_end, GLuint buf, const void* cpudata, GLsizeiptr struct_size, IndexType pos_end, BufferSendType send_type = BufferSendType::SUBDATA)
		{
			const std::byte* data = (const std::byte*)cpudata;
			switch (send_type)
			{
			case BufferSendType::SUBDATA:
			{
				GLintptr offset = 0;
				GLsizeiptr size = 0;
				for (auto iter = dirty_begin; iter != dirty_end; ++iter)
				{
					if (*iter >= pos_end)
						continue;
					if (*iter * struct_size == offset + size)
						size += struct_size;
					else
					{
						glNamedBufferSubData(buf, offset, size, data + offset);
						offset = *iter * struct_size;
						size = struct_size;
					}
				}
				glNamedBufferSubData(buf, offset, size, data + offset);
				break;
			}
			case BufferSendType::MAP:
			{
				std::byte* gpu_buf = (std::byte*)glMapNamedBuffer(buf, GL_WRITE_ONLY);
				GLintptr offset = 0;
				GLsizeiptr size = 0;
				for (auto iter = dirty_begin; iter != dirty_end; ++iter)
				{
					if (*iter >= pos_end)
						continue;
					if (*iter * struct_size == offset + size)
						size += struct_size;
					else
					{
						memcpy(gpu_buf + offset, data + offset, size);
						offset = *iter * struct_size;
						size = struct_size;
					}
				}
				memcpy(gpu_buf + offset, data + offset, size);
				glUnmapNamedBuffer(buf);
				break;
			}
			}
		}

		template<typename Iterator, typename StructType>
		inline void batch_send(Iterator dirty_begin, Iterator dirty_end, GLuint buf, const FixedVector<StructType>& cpudata, BufferSendType send_type = BufferSendType::SUBDATA)
		{
			batch_send(dirty_begin, dirty_end, buf, cpudata.data(), sizeof(StructType), cpudata.size(), send_type);
		}

		enum class Mutability
		{
			IMMUTABLE,
			MUTABLE
		};

		template<typename StructType, Mutability M>
		class CPUSideBuffer
		{
		public:
			using VectorAlias = std::conditional_t<M == Mutability::IMMUTABLE, FixedVector<StructType>, std::vector<StructType>>;
			using StructAlias = StructType;

		protected:
			VectorAlias cpudata;
			GLBuffer buf;

		public:
			CPUSideBuffer(size_t size, const StructType& default_value = StructType()) : cpudata(size, default_value) {}
			virtual ~CPUSideBuffer() = default;

			const VectorAlias& vector() const { return cpudata; }
			VectorAlias& vector() { return cpudata; }
			GLuint buffer() const { return buf; }

			void init(GLbitfield flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT) const requires (M == Mutability::IMMUTABLE)
					{ glNamedBufferStorage(this->buf, this->cpudata.size() * sizeof(StructType), this->cpudata.data(), flags); }
			void init(GLenum usage = GL_DYNAMIC_DRAW) const requires (M == Mutability::MUTABLE)
					{ glNamedBufferData(this->buf, this->cpudata.size() * sizeof(StructType), this->cpudata.data(), usage); }
		};

		template<typename StructType, std::unsigned_integral IndexType, Mutability M>
		class LazyBuffer : public CPUSideBuffer<StructType, M>
		{
			mutable std::set<IndexType> dirty;

		public:
			using CPUSideBuffer<StructType, M>::CPUSideBuffer;

			void lazy_send(IndexType pos) const { dirty.insert(pos); }
			void clear_dirty() const { dirty.clear(); }
			void flush(BufferSendType send_type = BufferSendType::SUBDATA) const
					{ batch_send(dirty.begin(), dirty.end(), this->buf, this->cpudata.data(), sizeof(StructType), (IndexType)this->cpudata.size(), send_type); dirty.clear(); }
		};

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

		template<Mutability M, std::unsigned_integral IndexType = GLuint>
		using QuadLayoutEBO = CPUSideEBO<IndexType, M, 6>;

		template<Mutability M, std::unsigned_integral IndexType>
		inline void pre_init(QuadLayoutEBO<M, IndexType>& ebo)
		{
			for (IndexType i = 0; i < ebo.vector().size(); ++i)
				rendering::quad_indices(ebo.vector()[i].data, i);
		}

		template<typename StructType, std::unsigned_integral IndexType, Mutability M>
		struct IndexedSSBO : public LazyBuffer<StructType, IndexType, M>
		{
			IndexedSSBO(IndexType size, const StructType& default_value = StructType(), GLbitfield flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT) requires (M == Mutability::IMMUTABLE)
				: LazyBuffer<StructType, IndexType, M>(size, default_value)
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->buf);
				LazyBuffer<StructType, IndexType, M>::init(flags);
			}
			IndexedSSBO(IndexType size, const StructType& default_value = StructType(), GLenum usage = GL_DYNAMIC_DRAW) requires (M == Mutability::MUTABLE)
				: LazyBuffer<StructType, IndexType, M>(size, default_value)
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->buf);
				LazyBuffer<StructType, IndexType, M>::init(usage);
			}

			void bind_base(GLuint index) const { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, this->buf); }
		};

		template<Mutability M>
		class LightweightBuffer
		{
			GLBuffer buf;
			mutable GLsizeiptr size = 0;

		public:
			LightweightBuffer(GLsizeiptr size_in_bytes, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT) requires (M == Mutability::IMMUTABLE)
				: size(size_in_bytes) { glNamedBufferStorage(buf, size, nullptr, flags); }
			LightweightBuffer(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) requires (M == Mutability::MUTABLE)
				: size(size_in_bytes) { glNamedBufferData(buf, size, nullptr, usage); }

			GLuint buffer() const { return buf; }
			GLsizeiptr get_size() const { return size; }
			template<typename StructType>
			void send(GLintptr pos, const StructType& obj) const { glNamedBufferSubData(buf, pos * sizeof(StructType), sizeof(StructType), &obj); }
			template<typename StructType, typename MemberType>
			void send(GLintptr pos, MemberType StructType::* member, const MemberType& obj) const { glNamedBufferSubData(buf, pos * sizeof(StructType) + member_offset(member), sizeof(MemberType), &obj); }
			void resize_empty(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) const requires (M == Mutability::MUTABLE) { size = size_in_bytes; glNamedBufferData(buf, size, nullptr, usage); }
			void resize(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) requires (M == Mutability::MUTABLE) { buf.mutable_resize(size_in_bytes, usage, size); size = size_in_bytes; }
			void grow(GLsizeiptr size_in_bytes, GLenum usage = GL_DYNAMIC_DRAW) requires (M == Mutability::MUTABLE) { if (size_in_bytes > size) { buf.mutable_grow(size_in_bytes, usage, size); size = size_in_bytes; } }
		};

		template<Mutability M>
		struct LightweightSSBO : public LightweightBuffer<M>
		{
			using LightweightBuffer<M>::LightweightBuffer;
		
			void bind_base(GLuint index) const { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, this->buffer()); }
		};

		template<Mutability M>
		struct LightweightUBO : public LightweightBuffer<M>
		{
			using LightweightBuffer<M>::LightweightBuffer;

			void bind_base(GLuint index) const { glBindBufferBase(GL_UNIFORM_BUFFER, index, this->buffer()); }
		};

		struct PersistentWaitOptions
		{
			GLuint64 timeout_ns = 10'000'000; // 10ms
			GLuint max_timeout_tries = 100; // 1s total
		};

		template<typename Struct, PersistentWaitOptions options = PersistentWaitOptions{}>
		class PersistentGPUBuffer
		{
			GLBuffer buf;
			bool accessible = true;
			GLuint size = 0;
			void* data = nullptr;

		public:
			using StructAlias = Struct;

			PersistentGPUBuffer(GLuint size) : size(size) { init(); }
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
				static const float REALLOC_MULTIPLIER = 1.8f;
				grow((GLuint)(REALLOC_MULTIPLIER * size));
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
				init();

				glCopyNamedBufferSubData(old_buf, buf, 0, 0, old_size * sizeof(Struct));
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
					return;
				for (GLuint i = 0; i < options.max_timeout_tries; ++i)
					if (sync.wait(options.timeout_ns))
						return;
				throw Error(ErrorCode::OUT_OF_TIME);
			}

		private:
			void init()
			{
				glNamedBufferStorage(buf, (GLsizeiptr)(size * sizeof(Struct)), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
				data = glMapNamedBufferRange(buf, 0, (GLsizeiptr)(size * sizeof(Struct)), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
			}
		};

		template<typename Struct, PersistentWaitOptions options = PersistentWaitOptions{} >
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
				while (i >= buf.get_size())
					grow();
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
				while (offset + length > buf.get_size())
					grow();
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

		template<size_t PrimitiveIndices = 6>
		class PersistentEBO
		{
			mutable LazyPersistentGPUBuffer<std::array<GLuint, PrimitiveIndices>> ebo;
			mutable GLuint draw_count = 0;
			GLuint vao = 0;

		public:
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
				GLuint primitive = draw_count++;
				if (primitive >= ebo.buf.get_size())
					grow();
				ebo.flag(primitive);
				return ebo.buf[primitive];
			}
			void render_elements(GLenum mode) const { ebo.pre_draw(); glDrawElements(mode, draw_count * PrimitiveIndices, GL_UNSIGNED_INT, 0); draw_count = 0; ebo.post_draw(); }
		};

		enum class VertexAttributeType
		{
			FLOAT,
			INT,
			DOUBLE
		};

		template<VertexAttributeType Type = VertexAttributeType::FLOAT>
		struct VertexAttribute
		{
			GLuint index;
			GLint size;
			GLenum type = GL_FLOAT;
			GLint cols = 1;
			GLboolean normalized = GL_FALSE;
			GLsizei stride = 0;
			GLsizei offset = 0;
			GLsizei col_stride = 0;
			GLuint divisor = 0;
			GLbitfield storage_flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;

			void setup() const
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
		};

		template<>
		struct VertexAttribute<VertexAttributeType::INT>
		{
			GLuint index;
			GLint size;
			GLenum type = GL_UNSIGNED_INT;
			GLint cols = 1;
			GLsizei stride = 0;
			GLsizei offset = 0;
			GLsizei col_stride = 0;
			GLuint divisor = 0;
			GLbitfield storage_flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;

			void setup() const
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
		};

		template<>
		struct VertexAttribute<VertexAttributeType::DOUBLE>
		{
			GLuint index;
			GLint size;
			GLint cols = 1;
			GLsizei stride = 0;
			GLsizei offset = 0;
			GLsizei col_stride = 0;
			GLuint divisor = 0;
			GLbitfield storage_flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;

			void setup() const
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
		};
	}
}
