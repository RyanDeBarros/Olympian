#pragma once

#include "desc/OptionalPrimitive.h"

#include "gui/scopes/IDScope.h"
#include "gui/DynamicList.h"
#include "gui/ImGuiWrapper.h"
#include "gui/PropertyGrid.h"
#include "gui/WidgetComponentCommon.h"

#include "external/GL.h"
#include "external/GLM.h"

#include <string>

namespace oly::editor
{
	struct DescIO
	{
		template<typename T, typename... Args>
		static void ValueInputData(const char* label, T& data, Args&&... args)
		{
			gui::PropertyGrid::Value::AddComponent(comp::InputData(label, data, std::forward<Args>(args)...));
		}

	private:
		template<typename T, typename... Args>
		static void RowInputData(const char* label, T& data, const T& def, Args&&... args)
		{
			gui::IDScope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);
			ValueInputData<T>("##", data, std::forward<Args>(args)...);
			if (data != def)
				gui::PropertyGrid::Reset::Button();
			gui::PropertyGrid::SubmitRow();
			if (gui::PropertyGrid::Reset::AnyActivated())
				data = def;
		}

	public:
		template<typename T, typename U = T>
		static void Draw(const char* label, T& data, const T& def, OptionalPrimitive<U> min, OptionalPrimitive<U> max)
		{
			RowInputData(label, data, def, min, max);
		}

		template<typename T>
		static void Draw(const char* label, T& data, const T& def)
		{
			RowInputData(label, data, def);
		}

		static void Draw(const char* label, int& data, const int& def, const char** names, size_t count);
		static void Draw(const char* label, std::string* data, const std::string* def, size_t count);
		static void Draw(const char* label, bool* data, const bool* def, const char** sublabels, size_t count);
		static void Draw(const char* label, bool* data, const bool* def, const char** sublabels, const bool* disabled, size_t count);

		static void Draw(const char* label, Rect& data, const Rect& def);
		static void Draw(const char* label, UVRect& data, const UVRect& def);
		static void Draw(const char* label, TopSidePadding& data, const TopSidePadding& def);

		template<typename T>
		static DrawResult ValueDrawDynamicList(std::vector<T>& data, const std::function<DrawResult(gui::DynamicRow&)>& draw_fn, gui::DynamicListState& ui_state)
		{
			DrawResult result;

			ui_state.DrawListHeader(data.size());

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

		template<typename T>
		static void DrawDynamicList(const char* label, std::vector<T>& data, const std::vector<T>& def, std::function<DrawResult(gui::DynamicRow&)> draw_fn, gui::DynamicListState& ui_state)
		{
			gui::IDScope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);
			if (data.size() != def.size())
				gui::PropertyGrid::Reset::Button(0);

			gui::PropertyGrid::Value::AddComponent(comp::Generic([&data, &ui_state, draw_fn = std::move(draw_fn)]() -> DrawResult { return ValueDrawDynamicList(data, draw_fn, ui_state); }));

			gui::PropertyGrid::SubmitRow();
			if (gui::PropertyGrid::Reset::Activated(0))
				ui_state.DeferResize(def.size());
		}

		template<typename T>
		static void DrawDynamicListRevertButtons(const std::vector<T>& data, const std::vector<T>& def)
		{
			for (size_t i = 0; i < data.size(); ++i)
			{
				if (i < def.size())
				{
					if (data[i] != def[i])
						gui::PropertyGrid::Reset::Button(1 + i);
				}
				else
				{
					if (data[i] != T{})
						gui::PropertyGrid::Reset::Button(1 + i);
				}
			}
		}

		template<typename T>
		static void CheckDynamicListRevertButtons(std::vector<T>& data, const std::vector<T>& def)
		{
			for (size_t i = 0; i < data.size(); ++i)
			{
				if (gui::PropertyGrid::Reset::Activated(1 + i))
				{
					if (i < def.size())
						data[i] = def[i];
					else
						data[i] = T{};
				}
			}
		}

		template<typename T> requires (!std::is_enum_v<T>)
		static void Draw(const char* label, std::vector<T>& data, const std::vector<T>& def, gui::DynamicListState& ui_state)
		{
			DrawDynamicListRevertButtons(data, def);

			DrawDynamicList(label, data, def, [&data, &def](gui::DynamicRow& row) {
				DrawResult result;

				ImGui::SameLine();
				result |= gui::InputData<T>{}("##Item", data[row.Index()]);

				if (ImGui::IsItemActivated())
					row.OnSelect();

				return result;
			}, ui_state);

			CheckDynamicListRevertButtons(data, def);
		}

		template<typename E> requires (std::is_enum_v<E>)
		static void Draw(const char* label, std::vector<E>& data, const std::vector<E>& def, gui::DynamicListState& ui_state)
		{
			DrawDynamicListRevertButtons(data, def);

			DrawDynamicList(label, data, def, [&data, &def](gui::DynamicRow& row) {
				DrawResult result;

				ImGui::SameLine();
				result |= DrawCombo("##Item", data[row.Index()]);

				if (ImGui::IsItemActivated())
					row.OnSelect();

				return result;
			}, ui_state);

			CheckDynamicListRevertButtons(data, def);
		}

		template<Enum E>
		static void Draw(const char* label, E& data, const E& def)
		{
			gui::IDScope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);
			gui::PropertyGrid::Value::AddComponent(comp::Generic([&data]() -> DrawResult { return DrawCombo("##", data); }));
			if (data != def)
				gui::PropertyGrid::Reset::Button();
			gui::PropertyGrid::SubmitRow();
			if (gui::PropertyGrid::Reset::AnyActivated())
				data = def;
		}

		template<Enum E>
		static DrawResult DrawCombo(const char* label, E& data);

	private:
		template<Enum E, size_t N>
		static DrawResult DrawEnumCombo(const char* label, E& data, const char* const (&values)[N])
		{
			DrawResult result;
			int index = static_cast<int>(data);
			result |= ImGui::Combo("##", &index, values, N);
			result.Query();
			data = static_cast<E>(index);
			return result;
		}
	};
}
