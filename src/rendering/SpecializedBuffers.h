#pragma once

#include <set>

#include "core/Core.h"
#include "util/FixedVector.h"
#include "util/General.h"

namespace oly
{
	namespace rendering
	{
		template<typename StructType, typename IndexType>
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

		template<typename StructType, typename IndexType>
		inline FixedVectorBuffer<StructType, IndexType>::FixedVectorBuffer(IndexType size, const StructType& default_value)
			: cpudata(size, default_value)
		{
		}

		enum class BufferSendType
		{
			SUBDATA,
			MAP
		};

		template<typename IndexType>
		class LazySender
		{
			bool allow_map_send;
			BufferSendType buffer_send_type;
			mutable std::set<IndexType> dirty;

		public:
			LazySender(BufferSendType buffer_send_type = BufferSendType::SUBDATA, bool allow_map_send = true);

			BufferSendType get_buffer_send_type() const { return buffer_send_type; }
			void set_buffer_send_type(BufferSendType type) { if (allow_map_send) buffer_send_type = type; }

			void init_storage(GLuint buf, const void* data, GLsizeiptr size) const;
			void lazy_send(IndexType pos) const;
			void flush(GLuint buf, const void* cpudata, GLsizeiptr struct_size) const;
		};

		template<typename IndexType>
		inline LazySender<IndexType>::LazySender(BufferSendType buffer_send_type, bool allow_map_send)
			: allow_map_send(allow_map_send), buffer_send_type(allow_map_send ? buffer_send_type : BufferSendType::SUBDATA)
		{
		}

