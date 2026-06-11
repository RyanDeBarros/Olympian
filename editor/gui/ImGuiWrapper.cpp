#include "ImGuiWrapper.h"

namespace oly::editor::gui
{
	static const char* StringVectorComboGetter(void* data, int idx)
	{
		auto& items = *static_cast<std::vector<std::string>*>(data);
		if (idx < 0 || idx >= items.size())
			return nullptr;
		else
			return items[idx].c_str();
	}

	bool Combo(const char* label, int& current_item, std::vector<std::string>& items)
	{
		return ImGui::Combo(label, &current_item, &StringVectorComboGetter, &items, static_cast<int>(items.size()));
	}

	bool InputText(const char* label, std::string& string, size_t max_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		std::string buf;
		buf.resize(max_size);
		string.copy(buf.data(), buf.size());
		bool changed = ImGui::InputText(label, buf.data(), buf.size(), flags, callback, user_data);
		string = std::move(buf);
		size_t n = string.find('\0');
		if (n != std::string::npos)
			string.resize(n);
		return changed;
	}

	bool InputData<bool>::operator()(const char* label, bool& data) const
	{
		return ImGui::Checkbox(label, &data);
	}

	bool InputData<int>::operator()(const char* label, int& data) const
	{
		return ImGui::InputInt(label, &data);
	}

	bool InputData<int>::operator()(const char* label, int& data, OptionalPrimitive<int> min, OptionalPrimitive<int> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	bool InputData<int>::operator()(const char* label, int& data, const char** names, size_t count)
	{
		return ImGui::Combo(label, &data, names, count);
	}

	bool InputData<float>::operator()(const char* label, float& data) const
	{
		return ImGui::InputFloat(label, &data);
	}

	bool InputData<float>::operator()(const char* label, float& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	bool InputData<double>::operator()(const char* label, double& data) const
	{
		return ImGui::InputDouble(label, &data);
	}

	bool InputData<double>::operator()(const char* label, double& data, OptionalPrimitive<double> min, OptionalPrimitive<double> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	bool InputData<glm::vec2>::operator()(const char* label, glm::vec2& data) const
	{
		return ImGui::InputFloat2(label, glm::value_ptr(data));
	}

	bool InputData<glm::vec2>::operator()(const char* label, glm::vec2& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	bool InputData<glm::vec3>::operator()(const char* label, glm::vec3& data) const
	{
		return ImGui::InputFloat3(label, glm::value_ptr(data));
	}

	bool InputData<glm::vec3>::operator()(const char* label, glm::vec3& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	bool InputData<glm::vec4>::operator()(const char* label, glm::vec4& data) const
	{
		return ImGui::InputFloat4(label, glm::value_ptr(data));
	}

	bool InputData<glm::vec4>::operator()(const char* label, glm::vec4& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	bool InputData<std::string>::operator()(const char* label, std::string& data) const
	{
		return InputText(label, data);
	}

	bool InputData<Color>::operator()(const char* label, Color& data) const
	{
		return ImGui::ColorEdit4(label, data.ValuePtr());
	}

	bool InputData<unsigned int>::operator()(unsigned int& data, const unsigned int* values, const char** names, const size_t count)
	{
		bool dirty = false;
		for (size_t i = 0; i < count; ++i)
		{
			bool flag = data & values[i];
			dirty |= InputData<bool>{}(names[i], flag);
			if (flag)
				data |= values[i];
			else
				data &= ~values[i];

			if (i + 1 < count)
				ImGui::SameLine();
		}
		return dirty;
	}
}
