#pragma once

#include "external/GLM.h"
#include "core/base/Errors.h"

namespace oly::particles
{
	class ConstFloatSpan;

	// FloatSpan is a non-owning view of a float N-vector. The data used to initialize the span must still be alive when using the span.
	class FloatSpan
	{
		friend class ConstFloatSpan;

		float* const _data;
		const size_t _size;

	public:
		explicit FloatSpan(float& data) : _data(&data), _size(1) {}
		template<size_t N>
		explicit FloatSpan(glm::vec<N, float>& data) : _data(glm::value_ptr(data)), _size(N) {}
		explicit FloatSpan(std::vector<float>& vector) : _data(vector.data()), _size(vector.size()) {}
		explicit FloatSpan(std::nullptr_t) : _data(nullptr), _size(0) {}

		size_t size() const
		{
			return _size;
		}

		float operator[](size_t i) const
		{
			if (i >= _size)
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return _data[i];
		}

		float& operator[](size_t i)
		{
			if (i >= _size)
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return _data[i];
		}

		template<size_t N>
		const glm::vec<N, float>& glm() const
		{
			if (N > _size)
				throw Error(ErrorCode::INVALID_SIZE);
			return *reinterpret_cast<const glm::vec<N, float>*>(_data);
		}

		template<size_t N>
		glm::vec<N, float>& glm()
		{
			if (N > _size)
				throw Error(ErrorCode::INVALID_SIZE);
			return *reinterpret_cast<glm::vec<N, float>*>(_data);
		}
	};

	// ConstFloatSpan is a const non-owning view of a float N-vector. The data used to initialize the span must still be alive when using the span.
	class ConstFloatSpan
	{
		const float* const _data;
		const size_t _size;

	public:
		explicit ConstFloatSpan(const float& data) : _data(&data), _size(1) {}
		template<size_t N>
		explicit ConstFloatSpan(const glm::vec<N, float>& data) : _data(glm::value_ptr(data)), _size(N) {}
		explicit ConstFloatSpan(const std::vector<float>& vector) : _data(vector.data()), _size(vector.size()) {}
		explicit ConstFloatSpan(FloatSpan span) : _data(span._data), _size(span._size) {}
		explicit ConstFloatSpan(std::nullptr_t) : _data(nullptr), _size(0) {}

		size_t size() const
		{
			return _size;
		}

		float operator[](size_t i) const
		{
			if (i >= _size)
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return _data[i];
		}

		template<size_t N>
		const glm::vec<N, float>& glm() const
		{
			if (N > _size)
				throw Error(ErrorCode::INVALID_SIZE);
			return *reinterpret_cast<const glm::vec<N, float>*>(_data);
		}
	};

	template<glm::length_t N>
	using GLMVector = std::conditional_t<(N > 1), glm::vec<N, float>, float>;
}
