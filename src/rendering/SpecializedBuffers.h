#pragma once

#include <set>

#include "core/Core.h"
#include "util/FixedVector.h"
#include "util/General.h"

namespace oly
{
	namespace rendering
	{
		template<typename StructType, std::unsigned_integral IndexType>
		class FixedVectorBuffer
		{
		protected:
			using StructTypeAlias = StructType;
			using IndexTypeAlias = IndexType;

			FixedVector<StructType> cpudata;
			GLBuffer buf;

		public:
			FixedVectorBuffer(IndexType size, const StructType& default_value = StructType());
			virtual ~FixedVectorBuffer() = default;

			const FixedVector<StructType>& vector() const { return cpudata; }
			FixedVector<StructType>& vector() { return cpudata; }
			GLuint buffer() const { return buf; }
		};

		template<typename StructType, std::unsigned_integral IndexType>
		inline FixedVectorBuffer<StructType, IndexType>::FixedVectorBuffer(IndexType size, const StructType& default_value)
			: cpudata(size, default_value)
		{
		}

		enum class BufferSendType
		{
			SUBDATA,
			MAP
		};

		struct BufferSendConfig
		{
			const BufferSendType buffer_send_type = BufferSendType::SUBDATA;
			const bool allow_map_send = true;

			BufferSendConfig(BufferSendType buffer_send_type = BufferSendType::SUBDATA, bool allow_map_send = true)
				: allow_map_send(allow_map_send), buffer_send_type(allow_map_send ? buffer_send_type : BufferSendType::SUBDATA) {}
		};

		template<std::unsigned_integral IndexType>
		class LazySender
		{
			BufferSendConfig config;
			mutable std::set<IndexType> dirty;

		public:
			LazySender(BufferSendConfig config = {}) : config(config) {}

			void override_config(const BufferSendConfig& new_config) { config = new_config; }
			BufferSendType get_buffer_send_type() const { return config.buffer_send_type; }
			void set_buffer_send_type(BufferSendType type) { if (config.allow_map_send) config.buffer_send_type = type; }

			void init_storage(GLuint buf, const void* data, GLsizeiptr size) const;
			void init_mutable(GLuint buf, const void* data, GLsizeiptr size) const;
			void lazy_send(IndexType pos) const;
			void flush(GLuint buf, const void* cpudata, GLsizeiptr struct_size, IndexType pos_end) const;
		};

