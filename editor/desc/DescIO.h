#pragma once

#include "core/Types.h"

#include "gui/DynamicList.h"
#include "gui/Widgets.h"

#include "desc/FieldSetAction.h"
#include "desc/DynamicListUndoActions.h"

#include <imtk.hpp>

namespace oly::editor
{
	struct DescIO
	{
		template<typename T>
		static void RowInputData(std::string_view label, T& data, const T& def, std::unique_ptr<imtk::w::widget>&& widget)
		{
			imtk::prop::key::set_label(label);

			if (data != def)
				imtk::prop::reset::button();

			imtk::prop::value::add_component(std::move(widget));
			
			imtk::prop::row::submit();
			if (imtk::prop::reset::any_activated())
				data = def;
		}

		template<typename T>
		static void RowInputData(std::string_view label, imtk::edit_session<T>& data, const T& def, std::unique_ptr<imtk::w::widget>&& widget)
		{
			imtk::prop::key::set_label(label);

			if (data.buffer() != def)
				imtk::prop::reset::button();

			imtk::prop::value::add_component(std::move(widget));

			imtk::prop::row::submit();
			if (imtk::prop::reset::any_activated())
				data.publish_reset(def);
		}

		template<typename T>
		static void Draw(std::string_view label, T& data, const T& def)
		{
			RowInputData(label, data, def, std::make_unique<imtk::w::bound_widget<T>>(data));
		}

		template<typename T>
		static void Draw(std::string_view label, imtk::edit_session<T>& data, const T& def)
		{
			RowInputData(label, data, def, std::make_unique<imtk::w::bound_widget<imtk::edit_session<T>>>(data));
		}

		template<typename T, typename U = T>
		static void Draw(std::string_view label, T& data, const T& def, imp::potential<U> min, imp::potential<U> max)
		{
			auto widget = std::make_unique<imtk::w::bound_widget<T>>(data);
			widget->config.min = min;
			widget->config.max = max;
			RowInputData(label, data, def, std::move(widget));
		}

		template<typename T, typename U = T>
		static void Draw(std::string_view label, imtk::edit_session<T>& data, const T& def, imp::potential<U> min, imp::potential<U> max)
		{
			auto widget = std::make_unique<imtk::w::bound_widget<imtk::edit_session<T>>>(data);
			widget->subwidget.config.min = min;
			widget->subwidget.config.max = max;
			RowInputData(label, data, def, std::move(widget));
		}

		template<typename T, typename U = T>
		static void Draw(std::string_view label, imtk::edit_session<imp::potential<T>>& data, const imp::potential<T>& def, imp::potential<U> min, imp::potential<U> max)
		{
			auto widget = std::make_unique<imtk::w::bound_widget<imtk::edit_session<imp::potential<T>>>>(data);
			widget->subwidget.value.config.min = min;
			widget->subwidget.value.config.max = max;
			RowInputData(label, data, def, std::move(widget));
		}

		static void Draw(std::string_view label, int& data, const int& def, imtk::label_span_registry::handle names);

		template<size_t N>
		static void Draw(std::string_view label, imtk::edit_session<std::array<std::string, N>>& data, const std::array<std::string, N>& def, const char** sublabels)
		{
			imtk::prop::view_generator generator = [&data]() {
				auto view = std::make_unique<imtk::prop::view_list>();
				view->subviews.reserve(N);
				for (size_t i = 0; i < N; ++i)
					view->subviews.push_back(std::make_unique<imtk::prop::simple_view<std::string>>(data.buffer()[i]));
				return view;
			};

			if (auto subform = imtk::prop::subform(label, generator))
			{
				imtk::item_state list_state;
				data.pre_edit();

				for (size_t i = 0; i < N; ++i)
				{
					auto widget = std::make_unique<imtk::w::bound_widget<std::string>>(data.buffer()[i]);
					if (sublabels && sublabels[i])
						RowInputData(sublabels[i], data.buffer()[i], def[i], std::move(widget));
					else
						RowInputData(std::to_string(i), data.buffer()[i], def[i], std::move(widget));

					list_state |= imtk::prop::value::get_draw_result().state;
				}

				data.post_edit(list_state);
			}
		}

		static void Draw(std::string_view label, bool* data, const bool* def, const char** sublabels, size_t count, bool inline_checkboxes);
		static void Draw(std::string_view label, bool* data, const bool* def, const char** sublabels, const bool* disabled, size_t count, bool inline_checkboxes);

