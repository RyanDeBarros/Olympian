#pragma once

#include "core/Types.h"
#include "desc/OptionalPrimitive.h"

#include "gui/DisabledSection.h"
#include "gui/IDScope.h"

#include "external/GLM.h"

#include <string>
#include <vector>

#include <imgui.h>

namespace oly::editor::gui
{
	extern bool Combo(const char* label, int& current_item, std::vector<std::string>& items);

	extern bool InputText(const char* label, std::string& string, size_t max_size = 256,
		ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	template<typename T>
	struct ItemComfortableWidth
	{
		float Get(T num) const
		{
			return ImGui::CalcTextSize(std::to_string(num).c_str()).x;
		}

		void Set(OptionalPrimitive<T> min, OptionalPrimitive<T> max) const
		{
			const float item_width = std::max(ItemComfortableWidth<OptionalPrimitive<T>>{}.Get(min), ItemComfortableWidth<OptionalPrimitive<T>>{}.Get(max));
			ImGui::SetNextItemWidth(item_width);
		}
	};

	template<typename T>
	struct ItemComfortableWidth<OptionalPrimitive<T>>
	{
		float Get(OptionalPrimitive<T> num) const
		{
			if (num.has_value)
				return ImGui::CalcTextSize(std::to_string(num.value).c_str()).x;
			else
				return ImGui::CalcTextSize(std::to_string(-FLT_MAX).c_str()).x;
		}
	};

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
	bool InputClampedData(const char* label, T& data, OptionalPrimitive<U> min, OptionalPrimitive<U> max)
	{
		ItemComfortableWidth<U>{}.Set(min, max);
		const auto og = data;
		if (InputData<T>{}(label, data))
			return Clamp(data, og, min, max);
		else
			return false;
	}

	template<>
	struct InputData<bool>
	{
		bool operator()(const char* label, bool& data) const
		{
			return ImGui::Checkbox(label, &data);
		}
	};

	template<>
	struct InputData<int>
	{
		bool operator()(const char* label, int& data) const
		{
			return ImGui::InputInt(label, &data);
		}

		bool operator()(const char* label, int& data, OptionalPrimitive<int> min, OptionalPrimitive<int> max) const
		{
			return InputClampedData(label, data, min, max);
		}

		bool operator()(const char* label, int& data, const char** names, size_t count)
		{
			return ImGui::Combo(label, &data, names, count);
		}
	};

	template<>
	struct InputData<float>
	{
		bool operator()(const char* label, float& data) const
		{
			return ImGui::InputFloat(label, &data);
		}

		bool operator()(const char* label, float& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
		{
			return InputClampedData(label, data, min, max);
		}
	};

	template<>
	struct InputData<double>
	{
		bool operator()(const char* label, double& data) const
		{
			return ImGui::InputDouble(label, &data);
		}

		bool operator()(const char* label, double& data, OptionalPrimitive<double> min, OptionalPrimitive<double> max) const
		{
			return InputClampedData(label, data, min, max);
		}
	};

	template<>
	struct InputData<glm::vec2>
	{
		bool operator()(const char* label, glm::vec2& data) const
		{
			return ImGui::InputFloat2(label, glm::value_ptr(data));
		}

		bool operator()(const char* label, glm::vec2& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
		{
			return InputClampedData(label, data, min, max);
		}
	};

	template<>
	struct InputData<glm::vec3>
	{
		bool operator()(const char* label, glm::vec3& data) const
		{
			return ImGui::InputFloat3(label, glm::value_ptr(data));
		}

		bool operator()(const char* label, glm::vec3& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
		{
			return InputClampedData(label, data, min, max);
		}
	};

	template<>
	struct InputData<glm::vec4>
	{
		bool operator()(const char* label, glm::vec4& data) const
		{
			return ImGui::InputFloat4(label, glm::value_ptr(data));
		}

		bool operator()(const char* label, glm::vec4& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
		{
			return InputClampedData(label, data, min, max);
		}
	};

	template<typename T>
	struct InputData<OptionalPrimitive<T>>
	{
		bool operator()(const char* label, OptionalPrimitive<T>& data, OptionalPrimitive<T> min, OptionalPrimitive<T> max) const
		{
			bool dirty = false;
			IDScope key_scope;
			key_scope.Push(&key_scope); // TODO v8 remove
			dirty |= gui::InputData<bool>{}("", data.has_value);
			if (auto disabled = DisabledSection(!data.has_value))
			{
				ImGui::SameLine();
				IDScope value_scope;
				value_scope.Push(&value_scope); // TODO v8 remove
				dirty |= gui::InputData<T>{}(label, data.value, min, max);
			}
			return dirty;
		}
	};

	template<>
	struct InputData<std::string>
	{
		bool operator()(const char* label, std::string& data) const
		{
			return InputText(label, data);
		}
	};

	template<>
	struct InputData<Color>
	{
		bool operator()(const char* label, Color& data) const
		{
			return ImGui::ColorEdit4(label, data.ValuePtr());
		}
	};
}