		template<std::unsigned_integral IndexType>
		inline void LazySender<IndexType>::init_storage(GLuint buf, const void* data, GLsizeiptr size) const
		{
			if (config.allow_map_send)
				glNamedBufferStorage(buf, size, data, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
			else
				glNamedBufferStorage(buf, size, data, GL_DYNAMIC_STORAGE_BIT);
		}

		template<std::unsigned_integral IndexType>
		inline void LazySender<IndexType>::lazy_send(IndexType pos) const
		{
			dirty.insert(pos);
		}

		template<std::unsigned_integral IndexType>
		inline void LazySender<IndexType>::flush(GLuint buf, const void* cpudata, GLsizeiptr struct_size, IndexType pos_end) const
		{
			const std::byte* data = (const std::byte*)cpudata;
			switch (config.buffer_send_type)
			{
			case BufferSendType::SUBDATA:
			{
				GLintptr offset = 0;
				GLsizeiptr size = 0;
				for (auto iter = dirty.begin(); iter != dirty.end(); ++iter)
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
				for (auto iter = dirty.begin(); iter != dirty.end(); ++iter)
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
			dirty.clear();
		}

		template<std::unsigned_integral... IndexTypes>
		class LazyMultiSender
		{
		public:
			static constexpr size_t N = sizeof...(IndexTypes);

		private:
			std::tuple<LazySender<IndexTypes>...> senders;

		public:
			template<size_t i>
			using IndexType = std::tuple_element_t<i, std::tuple<IndexTypes...>>;

			LazyMultiSender();

			template<size_t i>
			const auto& sender() const { return std::get<i>(senders); }
			template<size_t i>
			auto& sender() { return std::get<i>(senders); }
		};

		template<std::unsigned_integral... IndexTypes>
		inline LazyMultiSender<IndexTypes...>::LazyMultiSender()
			: senders(LazySender<IndexTypes>()...)
		{
		}

		template<typename StructType, std::unsigned_integral IndexType>
		class LazyBuffer : public FixedVectorBuffer<StructType, IndexType>
		{
		protected:
			using StructTypeAlias = StructType;

		private:
			LazySender<IndexType> lazy;

		public:
			LazyBuffer(IndexType size, const StructType& default_value = StructType(), BufferSendConfig config = {});

			const LazySender<IndexType>& get_sender() const { return lazy; }
			LazySender<IndexType>& get_sender() { return lazy; }

		protected:
			void init_storage() const { lazy.init_storage(this->buf, this->cpudata.data(), this->cpudata.size() * sizeof(StructType)); }

		public:
			void lazy_send(IndexType pos) const { lazy.lazy_send(pos); }
			void flush() const { lazy.flush(this->buf, this->cpudata.data(), sizeof(StructType), (IndexType)this->cpudata.size()); }
		};

		template<typename StructType, std::unsigned_integral IndexType>
		inline LazyBuffer<StructType, IndexType>::LazyBuffer(IndexType size, const StructType& default_value, BufferSendConfig config)
			: FixedVectorBuffer<StructType, IndexType>(size, default_value), lazy(config)
		{
		}

		template<std::unsigned_integral IndexType, size_t Size>
		struct FixedIndexLayout
		{
			IndexType data[Size];
		};

		template<std::unsigned_integral IndexType, size_t LayoutSize = 1>
		class FixedLayoutEBO : public LazyBuffer<FixedIndexLayout<IndexType, LayoutSize>, IndexType>
		{
			struct
			{
				IndexType first = 0;
				IndexType count = 0;
				IndexType offset = 0;
			} draw_spec;

		public:
			FixedLayoutEBO(IndexType size, BufferSendConfig config = {});

			void get_draw_spec(IndexType& first, IndexType& count) const { first = draw_spec.first; count = draw_spec.count; }
			void set_draw_spec(IndexType first, IndexType count);

			void init() const;
			void draw(GLenum mode, GLenum type) const;
		};

		template<std::unsigned_integral IndexType, size_t LayoutSize>
		inline FixedLayoutEBO<IndexType, LayoutSize>::FixedLayoutEBO(IndexType size, BufferSendConfig config)
			: LazyBuffer<FixedIndexLayout<IndexType, LayoutSize>, IndexType>(size, {}, config)
		{
			set_draw_spec(0, size);
		}

		template<std::unsigned_integral IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::set_draw_spec(IndexType first, IndexType count)
		{
			if (first < this->cpudata.size())
				draw_spec.first = first;
			draw_spec.count = LayoutSize * std::min(count, IndexType(this->cpudata.size() - draw_spec.first));
			draw_spec.offset = (IndexType)(draw_spec.first * sizeof(FixedIndexLayout<IndexType, LayoutSize>));
		}

		template<std::unsigned_integral IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::init() const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buf);
			this->init_storage();
		}

		template<std::unsigned_integral IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::draw(GLenum mode, GLenum type) const
		{
			glDrawElements(mode, (GLsizei)draw_spec.count, type, (void*)(draw_spec.offset));
		}

		template<std::unsigned_integral IndexType>
		class FixedLayoutEBO<IndexType, 1> : public LazyBuffer<IndexType, IndexType>
		{
			struct
			{
				IndexType first = 0;
				IndexType count = 0;
				IndexType offset = 0;
			} draw_spec;

		public:
			FixedLayoutEBO(IndexType size, BufferSendConfig config = {});

			void get_draw_spec(IndexType& first, IndexType& count) const { first = draw_spec.first; count = draw_spec.count; }
			void set_draw_spec(IndexType first, IndexType count);

			void init() const;
			void draw(GLenum mode, GLenum type) const;
		};

		template<std::unsigned_integral IndexType>
		inline FixedLayoutEBO<IndexType, 1>::FixedLayoutEBO(IndexType size, BufferSendConfig config)
			: LazyBuffer<IndexType, IndexType>(size, {}, config)
		{
			set_draw_spec(0, size);
		}

		template<std::unsigned_integral IndexType>
		inline void FixedLayoutEBO<IndexType, 1>::set_draw_spec(IndexType first, IndexType count)
		{
			if (first < this->cpudata.size())
				draw_spec.first = first;
			draw_spec.count = std::min(count, IndexType(this->cpudata.size() - draw_spec.first));
			draw_spec.offset = (IndexType)(draw_spec.first * sizeof(IndexType));
		}

		template<std::unsigned_integral IndexType>
		inline void FixedLayoutEBO<IndexType, 1>::init() const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buf);
			this->init_storage();
		}

