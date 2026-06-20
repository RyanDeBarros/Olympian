#pragma once

#include "core/Meta.h"
#include "core/Types.h"
#include "desc/OptionalPrimitive.h"

#include "gui/DisabledSection.h"
#include "gui/IDScope.h"
#include "gui/DrawResult.h"

#include "external/GLM.h"

#include <string>
#include <vector>

#include <imgui.h>

namespace oly::editor::gui
{
	extern void VerticalSeparator();

	extern DrawResult Combo(const char* label, int& current_item, const std::vector<std::string>& items);

	extern DrawResult InputText(const char* label, std::string& string, size_t max_size = 256,
		ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	template<typename T>
	struct InputData;

	template<typename T>
	bool Clamp(T& data, const T og, OptionalPrimitive<T> min, OptionalPrimitive<T> max)
	{
		if (max.has_value)
			data = std::min(data, max.value);
		if (min.has_value)
			data = std::max(data, min.value);
		return data != og;
	}

	template<typename T>
	bool Clamp(T* data, const T* og, size_t count, OptionalPrimitive<T> min, OptionalPrimitive<T> max)
	{
		bool dirty = false;
		for (size_t i = 0; i < count; ++i)
			dirty |= Clamp(data[i], og[i], min, max);
		return dirty;
	}

	template<typename T, glm::length_t L>
	bool Clamp(glm::vec<L, T>& data, const glm::vec<L, T> og, OptionalPrimitive<T> min, OptionalPrimitive<T> max)
	{
		return Clamp(glm::value_ptr(data), glm::value_ptr(og), L, min, max);
	}

	template<typename T, size_t N>
	bool Clamp(std::array<T, N>& data, const std::array<T, N> og, OptionalPrimitive<T> min, OptionalPrimitive<T> max)
	{
		return Clamp(data.data(), og.data(), N, min, max);
	}

	template<typename T, typename U>
	DrawResult InputClampedData(const char* label, T& data, OptionalPrimitive<U> min, OptionalPrimitive<U> max)
	{
		const auto og = data;
		auto result = InputData<T>{}(label, data);
		result.SetDirty(Clamp(data, og, min, max));
		return result;
	}

	template<>
	struct InputData<bool>
	{
		DrawResult operator()(const char* label, bool& data) const;
	};

	template<>
	struct InputData<int>
	{
		DrawResult operator()(const char* label, int& data) const;
		DrawResult operator()(const char* label, int& data, OptionalPrimitive<int> min, OptionalPrimitive<int> max) const;
		DrawResult operator()(const char* label, int& data, const char** names, size_t count);
	};

	template<>
	struct InputData<float>
	{
		DrawResult operator()(const char* label, float& data) const;
		DrawResult operator()(const char* label, float& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const;
	};

	template<>
	struct InputData<double>
	{
		DrawResult operator()(const char* label, double& data) const;
		DrawResult operator()(const char* label, double& data, OptionalPrimitive<double> min, OptionalPrimitive<double> max) const;
	};

	template<>
	struct InputData<glm::vec2>
	{
		DrawResult operator()(const char* label, glm::vec2& data) const;
		DrawResult operator()(const char* label, glm::vec2& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const;
	};

	template<>
	struct InputData<glm::vec3>
	{
		DrawResult operator()(const char* label, glm::vec3& data) const;
		DrawResult operator()(const char* label, glm::vec3& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const;
	};

	template<>
	struct InputData<glm::vec4>
	{
		DrawResult operator()(const char* label, glm::vec4& data) const;
		DrawResult operator()(const char* label, glm::vec4& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const;
	};

	template<typename T>
	struct InputData<OptionalPrimitive<T>>
	{
		DrawResult operator()(const char* label, OptionalPrimitive<T>& data, OptionalPrimitive<T> min, OptionalPrimitive<T> max) const
		{
			DrawResult result;
			IDScope scope(&data);
			result |= gui::InputData<bool>{}("", data.has_value);
			if (auto disabled = DisabledSection(!data.has_value))
			{
				ImGui::SameLine();
				scope.Push(1);
				result |= gui::InputData<T>{}(label, data.value, min, max);
			}
			return result;
		}
	};

	template<>
	struct InputData<std::string>
	{
		DrawResult operator()(const char* label, std::string& data) const;
	};

	template<>
	struct InputData<Color4>
	{
		DrawResult operator()(const char* label, Color4& data) const;
	};

	template<>
	struct InputData<Rect>
	{
		DrawResult operator()(const char* label, Rect& data) const;
	};

	template<>
	struct InputData<TopSidePadding>
	{
		DrawResult operator()(const char* label, TopSidePadding& data) const;
	};

	template<>
	struct InputData<unsigned int>
	{
		DrawResult operator()(unsigned int& data, const unsigned int* values, const char** names, const size_t count);
		DrawResult operator()(unsigned int& data, const unsigned int* values, const char** names, const bool* disabled, const size_t count);
	};

	template<Enum E>
	struct InputData<E>
	{
		DrawResult operator()(E& data, const E* values, const char** names, const size_t count)
		{
			return (*this)(data, values, names, nullptr, count);
		}

		DrawResult operator()(E& data, const E* values, const char** names, const bool* disabled, const size_t count)
		{
			DrawResult result;
			for (size_t i = 0; i < count; ++i)
			{
				bool flag = static_cast<bool>(data & values[i]);

				if (auto d = DisabledSection(disabled && disabled[i]))
					result |= InputData<bool>{}(names[i], flag);

				if (flag)
					data |= values[i];
				else
					data &= ~values[i];

				if (i + 1 < count)
					ImGui::SameLine();
			}
			return result;
		}
	};
}