		template<typename E> requires std::is_enum_v<E>
		static void Draw(std::string_view label, E& data, const E& def)
		{
			imtk::id_scope scope(&data);
			imtk::prop::key::set_label(label);
			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([&data]() -> imtk::item_result { return DrawCombo(data); }));
			if (data != def)
				imtk::prop::reset::button();
			imtk::prop::row::submit();
			if (imtk::prop::reset::any_activated())
				data = def;
		}

	private:
		template<typename E> requires std::is_enum_v<E>
		static imtk::item_result DrawCombo(E& data);

		template<typename E, size_t N> requires std::is_enum_v<E>
		static imtk::item_result DrawEnumCombo(E& data, const char* const (&values)[N])
		{
			int index = static_cast<int>(data);
			auto span = imtk::label_span_registry::intern(std::span<const char* const>(values, N));
			auto result = imtk::w::combo_widget(index, span).draw();
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
		static void DrawDynamicList(const imtk::datapath_link& link, std::string_view label, const imtk::desc::vector<T>& data, const std::vector<T>& def,
			std::function<imtk::item_result(gui::DynamicRow&)> draw_fn, gui::DynamicListState& ui_state)
		{
			imtk::id_scope scope(&data);
			imtk::prop::key::set_label(label);
			if (data.size() != def.size())
				imtk::prop::reset::button(0);

			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([&link, &data, &ui_state, draw_fn = std::move(draw_fn)]() -> imtk::item_result
				{ return ValueDrawDynamicList<T, Printer>(link, data, draw_fn, ui_state); }));

			imtk::prop::row::submit();
			if (imtk::prop::reset::activated(0))
				ui_state.DeferResize(def.size());
		}

		// TODO v9.3 use std::vector<imtk::edit_session<T>> instead
		template<typename T, typename Printer = StandardPrinter<T>>
		static void DrawDynamicList(const imtk::datapath_link& link, std::string_view label, imtk::edit_session<std::vector<T>>& data, const std::vector<T>& def,
			std::function<imtk::item_result(gui::DynamicRow&)> draw_fn, gui::DynamicListState& ui_state)
		{
			imtk::id_scope scope(&data);
			imtk::prop::key::set_label(label);
			if (data.buffer().size() != def.size())
				imtk::prop::reset::button(0);

			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([&link, &data, &ui_state, draw_fn = std::move(draw_fn)]() -> imtk::item_result
				{ return ValueDrawDynamicList<T, Printer>(link, data, draw_fn, ui_state); }));

			imtk::prop::row::submit();
			data.post_edit(imtk::prop::value::get_draw_result().state);
			if (imtk::prop::reset::activated(0))
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
						imtk::prop::reset::button(1 + i);
				}
				else
				{
					if (data[i] != T{})
						imtk::prop::reset::button(1 + i);
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
						imtk::prop::reset::button(1 + i);
				}
				else
				{
					if (data.buffer()[i] != T{})
						imtk::prop::reset::button(1 + i);
				}
			}
		}

		template<typename T>
		static void CheckDynamicListRevertButtons(std::vector<T>& data, const std::vector<T>& def)
		{
			for (size_t i = 0; i < data.size(); ++i)
			{
				if (imtk::prop::reset::activated(1 + i))
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
				if (imtk::prop::reset::activated(1 + i))
				{
					reset[i] = i < def.size() ? def[i] : T{};
					publish = true;
				}
			}

			if (publish)
				data.publish_reset(std::move(reset));
		}

		template<typename T> requires (!std::is_enum_v<T>)
		static void Draw(const imtk::datapath_link& link, std::string_view label, std::vector<T>& data, const std::vector<T>& def, gui::DynamicListState& ui_state)
		{
			DrawDynamicListRevertButtons(data, def);

			DrawDynamicList(std::move(link), label, data, def, [&data, &def](gui::DynamicRow& row) {
				imtk::item_result result;

				ImGui::SameLine();
				result |= imtk::w::widget<T>(data[row.Index()]).draw();

				if (ImGui::IsItemActivated())
					row.OnSelect();

				return result;
				}, ui_state);

			CheckDynamicListRevertButtons(data, def);
		}

		template<typename E> requires (std::is_enum_v<E>)
		static void Draw(const imtk::datapath_link& link, std::string_view label, std::vector<E>& data, const std::vector<E>& def, gui::DynamicListState& ui_state)
		{
			DrawDynamicListRevertButtons(data, def);

			DrawDynamicList(std::move(link), label, data, def, [&data, &def](gui::DynamicRow& row) {
				imtk::item_result result;

				ImGui::SameLine();
				result |= DrawCombo(data[row.Index()]);

				if (ImGui::IsItemActivated())
					row.OnSelect();

				return result;
				}, ui_state);

			CheckDynamicListRevertButtons(data, def);
		}
	};
}
