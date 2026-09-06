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
		static void DrawDynamicList(const imtk::datapath_link& link, std::string_view label, const imtk::desc::vector<T>& data, const std::vector<T>& def, gui::DynamicList& list)
		{
			imtk::id_scope scope(&data);
			imtk::prop::key::set_label(label);
			if (data.size() != def.size())
				imtk::prop::reset::button(0);

			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([&link, &data, &list]() {
				imtk::item_result result = list.Draw(data.size());

				result.modified |= list.model.visit_deferred_ops([&link](const imtk::list_op& op) {
					op.execute_desc_action<T, Printer>(link.compute_path());
				});

				return result;
			}));

			imtk::prop::row::submit();
			if (imtk::prop::reset::activated(0))
				list.model.defer_resize(def.size());
		}

		template<typename T, typename Printer = imtk::standard_printer<T>>
		static void DrawDynamicList(const imtk::datapath_link& link, std::string_view label, imtk::edit_session<std::vector<T>>& data, const std::vector<T>& def, gui::DynamicList& list)
		{
			imtk::id_scope scope(&data);
			imtk::prop::key::set_label(label);
			if (data.buffer().size() != def.size())
				imtk::prop::reset::button(0);

			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([&link, &data, &list]() {
				imtk::item_result result = list.Draw(data.buffer().size());

				result.modified |= list.model.visit_deferred_ops([&link, &data](const imtk::list_op& op) {
					data.cancel_editing();
					op.execute_field_action<T, Printer>(link.compute_path());
				});

				return result;
			}));

			imtk::prop::row::submit();
			data.post_edit(imtk::prop::value::get_draw_result().state);
			if (imtk::prop::reset::activated(0))
				list.model.defer_resize(def.size());
		}
	};
}
