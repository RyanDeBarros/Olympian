#pragma once

#include "gui/DynamicList.h"
#include "gui/EditSession.h"
#include "gui/ImGuiWrapper.h"
#include "gui/WidgetComponentCommon.h"

#include "gui/properties/PropertyGrid.h"
#include "gui/properties/PropertyGroup.h"
#include "gui/properties/PropertyViews.h"

#include "external/GL.h"
#include "external/GLM.h"

#include "desc/FieldSetAction.h"
#include "desc/DynamicListUndoActions.h"

#include <string>

namespace oly::editor
{
	struct DescIO
	{
		template<typename T>
		struct ValueInputData
		{
			template<typename... Args>
			void operator()(const char* label, T& data, Args&&... args) const
			{
				gui::PropertyGrid::Value::AddComponent(comp::InputData<T>(label, data, std::forward<Args>(args)...));
			}
		};

		template<typename T>
		struct ValueInputData<OptionalPrimitive<T>>
		{
			template<typename... Args>
			void operator()(const char* label, OptionalPrimitive<T>& data, Args&&... args) const
			{
				gui::PropertyGrid::Value::AddComponent(comp::InputData<bool>("##", data.has_value));
				gui::PropertyGrid::Value::AddComponent(comp::Generic([label, &data, ... args = std::forward<Args>(args)]() mutable -> DrawResult {
					gui::IDScope scope(&data.value);
					if (auto disabled = DisabledSection(!data.has_value))
						return gui::InputData<T>{}(label, data.value, std::forward<Args>(args)...);
					else
						return {};
				}));
			}
		};

		template<typename T, typename... Args>
		static void RowInputData(const char* label, T& data, const T& def, Args&&... args)
		{
			gui::IDScope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);
			ValueInputData<T>{}("##", data, std::forward<Args>(args)...);
			if (data != def)
				gui::PropertyGrid::Reset::Button();
			gui::PropertyGrid::SubmitRow();
			if (gui::PropertyGrid::Reset::AnyActivated())
				data = def;
		}

		template<typename T, typename... Args>
		static void RowInputData(const char* label, EditSession<T>& data, const T& def, Args&&... args)
		{
			gui::IDScope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);

			data.PreEdit();
			if (data.buffer != def)
				gui::PropertyGrid::Reset::Button();

			ValueInputData<T>{}("##", data.buffer, std::forward<Args>(args)...);

