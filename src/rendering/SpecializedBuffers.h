#pragma once

#include <set>

#include "core/Core.h"
#include "util/FixedVector.h"

namespace oly
{
	namespace rendering
	{
		template<typename StructType, typename IndexType>
		class LazyBuffer
		{
		protected:
			using StructTypeAlias = StructType;

			FixedVector<StructType> cpudata;
			GLBuffer buf;

		private:
			bool allow_map_send;

		public:
			enum class BufferSendType
			{
				SUBDATA,
				MAP
			};

		private:
			BufferSendType buffer_send_type;
			mutable std::set<IndexType> dirty;

		public:
			LazyBuffer(IndexType size, const StructType& default_value = StructType(), BufferSendType buffer_send_type = BufferSendType::SUBDATA, bool allow_map_send = true);
			virtual ~LazyBuffer() = default;

		protected:
			void init_storage() const;

		public:
			const FixedVector<StructType>& vector() const { return cpudata; }
			FixedVector<StructType>& vector() { return cpudata; }
			GLuint buffer() const { return buf; }
			BufferSendType get_buffer_send_type() const { return buffer_send_type; }
			void set_buffer_send_type(BufferSendType type) { if (allow_map_send) buffer_send_type = type; }
			
			void lazy_send(IndexType index) const;
			void flush() const;
		};

		template<typename StructType, typename IndexType>
		inline LazyBuffer<StructType, IndexType>::LazyBuffer(IndexType size, const StructType& default_value, BufferSendType buffer_send_type, bool allow_map_send)
			: cpudata(size, default_value), allow_map_send(allow_map_send), buffer_send_type(allow_map_send ? buffer_send_type : BufferSendType::SUBDATA)
		{
		}

		template<typename StructType, typename IndexType>
		inline void LazyBuffer<StructType, IndexType>::init_storage() const
		{
			if (allow_map_send)
				glNamedBufferStorage(buf, cpudata.size() * sizeof(StructType), cpudata.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
			else
				glNamedBufferStorage(buf, cpudata.size() * sizeof(StructType), cpudata.data(), GL_DYNAMIC_STORAGE_BIT);
		}

		template<typename StructType, typename IndexType>
		inline void LazyBuffer<StructType, IndexType>::lazy_send(IndexType index) const
		{
			dirty.insert(index);
		}

		template<typename StructType, typename IndexType>
		inline void LazyBuffer<StructType, IndexType>::flush() const
		{
			std::byte* data = (std::byte*)cpudata.data();
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
						if (*iter * sizeof(StructType) == offset + size)
							size += sizeof(StructType);
						else
						{
							glNamedBufferSubData(buf, offset, size, data + offset);
							contiguous = false;
						}
					}
					else
					{
						offset = *iter * sizeof(StructType);
						size = sizeof(StructType);
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
						if (*iter * sizeof(StructType) == offset + size)
							size += sizeof(StructType);
						else
						{
							memcpy(gpu_buf + offset, data + offset, size);
							contiguous = false;
						}
					}
					else
					{
						offset = *iter * sizeof(StructType);
						size = sizeof(StructType);
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

		template<typename IndexType, size_t Size>
		struct FixedIndexLayout
		{
			IndexType data[Size];
		};

		template<typename IndexType, size_t LayoutSize>
		class FixedLayoutEBO : public LazyBuffer<FixedIndexLayout<IndexType, LayoutSize>, IndexType>
		{
			using Parent = LazyBuffer<FixedIndexLayout<IndexType, LayoutSize>, IndexType>;

			struct
			{
				IndexType first = 0;
				IndexType count = 0;
				IndexType offset = 0;
			} draw_spec;

		public:
			FixedLayoutEBO(IndexType size, Parent::BufferSendType buffer_send_type = Parent::BufferSendType::SUBDATA, bool allow_map_send = true);

			void get_draw_spec(IndexType& first, IndexType& count) const { first = draw_spec.first; count = draw_spec.count; }
			void set_draw_spec(IndexType first, IndexType count);

			void init() const;
			void draw(GLenum mode, GLenum type) const;
		};

		template<typename IndexType, size_t LayoutSize>
		inline FixedLayoutEBO<IndexType, LayoutSize>::FixedLayoutEBO(IndexType size, Parent::BufferSendType buffer_send_type, bool allow_map_send)
			: Parent(size, {}, buffer_send_type, allow_map_send)
		{
		}

		template<typename IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::set_draw_spec(IndexType first, IndexType count)
		{
			if (first < Parent::cpudata.size())
				draw_spec.first = first;
			draw_spec.count = 6 * std::min(count, (IndexType)(Parent::cpudata.size() - draw_spec.first));
			draw_spec.offset = (IndexType)(draw_spec.first * sizeof(Parent::StructTypeAlias));
		}

		template<typename IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::init() const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Parent::buf);
			Parent::init_storage();
		}

		template<typename IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::draw(GLenum mode, GLenum type) const
		{
			glDrawElements(mode, (GLsizei)draw_spec.count, type, (void*)(draw_spec.offset));
		}

		template<typename StructType, typename IndexType>
		class IndexedSSBO : public LazyBuffer<StructType, IndexType>
		{
			using Parent = LazyBuffer<StructType, IndexType>;
		public:
			IndexedSSBO(IndexType size, const StructType& default_value = StructType(), Parent::BufferSendType buffer_send_type = Parent::BufferSendType::SUBDATA, bool allow_map_send = true);

			void bind_base(GLuint index) const;
		};

		template<typename StructType, typename IndexType>
		inline IndexedSSBO<StructType, IndexType>::IndexedSSBO(IndexType size, const StructType& default_value, Parent::BufferSendType buffer_send_type, bool allow_map_send)
			: Parent(size, default_value, buffer_send_type, allow_map_send)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, Parent::buf);
			Parent::init_storage();
		}

		template<typename StructType, typename IndexType>
		inline void IndexedSSBO<StructType, IndexType>::bind_base(GLuint index) const
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, Parent::buf);
		}

		// TODO UBOs

		typedef FixedLayoutEBO<GLushort, 6> QuadLayoutEBO;
		typedef IndexedSSBO<glm::mat3, GLushort> IndexedTransformSSBO;

		inline void pre_init(QuadLayoutEBO& ebo)
		{
			for (GLushort i = 0; i < ebo.vector().size(); ++i)
				rendering::quad_indices(ebo.vector()[i].data, i);
		}
	}
}
