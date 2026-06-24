#include "PropertyClipboard.h"

#include "gui/scopes/DisabledSection.h"

#include <imgui.h>

#include <cstring>

namespace oly::editor
{
	namespace prop
	{
		PropUID _prop_uid_counter = 0;

		const PropUID NULL_UID = OLY_DECL_PROP_UID;
	}

	RawPropertyPayload::RawPropertyPayload()
		: type(prop::NULL_UID)
	{
	}

	RawPropertyPayload RawPropertyPayload::Make(const void* data, size_t size, PropUID type)
	{
		RawPropertyPayload payload;
		payload.data.resize(size);
		std::memcpy(payload.data.data(), data, size);
		payload.type = type;
		return payload;
	}

	struct RawPropertyPayloadRow
	{
		std::vector<RawPropertyPayload> list;
	};

	struct RawPropertyPayloadPage
	{
		using V = std::variant<RawPropertyPayloadRow, std::unique_ptr<RawPropertyPayloadPage>>;
		std::vector<V> page;
	};

	static RawPropertyPayloadPage CLIPBOARD;

	void PropertyClipboard::Clear()
	{
		CLIPBOARD.page.clear();
	}

	static void Store(const IPropertyView& prop)
	{
		CLIPBOARD.page.clear();
		
		RawPropertyPayloadRow row;
		row.list.push_back(prop.Dump());
		
		CLIPBOARD.page.push_back(std::move(row));
	}
	
	static void Store(const PropertyRow& props)
	{
		CLIPBOARD.page.clear();
		
		RawPropertyPayloadRow row;
		row.list.reserve(props.list.size());
		for (auto& prop : props.list)
			row.list.push_back(prop->Dump());
		
		CLIPBOARD.page.push_back(std::move(row));
	}

	static void StorePage(const PropertyPage& props, RawPropertyPayloadPage& payload)
	{
		payload.page.reserve(props.page.size());
		for (auto& prop_el : props.page)
		{
			if (auto prop_row = std::get_if<PropertyRow>(&prop_el))
			{
				RawPropertyPayloadRow row;
				row.list.reserve(prop_row->list.size());
				for (auto& prop : prop_row->list)
					row.list.push_back(prop->Dump());

				payload.page.push_back(std::move(row));
			}
			else
			{
				const auto& prop_page = std::get<std::unique_ptr<PropertyPage>>(prop_el);
				auto page = std::make_unique<RawPropertyPayloadPage>();
				StorePage(*prop_page, *page);
				
				payload.page.push_back(std::move(page));
			}
		}
	}
	
	static void Store(const PropertyPage& props)
	{
		CLIPBOARD.page.clear();
		StorePage(props, CLIPBOARD);
	}

	static bool CanPaste(const IPropertyView& prop)
	{
		if (CLIPBOARD.page.size() != 1)
			return false;

		auto row = std::get_if<RawPropertyPayloadRow>(&CLIPBOARD.page.front());
		if (!row || row->list.size() != 1)
			return false;

		return prop.CanParse(row->list.front());
	}

	static bool CanPasteRow(const PropertyRow& props, const RawPropertyPayloadRow& payload)
	{
		if (payload.list.size() != props.list.size())
			return false;

		for (size_t i = 0; i < props.list.size(); ++i)
			if (!props.list[i]->CanParse(payload.list[i]))
				return false;

		return true;
	}
	
	static bool CanPaste(const PropertyRow& props)
	{
		if (CLIPBOARD.page.size() != 1)
			return false;

		if (auto row = std::get_if<RawPropertyPayloadRow>(&CLIPBOARD.page.front()))
			return CanPasteRow(props, *row);
		else
			return false;
	}

	static bool CanPastePage(const PropertyPage& props, const RawPropertyPayloadPage& payload)
	{
		if (payload.page.empty() || payload.page.size() != props.page.size())
			return false;

		for (size_t i = 0; i < props.page.size(); ++i)
		{
			if (auto prop_row = std::get_if<PropertyRow>(&props.page[i]))
			{
				auto row = std::get_if<RawPropertyPayloadRow>(&payload.page[i]);
				if (!row)
					return false;

				if (!CanPasteRow(*prop_row, *row))
					return false;
			}
			else
			{
				const auto& prop_page = std::get<std::unique_ptr<PropertyPage>>(props.page[i]);
				auto page = std::get_if<std::unique_ptr<RawPropertyPayloadPage>>(&payload.page[i]);
				if (!page)
					return false;

				if (!CanPastePage(*prop_page, **page))
					return false;
			}
		}

		return true;
	}
	
	static bool CanPaste(const PropertyPage& props)
	{
		return CanPastePage(props, CLIPBOARD);
	}

	static bool TryPaste(const IPropertyView& prop)
	{
		return prop.TryParse(std::get<RawPropertyPayloadRow>(CLIPBOARD.page.front()).list.front());
	}
	
	static bool TryPasteRow(const PropertyRow& props, const RawPropertyPayloadRow& payload)
	{
		bool dirty = false;
		for (size_t i = 0; i < props.list.size(); ++i)
			dirty |= props.list[i]->TryParse(payload.list[i]);

		return dirty;
	}

	static bool TryPaste(const PropertyRow& props)
	{
		auto& row = std::get<RawPropertyPayloadRow>(CLIPBOARD.page.front());
		return TryPasteRow(props, row);
	}
	
	static bool TryPastePage(const PropertyPage& props, const RawPropertyPayloadPage& payload)
	{
		bool dirty = false;
		for (size_t i = 0; i < props.page.size(); ++i)
		{
			if (auto prop_row = std::get_if<PropertyRow>(&props.page[i]))
			{
				auto& row = std::get<RawPropertyPayloadRow>(payload.page[i]);
				dirty |= TryPasteRow(*prop_row, row);
			}
			else
			{
				const auto& prop_page = std::get<std::unique_ptr<PropertyPage>>(props.page[i]);
				auto& page = std::get<std::unique_ptr<RawPropertyPayloadPage>>(payload.page[i]);
				dirty |= TryPastePage(*prop_page, *page);
			}
		}

		return dirty;
	}

	static bool TryPaste(const PropertyPage& props)
	{
		return TryPastePage(props, CLIPBOARD);
	}

	template<typename T>
	bool GenericContextMenuItems(const T& props)
	{
		if (ImGui::MenuItem("Copy"))
			Store(props);

		if (auto disabled = DisabledSection(!CanPaste(props)))
		{
			if (ImGui::MenuItem("Paste"))
				return TryPaste(props);
		}

		return false;
	}

	bool PropertyClipboard::ContextMenuItems(const IPropertyView& prop)
	{
		return GenericContextMenuItems(prop);
	}
	
	bool PropertyClipboard::ContextMenuItems(const PropertyRow& props)
	{
		return GenericContextMenuItems(props);
	}
	
	// TODO v9.1 pass row-by-row generator instead so that the full page doesn't need to be generated for CanPaste() check
	bool PropertyClipboard::ContextMenuItems(const PropertyPage& props)
	{
		return GenericContextMenuItems(props);
	}
}
