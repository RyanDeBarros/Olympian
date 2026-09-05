#include "ListModel.h"

#include "core/editor/ResourceLoader.h"

#include <string>

namespace oly::editor::gui
{
	static void ConfigureListIndexer(imtk::w::list_indexer& widget, std::string create_tooltip, std::string delete_tooltip, std::string clear_tooltip)
	{
		widget.configure_buttons(Icon(IconResource::Plus), std::move(create_tooltip),
			Icon(IconResource::Minus), std::move(delete_tooltip), Icon(IconResource::Close), std::move(clear_tooltip));
	}

	ListIndexer::ListIndexer(Ctor config, std::function<std::string(size_t)> combo_name)
	{
		ConfigureListIndexer(widget, std::move(config.create_tooltip), std::move(config.delete_tooltip), std::move(config.clear_tooltip));
		widget.prompt = std::move(config.prompt);
		widget.combo_name = std::move(combo_name);
	}
	
	ListIndexer::ListIndexer(Ctor config, std::string combo_slot_prefix)
	{
		ConfigureListIndexer(widget, std::move(config.create_tooltip), std::move(config.delete_tooltip), std::move(config.clear_tooltip));
		widget.prompt = std::move(config.prompt);
		widget.combo_name = imtk::w::make_combo_name_from_prefix(std::move(combo_slot_prefix));
	}
}
