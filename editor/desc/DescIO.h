#pragma once

#include "gui/DynamicList.h"

#include <imtk.hpp>

namespace oly::editor
{
	struct DescIO
	{
		template<size_t N>
		static void Draw(std::string_view label, imtk::edit_session<std::array<std::string, N>>& data, const std::array<std::string, N>& def, imtk::label_span_registry::handle sublabels)
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
					const char* sublabel = sublabels ? imtk::label_span_registry::string(sublabels, i) : nullptr;
					if (auto row = sublabel
							? imtk::prop::make_row_scope(sublabel, data.buffer()[i], def[i])
							: imtk::prop::make_row_scope(std::to_string(i), data.buffer()[i], def[i]))
					{
						imtk::prop::value::add_component(std::make_unique<imtk::w::bound_widget<std::string>>(data.buffer()[i]));
					}

					list_state |= imtk::prop::value::get_draw_result().state;
				}

				data.post_edit(list_state);
			}
		}

		template<typename T, typename Printer = imtk::standard_printer<T>>
		static void DrawDynamicList(const imtk::datapath_link& link, std::string_view label, const imtk::desc::vector<T>& data, const std::vector<T>& def,
			std::function<imtk::item_result(gui::DynamicRow&)> draw_fn, gui::DynamicListState& ui_state)
		{
			imtk::id_scope scope(&data);
			imtk::prop::key::set_label(label);
			if (data.size() != def.size())
				imtk::prop::reset::button(0);

			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([&link, &data, &ui_state, draw_fn = std::move(draw_fn)]() {
				imtk::item_result result;

				ui_state.DrawListHeader(data.size());

				ui_state.DrawBody([&result, &draw_fn](gui::DynamicRow& row) {
					ImGui::SameLine();
					auto row_result = draw_fn(row);
					result |= row_result;
					if (row_result.state.left_clicked() || row_result.state.focused())
						row.OnSelect();
					});

				result.modified |= ui_state.VisitRowOps([&link](const imtk::list_op& op) {
					op.execute_desc_action<T, Printer>(link.compute_path());
				});

				return result;
			}));

			imtk::prop::row::submit();
			if (imtk::prop::reset::activated(0))
				ui_state.DeferResize(def.size());
		}

		template<typename T, typename Printer = imtk::standard_printer<T>>
		static void DrawDynamicList(const imtk::datapath_link& link, std::string_view label, imtk::edit_session<std::vector<T>>& data, const std::vector<T>& def,
			std::function<imtk::item_result(gui::DynamicRow&)> draw_fn, gui::DynamicListState& ui_state)
		{
			imtk::id_scope scope(&data);
			imtk::prop::key::set_label(label);
			if (data.buffer().size() != def.size())
				imtk::prop::reset::button(0);

			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([&link, &data, &ui_state, draw_fn = std::move(draw_fn)]() {
				imtk::item_result result;

				ui_state.DrawListHeader(data.buffer().size());

				ui_state.DrawBody([&result, &draw_fn](gui::DynamicRow& row) {
					ImGui::SameLine();
					auto row_result = draw_fn(row);
					result |= row_result;
					if (row_result.state.left_clicked() || row_result.state.focused())
						row.OnSelect();
					});

				result.modified |= ui_state.VisitRowOps([&link, &data](const imtk::list_op& op) {
					data.cancel_editing();
					op.execute_field_action<T, Printer>(link.compute_path());
				});

				return result;
			}));

			imtk::prop::row::submit();
			data.post_edit(imtk::prop::value::get_draw_result().state);
			if (imtk::prop::reset::activated(0))
				ui_state.DeferResize(def.size());
		}

		// TODO v9.3 replace with imtk::prop::multi_row_scope
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
