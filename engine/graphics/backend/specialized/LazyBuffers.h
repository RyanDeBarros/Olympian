#pragma once

#include <set>

#include "external/GL.h"
#include "core/containers/FixedVector.h"
#include "graphics/backend/basic/Buffers.h"
#include "graphics/backend/specialized/Mutability.h"

namespace oly::graphics
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
		CPUSideBuffer(size_t size = 0, const StructType& default_value = StructType()) : cpudata(size, default_value) {}
		
		virtual ~CPUSideBuffer() = default;

		const VectorAlias& vector() const { return cpudata; }
		VectorAlias& vector() { return cpudata; }
		GLuint buffer() const { return buf; }

		void init(GLbitfield flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT) const requires (M == Mutability::IMMUTABLE)
		{
			glNamedBufferStorage(this->buf, this->cpudata.size() * sizeof(StructType), this->cpudata.data(), flags);
		}

		void init(GLenum usage = GL_DYNAMIC_DRAW) const requires (M == Mutability::MUTABLE)
		{
			glNamedBufferData(this->buf, this->cpudata.size() * sizeof(StructType), this->cpudata.data(), usage);
		}
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
		{
			batch_send(dirty.begin(), dirty.end(), this->buf, this->cpudata.data(), sizeof(StructType), (IndexType)this->cpudata.size(), send_type); dirty.clear();
		}
	};
}