		template<std::unsigned_integral IndexType>
		inline void FixedLayoutEBO<IndexType, 1>::draw(GLenum mode, GLenum type) const
		{
			glDrawElements(mode, (GLsizei)draw_spec.count, type, (void*)(draw_spec.offset));
		}

		template<std::unsigned_integral IndexType = GLushort>
		using QuadLayoutEBO = FixedLayoutEBO<IndexType, 6>;

		template<std::unsigned_integral IndexType>
		inline void pre_init(QuadLayoutEBO<IndexType>& ebo)
		{
			for (IndexType i = 0; i < ebo.vector().size(); ++i)
				rendering::quad_indices(ebo.vector()[i].data, i);
		}

		template<typename StructType, std::unsigned_integral IndexType>
		class IndexedSSBO : public LazyBuffer<StructType, IndexType>
		{
		public:
			IndexedSSBO(IndexType size, const StructType& default_value = StructType(), BufferSendConfig config = {});

			void bind_base(GLuint index) const;
		};

		template<typename StructType, std::unsigned_integral IndexType>
		inline IndexedSSBO<StructType, IndexType>::IndexedSSBO(IndexType size, const StructType& default_value, BufferSendConfig config)
			: LazyBuffer<StructType, IndexType>(size, default_value, config)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->buf);
			this->init_storage();
		}

		template<typename StructType, std::unsigned_integral IndexType>
		inline void IndexedSSBO<StructType, IndexType>::bind_base(GLuint index) const
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, this->buf);
		}

		template<typename StructType, std::unsigned_integral IndexType>
		class LightweightSSBO
		{
			GLBuffer buf;

		public:
			LightweightSSBO(IndexType size);

			GLuint buffer() const { return buf; }

			void bind_base(GLuint index) const;
			void send(IndexType pos, const StructType& obj) const;
			template<typename MemberType = StructType>
			void send(IndexType pos, MemberType StructType::*member, const MemberType& obj) const;
		};

		template<typename StructType, std::unsigned_integral IndexType>
		inline LightweightSSBO<StructType, IndexType>::LightweightSSBO(IndexType size)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
			glNamedBufferStorage(buf, size * sizeof(StructType), nullptr, GL_DYNAMIC_STORAGE_BIT);
		}

		template<typename StructType, std::unsigned_integral IndexType>
		inline void LightweightSSBO<StructType, IndexType>::bind_base(GLuint index) const
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buf);
		}

		template<typename StructType, std::unsigned_integral IndexType>
		inline void LightweightSSBO<StructType, IndexType>::send(IndexType pos, const StructType& obj) const
		{
			glNamedBufferSubData(buf, pos * sizeof(StructType), sizeof(StructType), &obj);
		}

		template<typename StructType, std::unsigned_integral IndexType>
		template<typename MemberType>
		inline void LightweightSSBO<StructType, IndexType>::send(IndexType pos, MemberType StructType::*member, const MemberType& obj) const
		{
			glNamedBufferSubData(buf, pos * sizeof(StructType) + member_offset(member), sizeof(MemberType), &obj);
		}

		template<typename StructType, std::unsigned_integral IndexType>
		class LightweightUBO
		{
			GLBuffer buf;

		public:
			LightweightUBO(IndexType size);

			GLuint buffer() const { return buf; }

			void bind_base(GLuint index) const;
			void send(IndexType pos, const StructType& obj) const;
		};

		template<typename StructType, std::unsigned_integral IndexType>
		inline LightweightUBO<StructType, IndexType>::LightweightUBO(IndexType size)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, buf);
			glNamedBufferStorage(buf, size * sizeof(StructType), nullptr, GL_DYNAMIC_STORAGE_BIT);
		}

		template<typename StructType, std::unsigned_integral IndexType>
		inline void LightweightUBO<StructType, IndexType>::bind_base(GLuint index) const
		{
			glBindBufferBase(GL_UNIFORM_BUFFER, index, buf);
		}

		template<typename StructType, std::unsigned_integral IndexType>
		inline void LightweightUBO<StructType, IndexType>::send(IndexType pos, const StructType& obj) const
		{
			glNamedBufferSubData(buf, pos * sizeof(StructType), sizeof(StructType), &obj);
		}

		enum class VertexAttributeType
		{
			FLOAT,
			INT,
			DOUBLE
		};

		template<VertexAttributeType Type = VertexAttributeType::FLOAT>
		struct VertexAttribute
		{
			static_assert(false);
		};

		template<>
		struct VertexAttribute<VertexAttributeType::FLOAT>
		{
			GLuint index;
			GLint size;
			GLint cols = 1;
			GLenum type = GL_FLOAT;
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
			GLint cols = 1;
			GLenum type = GL_UNSIGNED_INT;
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

		template<typename... StructTypes>
		class VertexBufferBlock
		{
		public:
			static constexpr size_t N = sizeof...(StructTypes);

		private:
			GLBufferBlock<N> bufblock;
			std::tuple<FixedVector<StructTypes>...> cpudata;

		public:
			template<std::unsigned_integral... IndexType>
			VertexBufferBlock(IndexType... sizes);
			template<std::unsigned_integral IndexType>
			VertexBufferBlock(IndexType size);

			constexpr GLuint buffer(GLsizei i) const { return bufblock[i]; }
			template<GLsizei i>
			const auto& vector() const { return std::get<i>(cpudata); }
			template<GLsizei i>
			auto& vector() { return std::get<i>(cpudata); }
			template<GLsizei i>
			constexpr size_t struct_size() const { return sizeof(std::tuple_element_t<i, std::tuple<StructTypes...>>); }

			template<VertexAttributeType... AttribTypes>
			void init_layout(const VertexAttribute<AttribTypes>&... attribs) const;

		private:
			template<size_t... Indexes, VertexAttributeType... AttribTypes>
			void init_layout_impl(std::index_sequence<Indexes...>, const VertexAttribute<AttribTypes>&... attribs) const;
		};

		template<typename... StructTypes>
		template<std::unsigned_integral... IndexType>
		inline VertexBufferBlock<StructTypes...>::VertexBufferBlock(IndexType... sizes)
			: cpudata(FixedVector<StructTypes>(sizes)...)
		{
		}

		template<typename... StructTypes>
		template<std::unsigned_integral IndexType>
		inline VertexBufferBlock<StructTypes...>::VertexBufferBlock(IndexType sizes)
			: cpudata(FixedVector<StructTypes>(sizes)...)
		{
		}

		template<typename... StructTypes>
		template<VertexAttributeType... AttribTypes>
		inline void VertexBufferBlock<StructTypes...>::init_layout(const VertexAttribute<AttribTypes>&... attribs) const
		{
			static_assert(sizeof...(AttribTypes) == N);
			init_layout_impl(std::make_index_sequence<sizeof...(StructTypes)>{}, attribs...);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		template<typename ...StructTypes>
		template<size_t ...Indexes, VertexAttributeType ...AttribTypes>
		inline void VertexBufferBlock<StructTypes...>::init_layout_impl(std::index_sequence<Indexes...>, const VertexAttribute<AttribTypes>&... attribs) const
		{
			((
				glBindBuffer(GL_ARRAY_BUFFER, bufblock[Indexes]),
				glNamedBufferStorage(bufblock[Indexes], std::get<Indexes>(cpudata).size() * sizeof(std::tuple_element_t<Indexes, std::tuple<StructTypes...>>),
					std::get<Indexes>(cpudata).data(), std::get<Indexes>(std::tie(attribs...)).storage_flags),
				std::get<Indexes>(std::tie(attribs...)).setup()
			), ...);
		}

		template<typename VertexBufferBlock, typename LazyMultiSender>
		class LazyVertexBufferBlock
		{
			VertexBufferBlock vertex_buffer;
			LazyMultiSender multi_sender;

			static_assert(VertexBufferBlock::N == LazyMultiSender::N);
			static constexpr size_t N = VertexBufferBlock::N;

		public:
			template<std::unsigned_integral... IndexType>
			LazyVertexBufferBlock(IndexType... sizes);

			const VertexBufferBlock& vbo() const { return vertex_buffer; }
			VertexBufferBlock& vbo() { return vertex_buffer; }
			const LazyMultiSender& sender() const { return multi_sender; }
			LazyMultiSender& sender() { return multi_sender; }

			template<size_t Attrib>
			void lazy_send(typename LazyMultiSender::template IndexType<Attrib> pos) const;
			void flush() const;

		private:
			template<size_t... Indexes>
			void flush_impl(std::index_sequence<Indexes...>) const;
		};

		template<typename VertexBufferBlock, typename LazyMultiSender>
		template<std::unsigned_integral... IndexType>
		inline LazyVertexBufferBlock<VertexBufferBlock, LazyMultiSender>::LazyVertexBufferBlock(IndexType... sizes)
			: vertex_buffer(sizes...)
		{
		}

		template<typename VertexBufferBlock, typename LazyMultiSender>
		template<size_t Attrib>
		inline void LazyVertexBufferBlock<VertexBufferBlock, LazyMultiSender>::lazy_send(typename LazyMultiSender::template IndexType<Attrib> pos) const
		{
			multi_sender.sender<Attrib>().lazy_send(pos);
		}

		template<typename VertexBufferBlock, typename LazyMultiSender>
		inline void LazyVertexBufferBlock<VertexBufferBlock, LazyMultiSender>::flush() const
		{
			flush_impl(std::make_index_sequence<N>{});
		}

		template<typename VertexBufferBlock, typename LazyMultiSender>
		template<size_t ...Indexes>
		inline void LazyVertexBufferBlock<VertexBufferBlock, LazyMultiSender>::flush_impl(std::index_sequence<Indexes...>) const
		{
			((multi_sender.sender<Indexes>().flush(vertex_buffer.buffer(Indexes), vertex_buffer.vector<Indexes>().data(), vertex_buffer.struct_size<Indexes>(),
				(typename LazyMultiSender::template IndexType<Indexes>)vertex_buffer.vector<Indexes>().size())), ...);
		}

		template<typename VertexAttrib, std::unsigned_integral IndexType>
		using LazyVertexBufferBlock1x1 = LazyVertexBufferBlock<
			VertexBufferBlock<VertexAttrib>,
			LazyMultiSender<IndexType>>;

		template<typename VertexAttrib1, typename VertexAttrib2, std::unsigned_integral IndexType>
		using LazyVertexBufferBlock2x1 = LazyVertexBufferBlock<
			VertexBufferBlock<VertexAttrib1, VertexAttrib2>,
			LazyMultiSender<IndexType, IndexType>>;

		template<typename VertexAttrib1, typename VertexAttrib2, typename VertexAttrib3, std::unsigned_integral IndexType>
		using LazyVertexBufferBlock3x1 = LazyVertexBufferBlock<
			VertexBufferBlock<VertexAttrib1, VertexAttrib2, VertexAttrib3>,
			LazyMultiSender<IndexType, IndexType, IndexType>>;

		template<typename VertexAttrib1, typename VertexAttrib2, typename VertexAttrib3, typename VertexAttrib4, std::unsigned_integral IndexType>
		using LazyVertexBufferBlock4x1 = LazyVertexBufferBlock<
			VertexBufferBlock<VertexAttrib1, VertexAttrib2, VertexAttrib3, VertexAttrib4>,
			LazyMultiSender<IndexType, IndexType, IndexType, IndexType>>;

		template<typename VertexAttrib1, typename VertexAttrib2, typename VertexAttrib3, typename VertexAttrib4, typename VertexAttrib5, std::unsigned_integral IndexType>
		using LazyVertexBufferBlock5x1 = LazyVertexBufferBlock<
			VertexBufferBlock<VertexAttrib1, VertexAttrib2, VertexAttrib3, VertexAttrib4, VertexAttrib5>,
			LazyMultiSender<IndexType, IndexType, IndexType, IndexType, IndexType>>;

		template<typename VertexAttrib1, typename VertexAttrib2, typename VertexAttrib3, typename VertexAttrib4, typename VertexAttrib5, typename VertexAttrib6, std::unsigned_integral IndexType>
		using LazyVertexBufferBlock6x1 = LazyVertexBufferBlock<
			VertexBufferBlock<VertexAttrib1, VertexAttrib2, VertexAttrib3, VertexAttrib4, VertexAttrib5, VertexAttrib6>,
			LazyMultiSender<IndexType, IndexType, IndexType, IndexType, IndexType, IndexType>>;
	}
}