			gui::PropertyGrid::SubmitRow();
			data.PostEdit(gui::PropertyGrid::Value::GetDrawResult());
			if (gui::PropertyGrid::Reset::AnyActivated())
				data.PublishReset(def);
		}

		template<typename T, typename U = T>
		static void Draw(const char* label, T& data, const T& def, OptionalPrimitive<U> min, OptionalPrimitive<U> max)
		{
			RowInputData(label, data, def, min, max);
		}

		template<typename T, typename U = T>
		static void Draw(const char* label, EditSession<T>& data, const T& def, OptionalPrimitive<U> min, OptionalPrimitive<U> max)
		{
			RowInputData(label, data, def, min, max);
		}

		template<typename T>
		static void Draw(const char* label, T& data, const T& def)
		{
			RowInputData(label, data, def);
		}

		template<typename T>
		static void Draw(const char* label, EditSession<T>& data, const T& def)
		{
			RowInputData(label, data, def);
		}

		static void Draw(const char* label, int& data, const int& def, LabelSpanRegistry::Handle names);
		static void Draw(const char* label, EditSession<std::string>* data, const std::string* def, size_t count);
		static void Draw(const char* label, EditSession<std::string>* data, const std::string* def, const char** sublabels, size_t count);
		static void Draw(const char* label, bool* data, const bool* def, const char** sublabels, size_t count);
		static void Draw(const char* label, bool* data, const bool* def, const char** sublabels, const bool* disabled, size_t count);

		static void Draw(const char* label, EditSession<Rect>& data, const Rect& def);
		static void Draw(const char* label, EditSession<UVRect>& data, const UVRect& def);
		static void Draw(const char* label, EditSession<TopSidePadding>& data, const TopSidePadding& def);

		template<typename T, typename Printer = StandardPrinter<T>>
		static DrawResult ValueDrawDynamicList(DataPath path, std::vector<T>& data, const std::function<DrawResult(gui::DynamicRow&)>& draw_fn, gui::DynamicListState& ui_state)
		{
			DrawResult result;

			ui_state.DrawListHeader(data.size());

			ui_state.DrawBody([&result, &draw_fn](gui::DynamicRow& row) {
				auto row_result = draw_fn(row);
				result |= row_result;
				if (row_result.IsLeftClicked() || row_result.IsFocused())
					row.OnSelect();
			});

			result |= ui_state.VisitRowOps([path, &data](const gui::RowOperation& op) {
				switch (op.type)
				{
				case gui::RowOperation::Type::Delete:
				{
					ExecuteDynamicListDeleteAction<T, Printer>(path, op.GetIndex());
					break;
				}

				case gui::RowOperation::Type::Move:
				{
					if (op.GetSrcIndex() != op.GetDstIndex())
						ExecuteDynamicListMoveAction<T>(path, op.GetSrcIndex(), op.GetDstIndex());
					break;
				}

				case gui::RowOperation::Type::Resize:
				{
					if (data.size() != op.GetSize())
						ExecuteDynamicListResizeAction<T>(path, data.size(), op.GetSize());
					break;
				}

				case gui::RowOperation::Type::PushBack:
				{
					ExecuteDynamicListInsertAction<T, Printer>(path, data.size());
					break;
				}
				}
			});
			
			return result;
		}

		template<typename T, typename Printer = StandardPrinter<T>>
		static DrawResult ValueDrawDynamicList(DataPath path, EditSession<std::vector<T>>& data, const std::function<DrawResult(gui::DynamicRow&)>& draw_fn, gui::DynamicListState& ui_state)
		{
			DrawResult result;

			ui_state.DrawListHeader(data.buffer.size());

			ui_state.DrawBody([&result, &draw_fn](gui::DynamicRow& row) {
				auto row_result = draw_fn(row);
				result |= row_result;
				if (row_result.IsLeftClicked() || row_result.IsFocused())
					row.OnSelect();
			});

			result |= ui_state.VisitRowOps([path, &data](const gui::RowOperation& op) {
				switch (op.type)
				{
				case gui::RowOperation::Type::Delete:
				{
					data.CancelEditing();
					ExecuteDynamicListDeleteAction<T, Printer>(path, op.GetIndex());
					break;
				}

				case gui::RowOperation::Type::Move:
				{
					data.CancelEditing();
					if (op.GetSrcIndex() != op.GetDstIndex())
						ExecuteDynamicListMoveAction<T>(path, op.GetSrcIndex(), op.GetDstIndex());
					break;
				}

				case gui::RowOperation::Type::Resize:
				{
					data.CancelEditing();
					if (data.truth.size() != op.GetSize())
						ExecuteDynamicListResizeAction<T>(path, data.truth.size(), op.GetSize());
					break;
				}

				case gui::RowOperation::Type::PushBack:
				{
					data.CancelEditing();
					ExecuteDynamicListInsertAction<T, Printer>(path, data.truth.size());
					break;
				}
				}
			});

			return result;
		}

		template<typename T, typename Printer = StandardPrinter<T>>
		static void DrawDynamicList(DataPath path, const char* label, std::vector<T>& data, const std::vector<T>& def, std::function<DrawResult(gui::DynamicRow&)> draw_fn, gui::DynamicListState& ui_state)
		{
			gui::IDScope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);
			if (data.size() != def.size())
				gui::PropertyGrid::Reset::Button(0);

			gui::PropertyGrid::Value::AddComponent(comp::Generic([path, &data, &ui_state, draw_fn = std::move(draw_fn)]() -> DrawResult { return ValueDrawDynamicList<T, Printer>(path, data, draw_fn, ui_state); }));

			gui::PropertyGrid::SubmitRow();
			if (gui::PropertyGrid::Reset::Activated(0))
				ui_state.DeferResize(def.size());
		}

		template<typename T, typename Printer = StandardPrinter<T>>
		static void DrawDynamicList(DataPath path, const char* label, EditSession<std::vector<T>>& data, const std::vector<T>& def, std::function<DrawResult(gui::DynamicRow&)> draw_fn, gui::DynamicListState& ui_state)
		{
			gui::IDScope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);
			if (data.buffer.size() != def.size())
				gui::PropertyGrid::Reset::Button(0);

			gui::PropertyGrid::Value::AddComponent(comp::Generic([path, &data, &ui_state, draw_fn = std::move(draw_fn)]() -> DrawResult { return ValueDrawDynamicList<T, Printer>(path, data, draw_fn, ui_state); }));

			gui::PropertyGrid::SubmitRow();
			data.PostEdit(gui::PropertyGrid::Value::GetDrawResult());
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
		static void DrawDynamicListRevertButtons(const EditSession<std::vector<T>>& data, const std::vector<T>& def)
		{
			for (size_t i = 0; i < data.buffer.size(); ++i)
			{
				if (i < def.size())
				{
					if (data.buffer[i] != def[i])
						gui::PropertyGrid::Reset::Button(1 + i);
				}
				else
				{
					if (data.buffer[i] != T{})
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

		template<typename T>
		static void CheckDynamicListRevertButtons(EditSession<std::vector<T>>& data, const std::vector<T>& def)
		{
			for (size_t i = 0; i < data.buffer.size(); ++i)
			{
				if (gui::PropertyGrid::Reset::Activated(1 + i))
				{
					T def_value = i < def.size() ? def[i] : T{};
					data.buffer[i] = def_value;
					data.truth[i] = def_value;
					data.published = true;
				}
			}
		}

		template<typename T> requires (!std::is_enum_v<T>)
		static void Draw(DataPath path, const char* label, std::vector<T>& data, const std::vector<T>& def, gui::DynamicListState& ui_state)
		{
			DrawDynamicListRevertButtons(data, def);

			DrawDynamicList(path, label, data, def, [&data, &def](gui::DynamicRow& row) {
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
		static void Draw(DataPath path, const char* label, std::vector<E>& data, const std::vector<E>& def, gui::DynamicListState& ui_state)
		{
			DrawDynamicListRevertButtons(data, def);

			DrawDynamicList(path, label, data, def, [&data, &def](gui::DynamicRow& row) {
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
			int index = static_cast<int>(data);
			LabelSpanRegistry::Handle span = LabelSpanRegistry::Intern(std::span<const char* const>(values, N));
			DrawResult result = gui::InputData<int>{}("##", index, span);
			data = static_cast<E>(index);
			return result;
		}
	};
}
