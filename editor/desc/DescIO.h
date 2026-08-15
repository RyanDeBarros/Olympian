#pragma once

#include "gui/DynamicList.h"
#include "gui/ImGuiWrapper.h"
#include "gui/WidgetComponentCommon.h"

#include "gui/properties/PropertyGrid.h"

#include "desc/FieldSetAction.h"
#include "desc/DynamicListUndoActions.h"

#include <imtk.hpp>

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
		struct ValueLabelInputData
		{
			template<typename... Args>
			void operator()(const char* label, const char* data_label, T& data, Args&&... args) const
			{
				gui::PropertyGrid::Value::AddComponent(comp::LabelInputData<T>(label, data_label, data, std::forward<Args>(args)...));
			}
		};

		template<typename T>
		struct ValueLabelInputDataSep
		{
			template<typename... Args>
			void operator()(const char* label, const char* data_label, T& data, Args&&... args) const
			{
				gui::PropertyGrid::Value::AddComponent(comp::LabelInputDataSep<T>(label, data_label, data, std::forward<Args>(args)...));
			}
		};

		template<typename T>
		struct ValueInputData<imp::potential<T>>
		{
			template<typename... Args>
			void operator()(const char* label, imp::potential<T>& data, Args&&... args) const
			{
				gui::PropertyGrid::Value::AddComponent(std::make_unique<imtk::w::generic_widget>([label, &data, ... args = std::forward<Args>(args)]() mutable -> imtk::item_result {
					imtk::item_result result = gui::InputData<bool>{}("##Checkbox", data.has_value);;

					imtk::id_scope scope(&data.value);
					if (auto d = imtk::disabled(!data.has_value))
					{
						ImGui::SameLine();
						result |= gui::InputData<T>{}(label, data.value, std::forward<Args>(args)...);
					}

					return result;
				}));
			}
		};

		template<typename T, typename... Args>
		static void RowInputData(const char* label, T& data, const T& def, Args&&... args)
		{
			imtk::id_scope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);
			ValueInputData<T>{}("##", data, std::forward<Args>(args)...);
			if (data != def)
				gui::PropertyGrid::Reset::Button();
			gui::PropertyGrid::SubmitRow();
			if (gui::PropertyGrid::Reset::AnyActivated())
				data = def;
		}

		template<typename T, typename... Args>
		static void RowInputData(const char* label, imtk::edit_session<T>& data, const T& def, Args&&... args)
		{
			imtk::id_scope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);

			data.pre_edit();
			if (data.buffer() != def)
				gui::PropertyGrid::Reset::Button();

			ValueInputData<T>{}("##", data.buffer(), std::forward<Args>(args)...);

			gui::PropertyGrid::SubmitRow();
			data.post_edit(gui::PropertyGrid::Value::GetDrawResult().state);
			if (gui::PropertyGrid::Reset::AnyActivated())
				data.publish_reset(def);
		}

		template<typename T, typename U = T>
		static void Draw(const char* label, T& data, const T& def, imp::potential<U> min, imp::potential<U> max)
		{
			RowInputData(label, data, def, min, max);
		}

		template<typename T, typename U = T>
		static void Draw(const char* label, imtk::edit_session<T>& data, const T& def, imp::potential<U> min, imp::potential<U> max)
		{
			RowInputData(label, data, def, min, max);
		}

		template<typename T>
		static void Draw(const char* label, T& data, const T& def)
		{
			RowInputData(label, data, def);
		}

		template<typename T>
		static void Draw(const char* label, imtk::edit_session<T>& data, const T& def)
		{
			RowInputData(label, data, def);
		}

		static void Draw(const char* label, int& data, const int& def, imtk::label_span_registry::handle names);
		static void Draw(const char* label, imtk::edit_session<std::string>* data, const std::string* def, size_t count);
		static void Draw(const char* label, imtk::edit_session<std::string>* data, const std::string* def, const char** sublabels, size_t count);
		static void Draw(const char* label, bool* data, const bool* def, const char** sublabels, size_t count, bool inline_checkboxes);
		static void Draw(const char* label, bool* data, const bool* def, const char** sublabels, const bool* disabled, size_t count, bool inline_checkboxes);

		static void Draw(const char* label, imtk::edit_session<Rect>& data, const Rect& def);
		static void Draw(const char* label, imtk::edit_session<UVRect>& data, const UVRect& def);
		static void Draw(const char* label, imtk::edit_session<TopSidePadding>& data, const TopSidePadding& def);

		template<Enum E>
		static void Draw(const char* label, E& data, const E& def)
		{
			imtk::id_scope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);
			gui::PropertyGrid::Value::AddComponent(std::make_unique<imtk::w::generic_widget>([&data]() -> imtk::item_result { return DrawCombo("##", data); }));
			if (data != def)
				gui::PropertyGrid::Reset::Button();
			gui::PropertyGrid::SubmitRow();
			if (gui::PropertyGrid::Reset::AnyActivated())
				data = def;
		}

		template<Enum E>
		static imtk::item_result DrawCombo(const char* label, E& data);

	private:
		template<Enum E, size_t N>
		static imtk::item_result DrawEnumCombo(const char* label, E& data, const char* const (&values)[N])
		{
			int index = static_cast<int>(data);
			auto span = imtk::label_span_registry::intern(std::span<const char* const>(values, N));
			imtk::item_result result = gui::InputData<int>{}("##", index, span);
			data = static_cast<E>(index);
			return result;
		}

	public:
		template<typename T, typename Printer = StandardPrinter<T>>
		static imtk::item_result ValueDrawDynamicList(const imtk::datapath_link& link, const imtk::desc::vector<T>& data,
			const std::function<imtk::item_result(gui::DynamicRow&)>& draw_fn, gui::DynamicListState& ui_state)
		{
			imtk::item_result result;

			ui_state.DrawListHeader(data.size());

			ui_state.DrawBody([&result, &draw_fn](gui::DynamicRow& row) {
				ImGui::SameLine();
				auto row_result = draw_fn(row);
				result |= row_result;
				if (row_result.state.left_clicked() || row_result.state.focused())
					row.OnSelect();
				});

			result.modified |= ui_state.VisitRowOps([&link, &data](const gui::RowOperation& op) {
				switch (op.type)
				{
				case gui::RowOperation::Type::Delete:
					ExecuteDynamicVectorDescDeleteAction<T, Printer>(link.compute_path(), op.GetIndex());
					break;

				case gui::RowOperation::Type::Move:
					if (op.GetSrcIndex() != op.GetDstIndex())
						ExecuteDynamicVectorDescMoveAction<T>(link.compute_path(), op.GetSrcIndex(), op.GetDstIndex());
					break;

				case gui::RowOperation::Type::Resize:
					if (data.size() != op.GetSize())
						ExecuteDynamicVectorDescResizeAction<T>(link.compute_path(), data.size(), op.GetSize());
					break;

				case gui::RowOperation::Type::PushBack:
					ExecuteDynamicVectorDescInsertAction<T, Printer>(link.compute_path(), data.size());
					break;
				}
			});

			return result;
		}

		template<typename T, typename Printer = StandardPrinter<T>>
		static imtk::item_result ValueDrawDynamicList(const imtk::datapath_link& link, imtk::edit_session<std::vector<T>>& data,
			const std::function<imtk::item_result(gui::DynamicRow&)>& draw_fn, gui::DynamicListState& ui_state)
		{
			imtk::item_result result;

			ui_state.DrawListHeader(data.buffer().size());

			ui_state.DrawBody([&result, &draw_fn](gui::DynamicRow& row) {
				ImGui::SameLine();
				auto row_result = draw_fn(row);
				result |= row_result;
				if (row_result.state.left_clicked() || row_result.state.focused())
					row.OnSelect();
				});

			result.modified |= ui_state.VisitRowOps([&link, &data](const gui::RowOperation& op) {
				switch (op.type)
				{
				case gui::RowOperation::Type::Delete:
					data.cancel_editing();
					ExecuteDynamicListDeleteAction<T, Printer>(link.compute_path(), op.GetIndex());
					break;

				case gui::RowOperation::Type::Move:
					data.cancel_editing();
					if (op.GetSrcIndex() != op.GetDstIndex())
						ExecuteDynamicListMoveAction<T>(link.compute_path(), op.GetSrcIndex(), op.GetDstIndex());
					break;

				case gui::RowOperation::Type::Resize:
					data.cancel_editing();
					if (data.truth().size() != op.GetSize())
						ExecuteDynamicListResizeAction<T>(link.compute_path(), data.truth().size(), op.GetSize());
					break;

				case gui::RowOperation::Type::PushBack:
					data.cancel_editing();
					ExecuteDynamicListInsertAction<T, Printer>(link.compute_path(), data.truth().size());
					break;
				}
			});

			return result;
		}

		template<typename T, typename Printer = StandardPrinter<T>>
		static void DrawDynamicList(const imtk::datapath_link& link, const char* label, const imtk::desc::vector<T>& data, const std::vector<T>& def,
			std::function<imtk::item_result(gui::DynamicRow&)> draw_fn, gui::DynamicListState& ui_state)
		{
			imtk::id_scope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);
			if (data.size() != def.size())
				gui::PropertyGrid::Reset::Button(0);

			gui::PropertyGrid::Value::AddComponent(std::make_unique<imtk::w::generic_widget>([&link, &data, &ui_state, draw_fn = std::move(draw_fn)]() -> imtk::item_result
				{ return ValueDrawDynamicList<T, Printer>(link, data, draw_fn, ui_state); }));

			gui::PropertyGrid::SubmitRow();
			if (gui::PropertyGrid::Reset::Activated(0))
				ui_state.DeferResize(def.size());
		}

		template<typename T, typename Printer = StandardPrinter<T>>
		static void DrawDynamicList(const imtk::datapath_link& link, const char* label, imtk::edit_session<std::vector<T>>& data, const std::vector<T>& def,
			std::function<imtk::item_result(gui::DynamicRow&)> draw_fn, gui::DynamicListState& ui_state)
		{
			imtk::id_scope scope(&data);
			gui::PropertyGrid::Key::SetLabel(label);
			if (data.buffer().size() != def.size())
				gui::PropertyGrid::Reset::Button(0);

			gui::PropertyGrid::Value::AddComponent(std::make_unique<imtk::w::generic_widget>([&link, &data, &ui_state, draw_fn = std::move(draw_fn)]() -> imtk::item_result
				{ return ValueDrawDynamicList<T, Printer>(link, data, draw_fn, ui_state); }));

			gui::PropertyGrid::SubmitRow();
			data.post_edit(gui::PropertyGrid::Value::GetDrawResult().state);
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
		static void DrawDynamicListRevertButtons(const imtk::edit_session<std::vector<T>>& data, const std::vector<T>& def)
		{
			for (size_t i = 0; i < data.buffer().size(); ++i)
			{
				if (i < def.size())
				{
					if (data.buffer()[i] != def[i])
						gui::PropertyGrid::Reset::Button(1 + i);
				}
				else
				{
					if (data.buffer()[i] != T{})
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
		static void CheckDynamicListRevertButtons(imtk::edit_session<std::vector<T>>& data, const std::vector<T>& def)
		{
			std::vector<T> reset = data.buffer();
			bool publish = false;

			for (size_t i = 0; i < data.buffer().size(); ++i)
			{
				if (gui::PropertyGrid::Reset::Activated(1 + i))
				{
					reset[i] = i < def.size() ? def[i] : T{};
					publish = true;
				}
			}

			if (publish)
				data.publish_reset(std::move(reset));
		}

		template<typename T> requires (!std::is_enum_v<T>)
		static void Draw(const imtk::datapath_link& link, const char* label, std::vector<T>& data, const std::vector<T>& def, gui::DynamicListState& ui_state)
		{
			DrawDynamicListRevertButtons(data, def);

			DrawDynamicList(std::move(link), label, data, def, [&data, &def](gui::DynamicRow& row) {
				imtk::item_result result;

				ImGui::SameLine();
				result |= gui::InputData<T>{}("##Item", data[row.Index()]);

				if (ImGui::IsItemActivated())
					row.OnSelect();

				return result;
				}, ui_state);

			CheckDynamicListRevertButtons(data, def);
		}

		template<typename E> requires (std::is_enum_v<E>)
		static void Draw(const imtk::datapath_link& link, const char* label, std::vector<E>& data, const std::vector<E>& def, gui::DynamicListState& ui_state)
		{
			DrawDynamicListRevertButtons(data, def);

			DrawDynamicList(std::move(link), label, data, def, [&data, &def](gui::DynamicRow& row) {
				imtk::item_result result;

				ImGui::SameLine();
				result |= DrawCombo("##Item", data[row.Index()]);

				if (ImGui::IsItemActivated())
					row.OnSelect();

				return result;
				}, ui_state);

			CheckDynamicListRevertButtons(data, def);
		}
	};
}
