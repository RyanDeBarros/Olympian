#pragma once

#include <set>

#include "core/Core.h"
#include "util/FixedVector.h"

namespace oly
{
	namespace rendering
	{
		template<typename IndexType, size_t LayoutSize>
		class FixedLayoutEBO
		{
			template<size_t Size>
			struct Layout
			{
				IndexType data[Size];
			};

			FixedVector<Layout<LayoutSize>> cpudata;
			GLBuffer ebo;

			struct
			{
				IndexType first = 0;
				IndexType count = 0;
				IndexType offset = 0;
			} draw_spec;

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
			FixedLayoutEBO(IndexType size, BufferSendType buffer_send_type = BufferSendType::SUBDATA, bool allow_map_send = true);

			const FixedVector<Layout<LayoutSize>>& vector() const { return cpudata; }
			FixedVector<Layout<LayoutSize>>& vector() { return cpudata; }
			GLuint buffer() const { return ebo; }
			BufferSendType get_buffer_send_type() const { return buffer_send_type; }
			void set_buffer_send_type(BufferSendType type) { if (allow_map_send) buffer_send_type = type; }

			void get_draw_spec(IndexType& first, IndexType& count) const { first = draw_spec.first; count = draw_spec.count; }
			void set_draw_spec(IndexType first, IndexType count);

			void init() const;
			void draw(GLenum mode, GLenum type) const;
			void lazy_send(IndexType index) const;
			void flush() const;
		};

		template<typename IndexType, size_t LayoutSize>
		inline FixedLayoutEBO<IndexType, LayoutSize>::FixedLayoutEBO(IndexType size, BufferSendType buffer_send_type, bool allow_map_send)
			: cpudata(size), allow_map_send(allow_map_send), buffer_send_type(allow_map_send ? buffer_send_type : BufferSendType::SUBDATA)
		{
		}

		template<typename IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::set_draw_spec(IndexType first, IndexType count)
		{
			if (first < cpudata.size())
				draw_spec.first = first;
			draw_spec.count = 6 * std::min(count, (IndexType)(cpudata.size() - draw_spec.first));
			draw_spec.offset = (IndexType)(draw_spec.first * sizeof(Layout<LayoutSize>));
		}

		template<typename IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::init() const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			if (allow_map_send)
				glNamedBufferStorage(ebo, cpudata.size() * sizeof(Layout<LayoutSize>), cpudata.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
			else
				glNamedBufferStorage(ebo, cpudata.size() * sizeof(Layout<LayoutSize>), cpudata.data(), GL_DYNAMIC_STORAGE_BIT);
		}

		template<typename IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::draw(GLenum mode, GLenum type) const
		{
			glDrawElements(mode, (GLsizei)draw_spec.count, type, (void*)(draw_spec.offset));
		}

		template<typename IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::lazy_send(IndexType index) const
		{
			dirty.insert(index);
		}

		template<typename IndexType, size_t LayoutSize>
		inline void FixedLayoutEBO<IndexType, LayoutSize>::flush() const
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
						if (*iter * sizeof(Layout<LayoutSize>) == offset + size)
							size += sizeof(Layout<LayoutSize>);
						else
						{
							glNamedBufferSubData(ebo, offset, size, data + offset);
							contiguous = false;
						}
					}
					else
					{
						offset = *iter * sizeof(Layout<LayoutSize>);
						size = sizeof(Layout<LayoutSize>);
						contiguous = true;
					}
				}
				if (contiguous)
					glNamedBufferSubData(ebo, offset, size, data + offset);
				break;
			}
			case BufferSendType::MAP:
			{
				std::byte* gpu_buf = (std::byte*)glMapNamedBuffer(ebo, GL_WRITE_ONLY);
				bool contiguous = false;
				GLintptr offset = 0;
				GLsizeiptr size = 0;
				for (auto iter = dirty.begin(); iter != dirty.end(); ++iter)
				{
					if (contiguous)
					{
						if (*iter * sizeof(Layout<LayoutSize>) == offset + size)
							size += sizeof(Layout<LayoutSize>);
						else
						{
							memcpy(gpu_buf + offset, data + offset, size);
							contiguous = false;
						}
					}
					else
					{
						offset = *iter * sizeof(Layout<LayoutSize>);
						size = sizeof(Layout<LayoutSize>);
						contiguous = true;
					}
				}
				if (contiguous)
					memcpy(gpu_buf + offset, data + offset, size);
				glUnmapNamedBuffer(ebo);
				break;
			}
			}
			dirty.clear();
		}

		template<typename StructType, typename IndexType>
		class IndexedSSBO
		{
			FixedVector<StructType> cpudata;
			GLBuffer ssbo;
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
			IndexedSSBO(IndexType size, const StructType& default_value = StructType(), BufferSendType buffer_send_type = BufferSendType::SUBDATA, bool allow_map_send = true);

			const FixedVector<StructType>& vector() const { return cpudata; }
			FixedVector<StructType>& vector() { return cpudata; }
			GLuint buffer() const { return ssbo; }
			BufferSendType get_buffer_send_type() const { return buffer_send_type; }
			void set_buffer_send_type(BufferSendType type) { if (allow_map_send) buffer_send_type = type; }

			void bind_base(GLuint index) const;
			void lazy_send(IndexType index) const;
			void flush() const;
		};

		template<typename StructType, typename IndexType>
		inline IndexedSSBO<StructType, IndexType>::IndexedSSBO(IndexType size, const StructType& default_value, BufferSendType buffer_send_type, bool allow_map_send)
			: cpudata(size, default_value), allow_map_send(allow_map_send), buffer_send_type(allow_map_send ? buffer_send_type : BufferSendType::SUBDATA)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
			if (allow_map_send)
				glNamedBufferStorage(ssbo, size * sizeof(StructType), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
			else
				glNamedBufferStorage(ssbo, size * sizeof(StructType), nullptr, GL_DYNAMIC_STORAGE_BIT);
		}

		template<typename StructType, typename IndexType>
		inline void IndexedSSBO<StructType, IndexType>::bind_base(GLuint index) const
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ssbo);
		}

		template<typename StructType, typename IndexType>
		inline void IndexedSSBO<StructType, IndexType>::lazy_send(IndexType index) const
		{
			dirty.insert(index);
		}

		template<typename StructType, typename IndexType>
		inline void IndexedSSBO<StructType, IndexType>::flush() const
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
							glNamedBufferSubData(ssbo, offset, size, data + offset);
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
					glNamedBufferSubData(ssbo, offset, size, data + offset);
				break;
			}
			case BufferSendType::MAP:
			{
				std::byte* gpu_buf = (std::byte*)glMapNamedBuffer(ssbo, GL_WRITE_ONLY);
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
				glUnmapNamedBuffer(ssbo);
				break;
			}
			}
			dirty.clear();
		}

		typedef FixedLayoutEBO<GLushort, 6> QuadLayoutEBO;
		typedef IndexedSSBO<glm::mat3, GLushort> IndexedTransformSSBO;

		inline void pre_init(QuadLayoutEBO& ebo)
		{
			for (GLushort i = 0; i < ebo.vector().size(); ++i)
				rendering::quad_indices(ebo.vector()[i].data, i);
		}
	}
}
