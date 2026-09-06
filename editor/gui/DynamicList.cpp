#include "DynamicList.h"

#include "core/editor/ResourceLoader.h"

#include <imtk.hpp>

namespace oly::editor::gui
{
	DynamicListHeader::DynamicListHeader(imtk::list_model& model)
		: imtk::w::dynamic_list_header(model)
	{
		create_button.config.icon = Icon(IconResource::Plus);
		create_button.config.tooltip = "New item";
		delete_button.config.icon = Icon(IconResource::Minus);
		delete_button.config.tooltip = "Remove item (del)";
		clear_button.config.icon = Icon(IconResource::Close);
		clear_button.config.tooltip = "Clear items";
	}

	DynamicList::DynamicList()
		: header(model), body(model)
	{
	}

	DynamicList::DynamicList(const DynamicList& o)
		: model(o.model), header(model, o.header), body(model, o.body)
	{
	}
	
	DynamicList::DynamicList(DynamicList&& o) noexcept
		: model(std::move(o.model)), header(model, std::move(o.header)), body(model, std::move(o.body))
	{
	}

	imtk::item_result DynamicList::Draw(size_t list_size)
	{
		imtk::item_result result;
		model.sync(list_size);
		result |= header.draw();
		result |= body.draw();
		return result;
	}
}
