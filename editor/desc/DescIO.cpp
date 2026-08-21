#include "DescIO.h"

#include "definitions/Keys.h"

#include <imgui.h>

#include <span>

// TODO DEBT support more complex property views. For example, a dynamic list of strings should be able to paste into another, even though they might have different sizes. Another example is dynamic descriptors, such as checkboxes or combos enabling/disabling sections.

namespace oly::editor
{
	// TODO v9.3 prop::multi_row_scope
	void DescIO::Draw(std::string_view label, bool* data, const bool* def, imtk::label_span_registry::handle sublabels, const bool* disabled, size_t count, bool inline_checkboxes)
	{
		imtk::id_scope scope(data);
		imtk::prop::key::set_label(label);

		for (size_t i = 0; i < count; ++i)
		{
			if (data[i] != def[i])
			{
				imtk::prop::reset::button();
				break;
			}
		}

		imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([&data, sublabels, disabled, count, inline_checkboxes]() -> imtk::item_result {
			imtk::item_result result;
			
			for (size_t i = 0; i < count; ++i)
			{
				if (auto d = imtk::disabled(disabled && disabled[i]))
				{
					result |= imtk::w::bound_widget<bool>(data[i], { .label = imtk::label_span_registry::string(sublabels, i) }).draw();
					if (inline_checkboxes && i + 1 < count)
						ImGui::SameLine();
				}
			}

			return result;
		}));

		imtk::prop::row::submit();

		if (imtk::prop::reset::any_activated())
		{
			for (size_t i = 0; i < count; ++i)
				data[i] = def[i];
		}
	}
}
