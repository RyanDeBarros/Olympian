#pragma once

#include "desc/OptionalPrimitive.h"

#include "gui/DynamicList.h"
#include "gui/IDScope.h"
#include "gui/ImGuiWrapper.h"

#include "external/GL.h"
#include "external/GLM.h"

#include <string>

namespace oly::editor
{
	struct DescIO
	{
		static void PrepareValue(const char* label);
		static bool DrawRevertButton();

		template<typename T>
		static bool CheckRevertButton(T& desc, const T& def)
		{
			if (desc != def && DrawRevertButton())
			{
				desc = def;
				return true;
			}
			else
				return false;
		}

		template<typename T, typename U = T>
		static DrawResult Draw(const char* label, T& data, const T& def, OptionalPrimitive<U> min, OptionalPrimitive<U> max)
		{
			DrawResult result;
			PrepareValue(label);
			gui::IDScope scope(&data);
			result |= gui::InputData<T>{}("", data, min, max);
			result |= CheckRevertButton(data, def);
			return result;
		}

		template<typename T>
		static DrawResult Draw(const char* label, T& data, const T& def)
		{
			DrawResult result;
			PrepareValue(label);
			gui::IDScope scope(&data);
			result |= gui::InputData<T>{}("", data);
			result |= CheckRevertButton(data, def);
			return result;
		}
		
		static DrawResult Draw(const char* label, int& data, const int& def, const char** names, size_t count);
		static DrawResult Draw(const char* label, std::string* data, const std::string* def, size_t count);
		static DrawResult Draw(const char* label, bool* data, const bool* def, const char** sublabels, size_t count);

		template<typename T>
		static DrawResult DrawDynamicList(const char* label, std::vector<T>& data, const std::vector<T>& def, std::function<DrawResult(gui::DynamicRow&)> draw_fn, gui::DynamicListState& ui_state)
		{
			DrawResult result;
			DescIO::PrepareValue(label);
			gui::IDScope scope(&data);

			ui_state.DrawListHeader(data.size());

			if (data.size() != def.size())
			{
				if (result |= DescIO::DrawRevertButton())
					ui_state.DeferResize(def.size());
			}

			ui_state.DrawBody([&result, &draw_fn](gui::DynamicRow& row) {
				auto row_result = draw_fn(row);
				result |= row_result;
				if (row_result.IsLeftClicked() || row_result.IsFocused())
					row.OnSelect();
			});

			result |= ui_state.VisitRowOps([&data](const gui::RowOperation& op) {
				switch (op.type)
				{
				case gui::RowOperation::Type::Delete:
					data.erase(data.begin() + op.index);
					break;

				case gui::RowOperation::Type::Move:
				{
					auto moved(std::move(data[op.src]));
					data.erase(data.begin() + op.src);
					data.insert(data.begin() + op.index, std::move(moved));
					break;
				}

				case gui::RowOperation::Type::Resize:
					data.resize(op.index);
					break;

				case gui::RowOperation::Type::PushBack:
					data.push_back(T{});
					break;
				}
			});

			return result;
		}

		template<typename T> requires (!std::is_enum_v<T>)
		static DrawResult Draw(const char* label, std::vector<T>& data, const std::vector<T>& def, gui::DynamicListState& ui_state)
		{
			return DrawDynamicList(label, data, def, [&data, &def](gui::DynamicRow& row) {
				DrawResult result;

				ImGui::SameLine();
				result |= gui::InputData<T>{}("##Item", data[row.Index()]);

				if (ImGui::IsItemActivated())
					row.OnSelect();

				if (row.Index() < def.size())
					result |= CheckRevertButton(data[row.Index()], def[row.Index()]);
				else
				{
					static const T empty = {};
					result |= CheckRevertButton(data[row.Index()], empty);
				}

				return result;
			}, ui_state);
		}

		template<typename E> requires (std::is_enum_v<E>)
		static DrawResult Draw(const char* label, std::vector<E>& data, const std::vector<E>& def, gui::DynamicListState& ui_state)
		{
			return DrawDynamicList(label, data, def, [&data, &def](gui::DynamicRow& row) {
				DrawResult result;

				ImGui::SameLine();
				result |= DrawCombo("##Item", data[row.Index()]);

				if (ImGui::IsItemActivated())
					row.OnSelect();

				if (row.Index() < def.size())
					result |= CheckRevertButton(data[row.Index()], def[row.Index()]);
				else
				{
					static const E empty = {};
					result |= CheckRevertButton(data[row.Index()], empty);
				}

				return result;
			}, ui_state);
		}

		template<typename E>
		static DrawResult Draw(const char* label, E& data, const E& def, const E* values, const char** names, const bool* disabled, size_t count)
		{
			DrawResult result;
			PrepareValue(label);
			gui::IDScope scope(&data);
			result |= gui::InputData<E>{}(data, values, names, disabled, count);
			result |= CheckRevertButton(data, def);
			return result;
		}

		template<Enum E>
		static bool Draw(const char* label, E& data, const E& def)
		{
			DrawResult result;
			PrepareValue(label);
			gui::IDScope scope(&data);
			result |= DrawCombo("", data);
			result |= CheckRevertButton(data, def);
			return result;
		}

		template<Enum E>
		static DrawResult DrawCombo(const char* label, E& data);

	private:
		template<Enum E, size_t N>
		static DrawResult DrawEnumCombo(const char* label, E& data, const char* const (&values)[N])
		{
			DrawResult result;
			int index = static_cast<int>(data);
			result |= ImGui::Combo("", &index, values, N);
			result.Query();
			data = static_cast<E>(index);
			return result;
		}
	};
}