		template<typename IndexType>
		inline void LazySender<IndexType>::init_storage(GLuint buf, const void* data, GLsizeiptr size) const
		{
			if (allow_map_send)
				glNamedBufferStorage(buf, size, data, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
			else
				glNamedBufferStorage(buf, size, data, GL_DYNAMIC_STORAGE_BIT);
		}

		template<typename IndexType>
		inline void LazySender<IndexType>::lazy_send(IndexType pos) const
		{
			dirty.insert(pos);
		}

		template<typename IndexType>
		inline void LazySender<IndexType>::flush(GLuint buf, const void* cpudata, GLsizeiptr struct_size) const
		{
			// TODO add checking for out-of-range access of cpudata
			const std::byte* data = (const std::byte*)cpudata;
			switch (buffer_send_type)
			{
			case BufferSendType::SUBDATA:
			{
				bool contiguous = false;
				GLintptr offset = 0;
				GLsizeiptr size = 0;
				for (auto iter = dirty.begin(); iter != dirty.end(); ++iter)
				{
					if (contiguous)
					{
						if (*iter * struct_size == offset + size)
							size += struct_size;
						else
						{
							glNamedBufferSubData(buf, offset, size, data + offset);
							contiguous = false;
						}
					}
					else
					{
						offset = *iter * struct_size;
						size = struct_size;
						contiguous = true;
					}
				}
				if (contiguous)
					glNamedBufferSubData(buf, offset, size, data + offset);
				break;
			}
			case BufferSendType::MAP:
			{
				std::byte* gpu_buf = (std::byte*)glMapNamedBuffer(buf, GL_WRITE_ONLY);
				bool contiguous = false;
				GLintptr offset = 0;
				GLsizeiptr size = 0;
				for (auto iter = dirty.begin(); iter != dirty.end(); ++iter)
				{
					if (contiguous)
					{
						if (*iter * struct_size == offset + size)
							size += struct_size;
						else
						{
							memcpy(gpu_buf + offset, data + offset, size);
							contiguous = false;
						}
					}
					else
					{
						offset = *iter * struct_size;
						size = struct_size;
						contiguous = true;
					}
				}
				if (contiguous)
					memcpy(gpu_buf + offset, data + offset, size);
				glUnmapNamedBuffer(buf);
				break;
			}
			}
			dirty.clear();
		}

		template<typename StructType, typename IndexType>
		class LazyBuffer : public FixedVectorBuffer<StructType, IndexType>
		{
		protected:
			using StructTypeAlias = StructType;

		private:
			LazySender<IndexType> lazy;

		public:
			LazyBuffer(IndexType size, const StructType& default_value = StructType(), BufferSendType buffer_send_type = BufferSendType::SUBDATA, bool allow_map_send = true);

		protected:
			void init_storage() const { lazy.init_storage(this->buf, this->cpudata.data(), this->cpudata.size() * sizeof(StructType)); }

		public:
			void lazy_send(IndexType pos) const { lazy.lazy_send(pos); }
			void flush() const { lazy.flush(this->buf, this->cpudata.data(), sizeof(StructType)); }
		};

		template<typename StructType, typename IndexType>
		inline LazyBuffer<StructType, IndexType>::LazyBuffer(IndexType size, const StructType& default_value, BufferSendType buffer_send_type, bool allow_map_send)
			: FixedVectorBuffer<StructType, IndexType>(size, default_value), lazy(buffer_send_type, allow_map_send)
		{
		}

		template<typename IndexType, size_t Size>
		struct FixedIndexLayout
		{
			IndexType data[Size];
		};

		template<typename IndexType, size_t LayoutSize>
		class FixedLayoutEBO : public LazyBuffer<FixedIndexLayout<IndexType, LayoutSize>, IndexType>
		{
			struct
			{
				IndexType first = 0;
				IndexType count = 0;
				IndexType offset = 0;
			} draw_spec;

		public:
			FixedLayoutEBO(IndexType size, BufferSendType buffer_send_type = BufferSendType::SUBDATA, bool allow_map_send = true);

			void get_draw_spec(IndexType& first, IndexType& count) const { first = draw_spec.first; count = draw_spec.count; }
			void set_draw_spec(IndexType first, IndexType count);

			void init() const;
			void draw(GLenum mode, GLenum type) const;
		};

		template<typename IndexType, size_t LayoutSize>
		inline FixedLayoutEBO<IndexType, LayoutSize>::FixedLayoutEBO(IndexType size, BufferSendType buffer_send_type, bool allow_map_send)
			: LazyBuffer<FixedIndexLayout<IndexType, LayoutSize>, IndexType>(size, {}, buffer_send_type, allow_map_send)
		{
			set_draw_spec(0, size);
		}

		template<typename IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::set_draw_spec(IndexType first, IndexType count)
		{
			if (first < this->cpudata.size())
				draw_spec.first = first;
			draw_spec.count = 6 * std::min(count, (IndexType)(this->cpudata.size() - draw_spec.first));
			draw_spec.offset = (IndexType)(draw_spec.first * sizeof(FixedIndexLayout<IndexType, LayoutSize>));
		}

		template<typename IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::init() const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buf);
			this->init_storage();
		}

		template<typename IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::draw(GLenum mode, GLenum type) const
		{
			glDrawElements(mode, (GLsizei)draw_spec.count, type, (void*)(draw_spec.offset));
		}

		template<typename IndexType>
		class FixedLayoutEBO<IndexType, 1> : public LazyBuffer<IndexType, IndexType>
		{
			struct
			{
				IndexType first = 0;
				IndexType count = 0;
				IndexType offset = 0;
			} draw_spec;

		public:
			FixedLayoutEBO(IndexType size, BufferSendType buffer_send_type = BufferSendType::SUBDATA, bool allow_map_send = true);

			void get_draw_spec(IndexType& first, IndexType& count) const { first = draw_spec.first; count = draw_spec.count; }
			void set_draw_spec(IndexType first, IndexType count);

			void init() const;
			void draw(GLenum mode, GLenum type) const;
		};

		template<typename IndexType>
		inline FixedLayoutEBO<IndexType, 1>::FixedLayoutEBO(IndexType size, BufferSendType buffer_send_type, bool allow_map_send)
			: LazyBuffer<IndexType, IndexType>(size, {}, buffer_send_type, allow_map_send)
		{
			set_draw_spec(0, size);
		}

