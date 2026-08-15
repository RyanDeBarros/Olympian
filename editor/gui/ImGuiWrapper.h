#pragma once

#include <imtk.hpp>

#include <imp/potential.hpp>

namespace oly::editor::gui
{
	template<typename T>
	struct InputData;

	template<typename T>
	bool Clamp(T& data, const T og, imp::potential<T> min, imp::potential<T> max)
	{
		if (max.has_value)
			data = std::min(data, max.value);
		if (min.has_value)
			data = std::max(data, min.value);
		return data != og;
	}

	template<typename T>
	bool Clamp(T* data, const T* og, size_t count, imp::potential<T> min, imp::potential<T> max)
	{
		bool dirty = false;
		for (size_t i = 0; i < count; ++i)
			dirty |= Clamp(data[i], og[i], min, max);
		return dirty;
	}

	template<typename T, glm::length_t L>
	bool Clamp(glm::vec<L, T>& data, const glm::vec<L, T> og, imp::potential<T> min, imp::potential<T> max)
	{
		return Clamp(glm::value_ptr(data), glm::value_ptr(og), L, min, max);
	}

	template<typename T, size_t N>
	bool Clamp(std::array<T, N>& data, const std::array<T, N> og, imp::potential<T> min, imp::potential<T> max)
	{
		return Clamp(data.data(), og.data(), N, min, max);
	}

	template<typename T, typename U>
	imtk::item_result InputClampedData(const char* label, T& data, imp::potential<U> min, imp::potential<U> max)
	{
		const auto og = data;
		auto result = InputData<T>{}(label, data);
		result.modified = Clamp(data, og, min, max);
		return result;
	}

	template<>
	struct InputData<bool>
	{
		imtk::item_result operator()(const char* label, bool& data) const;
	};

	template<>
	struct InputData<int>
	{
		imtk::item_result operator()(const char* label, int& data) const;
		imtk::item_result operator()(const char* label, int& data, imp::potential<int> min, imp::potential<int> max) const;
		imtk::item_result operator()(const char* label, int& data, imtk::label_span_registry::handle names);
	};

	template<>
	struct InputData<float>
	{
		imtk::item_result operator()(const char* label, float& data) const;
		imtk::item_result operator()(const char* label, float& data, imp::potential<float> min, imp::potential<float> max) const;
	};

	template<>
	struct InputData<double>
	{
		imtk::item_result operator()(const char* label, double& data) const;
		imtk::item_result operator()(const char* label, double& data, imp::potential<double> min, imp::potential<double> max) const;
	};

	template<>
	struct InputData<glm::vec2>
	{
		imtk::item_result operator()(const char* label, glm::vec2& data) const;
		imtk::item_result operator()(const char* label, glm::vec2& data, imp::potential<float> min, imp::potential<float> max) const;
	};

	template<>
	struct InputData<glm::vec3>
	{
		imtk::item_result operator()(const char* label, glm::vec3& data) const;
		imtk::item_result operator()(const char* label, glm::vec3& data, imp::potential<float> min, imp::potential<float> max) const;
	};

	template<>
	struct InputData<glm::vec4>
	{
		imtk::item_result operator()(const char* label, glm::vec4& data) const;
		imtk::item_result operator()(const char* label, glm::vec4& data, imp::potential<float> min, imp::potential<float> max) const;
	};

	template<>
	struct InputData<std::string>
	{
		imtk::item_result operator()(const char* label, std::string& data) const;
	};
}
