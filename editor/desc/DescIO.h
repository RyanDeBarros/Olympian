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
		static bool Draw(const char* label, T& data, const T& def, OptionalPrimitive<U> min, OptionalPrimitive<U> max)
		{
			bool dirty = false;
			PrepareValue(label);
			gui::IDScope scope(&data);
			dirty |= gui::InputData<T>{}("", data, min, max);
			dirty |= CheckRevertButton(data, def);
			return dirty;
		}

		template<typename T>
		static bool Draw(const char* label, T& data, const T& def)
		{
			bool dirty = false;
			PrepareValue(label);
			gui::IDScope scope(&data);
			dirty |= gui::InputData<T>{}("", data);
			dirty |= CheckRevertButton(data, def);
			return dirty;
		}
		
		static bool Draw(const char* label, int& data, const int& def, const char** names, size_t count);
		static bool Draw(const char* label, std::string* data, const std::string* def, size_t count);
		static bool Draw(const char* label, bool* data, const bool* def, const char** sublabels, size_t count);

		template<typename T>
		static bool DrawDynamicList(const char* label, std::vector<T>& data, const std::vector<T>& def, std::function<bool(gui::DynamicRow&)> draw_fn, gui::DynamicListState& ui_state)
		{
			bool dirty = false;
			DescIO::PrepareValue(label);
			gui::IDScope scope(&data);

			ui_state.DrawListHeader(data.size());

			if (data.size() != def.size())
			{
				if (DescIO::DrawRevertButton())
					ui_state.DeferResize(def.size());
			}

			ui_state.DrawBody([&dirty, &draw_fn](gui::DynamicRow& row) { dirty |= draw_fn(row); });

			dirty |= ui_state.VisitRowOps([&data](const gui::RowOperation& op) {
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

			return dirty;
		}

		template<typename T> requires (!std::is_enum_v<T>)
		static bool Draw(const char* label, std::vector<T>& data, const std::vector<T>& def, gui::DynamicListState& ui_state)
		{
			return DrawDynamicList(label, data, def, [&data, &def](gui::DynamicRow& row) {
				bool dirty = false;

				ImGui::SameLine();
				dirty |= gui::InputData<T>{}("##Item", data[row.Index()]);

				if (ImGui::IsItemActivated())
					row.OnSelect();

				if (row.Index() < def.size())
					dirty |= CheckRevertButton(data[row.Index()], def[row.Index()]);
				else
				{
					static const T empty = {};
					dirty |= CheckRevertButton(data[row.Index()], empty);
				}

				return dirty;
			}, ui_state);
		}

		template<typename E> requires (std::is_enum_v<E>)
		static bool Draw(const char* label, std::vector<E>& data, const std::vector<E>& def, gui::DynamicListState& ui_state)
		{
			return DrawDynamicList(label, data, def, [&data, &def](gui::DynamicRow& row) {
				bool dirty = false;

				ImGui::SameLine();
				dirty |= DrawCombo("##Item", data[row.Index()]);

				if (ImGui::IsItemActivated())
					row.OnSelect();

				if (row.Index() < def.size())
					dirty |= CheckRevertButton(data[row.Index()], def[row.Index()]);
				else
				{
					static const E empty = {};
					dirty |= CheckRevertButton(data[row.Index()], empty);
				}

				return dirty;
			}, ui_state);
		}

		static bool Draw(const char* label, unsigned int& data, const unsigned int& def, const unsigned int* values, const char** names, const bool* disabled, size_t count);

		template<typename E> requires (std::is_enum_v<E>)
		static bool Draw(const char* label, E& data, const E& def)
		{
			bool dirty = false;
			PrepareValue(label);
			gui::IDScope scope(&data);
			dirty |= DrawCombo("", data);
			dirty |= CheckRevertButton(data, def);
			return dirty;
		}

		template<typename E> requires (std::is_enum_v<E>)
		static bool DrawCombo(const char* label, E& data);

	private:
		template<typename E, size_t N>
		static bool DrawEnumCombo(const char* label, E& data, const char* const (&values)[N])
		{
			bool dirty = false;
			int index = static_cast<int>(data);
			dirty |= ImGui::Combo("", &index, values, N);
			data = static_cast<E>(index);
			return dirty;
		}
	};
}