		template<typename IndexType>
		inline void FixedLayoutEBO<IndexType, 1>::set_draw_spec(IndexType first, IndexType count)
		{
			if (first < this->cpudata.size())
				draw_spec.first = first;
			draw_spec.count = 6 * std::min(count, (IndexType)(this->cpudata.size() - draw_spec.first));
			draw_spec.offset = (IndexType)(draw_spec.first * sizeof(IndexType));
		}

		template<typename IndexType>
		inline void FixedLayoutEBO<IndexType, 1>::init() const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buf);
			this->init_storage();
		}

		template<typename IndexType>
		inline void FixedLayoutEBO<IndexType, 1>::draw(GLenum mode, GLenum type) const
		{
			glDrawElements(mode, (GLsizei)draw_spec.count, type, (void*)(draw_spec.offset));
		}

		template<typename StructType, typename IndexType>
		class IndexedSSBO : public LazyBuffer<StructType, IndexType>
		{
		public:
			IndexedSSBO(IndexType size, const StructType& default_value = StructType(), BufferSendType buffer_send_type = BufferSendType::SUBDATA, bool allow_map_send = true);

			void bind_base(GLuint index) const;
		};

		template<typename StructType, typename IndexType>
		inline IndexedSSBO<StructType, IndexType>::IndexedSSBO(IndexType size, const StructType& default_value, BufferSendType buffer_send_type, bool allow_map_send)
			: LazyBuffer<StructType, IndexType>(size, default_value, buffer_send_type, allow_map_send)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->buf);
			this->init_storage();
		}

		template<typename StructType, typename IndexType>
		inline void IndexedSSBO<StructType, IndexType>::bind_base(GLuint index) const
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, this->buf);
		}

		template<typename StructType, typename IndexType>
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

		template<typename StructType, typename IndexType>
		inline LightweightSSBO<StructType, IndexType>::LightweightSSBO(IndexType size)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
			glNamedBufferStorage(buf, size * sizeof(StructType), nullptr, GL_DYNAMIC_STORAGE_BIT);
		}

		template<typename StructType, typename IndexType>
		inline void LightweightSSBO<StructType, IndexType>::bind_base(GLuint index) const
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buf);
		}

		template<typename StructType, typename IndexType>
		inline void LightweightSSBO<StructType, IndexType>::send(IndexType pos, const StructType& obj) const
		{
			glNamedBufferSubData(buf, pos * sizeof(StructType), sizeof(StructType), &obj);
		}

		template<typename StructType, typename IndexType>
		template<typename MemberType>
		inline void LightweightSSBO<StructType, IndexType>::send(IndexType pos, MemberType StructType::*member, const MemberType& obj) const
		{
			glNamedBufferSubData(buf, pos * sizeof(StructType) + member_offset(member), sizeof(MemberType), &obj);
		}

		template<typename StructType, typename IndexType>
		class LightweightUBO
		{
			GLBuffer buf;

		public:
			LightweightUBO(IndexType size);

			GLuint buffer() const { return buf; }

			void bind_base(GLuint index) const;
			void send(IndexType pos, const StructType& obj) const;
		};

		template<typename StructType, typename IndexType>
		inline LightweightUBO<StructType, IndexType>::LightweightUBO(IndexType size)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, buf);
			glNamedBufferStorage(buf, size * sizeof(StructType), nullptr, GL_DYNAMIC_STORAGE_BIT);
		}

		template<typename StructType, typename IndexType>
		inline void LightweightUBO<StructType, IndexType>::bind_base(GLuint index) const
		{
			glBindBufferBase(GL_UNIFORM_BUFFER, index, buf);
		}

		template<typename StructType, typename IndexType>
		inline void LightweightUBO<StructType, IndexType>::send(IndexType pos, const StructType& obj) const
		{
			glNamedBufferSubData(buf, pos * sizeof(StructType), sizeof(StructType), &obj);
		}

		typedef FixedLayoutEBO<GLushort, 6> QuadLayoutEBO;

		inline void pre_init(QuadLayoutEBO& ebo)
		{
			for (GLushort i = 0; i < ebo.vector().size(); ++i)
				rendering::quad_indices(ebo.vector()[i].data, i);
		}
}
}
