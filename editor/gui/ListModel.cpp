#include "ListModel.h"

#include "core/editor/ResourceLoader.h"

#include <string>

namespace oly::editor::gui
{
	imtk::item_result ListModel::DrawComboHeader(const ComboHeader& header, const char* slot_prefix)
	{
		return DrawComboHeader(header, [slot_prefix](size_t i) { return slot_prefix + (" " + std::to_string(i)); });
	}

	imtk::item_result ListModel::DrawComboHeader(const ComboHeader& header, std::function<std::string(size_t)> combo_getter)
	{
		imtk::item_result result;

		imtk::style_color sc(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_FrameBg, 0.75f));

		if (auto _ = imtk::child(header.prompt, ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders))
		{
			ImGui::TextUnformatted(header.prompt);
			ImGui::SameLine();

			imtk::item_result subresult;

			std::vector<std::string> slot_names;
			slot_names.reserve(model.size());
			for (int i = 0; i < model.size(); ++i)
				slot_names.push_back(combo_getter(i));

			int slot = model.index();
			result |= imtk::controls::combo("##SelectSlot", slot, slot_names);
			model.on_select(slot, false, false);

			ImGui::SameLine();
			subresult = imtk::w::icon_button({ .icon = Icon(IconResource::Plus), .str_id = "##+", .tooltip = header.create_tooltip }).draw();
			result |= subresult;
			if (subresult.modified)
				model.defer_append();

			ImGui::SameLine();
			subresult = imtk::w::icon_button({ .icon = Icon(IconResource::Minus), .str_id = "##-", .tooltip = header.delete_tooltip }).draw();
			result |= subresult;
			if (subresult.modified)
				model.defer_delete();

			ImGui::SameLine();
			subresult = imtk::w::icon_button({ .icon = Icon(IconResource::Close), .str_id = "##x", .tooltip = header.clear_tooltip }).draw();
			result |= subresult;
			if (subresult.modified)
				model.defer_clear();
		}

		return result;
	}
}
