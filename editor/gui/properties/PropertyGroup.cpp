#include "PropertyGroup.h"

#include "core/Errors.h"
#include "gui/scopes/IDScope.h"

#include <imgui.h>

#include <string>
#include <variant>

namespace oly::editor
{
	struct PropertyNode
	{
		using Row = std::vector<std::unique_ptr<IPropertyView>>;

		PropertyNode* parent = nullptr;
		std::vector<std::variant<Row, std::unique_ptr<PropertyNode>>> subnodes;
		size_t active_row_index = 0;

		IPropertyView* Append(std::unique_ptr<IPropertyView>&& prop)
		{
			IPropertyView* p = prop.get();

			if (active_row_index < subnodes.size())
			{
				if (auto row = std::get_if<Row>(&subnodes[active_row_index]))
				{
					row->push_back(std::move(prop));
					return p;
				}
				else
					active_row_index = subnodes.size();
			}

			Row new_row;
			new_row.push_back(std::move(prop));
			subnodes.push_back(std::move(new_row));
			return p;
		}

		void EndRow()
		{
			++active_row_index;
		}

		PropertyNode* AppendNode()
		{
			auto subnode = std::make_unique<PropertyNode>();
			subnode->parent = this;
			PropertyNode* n = subnode.get();
			subnodes.push_back(std::move(subnode));
			++active_row_index;
			return n;
		}

		Row* GetLastRow()
		{
			if (!subnodes.empty())
				return std::get_if<Row>(&subnodes.back());
			else
				return nullptr;
		}

		PropertyNode* GetLastHeader()
		{
			if (!subnodes.empty())
			{
				if (auto node = std::get_if<std::unique_ptr<PropertyNode>>(&subnodes.back()))
					return node->get();
				else
					return nullptr;
			}
			else
				return nullptr;
		}
	};

	static std::unique_ptr<PropertyNode> PROPERTIES;
	static PropertyNode* ACTIVE_PROPERTY_NODE = nullptr;
	static size_t CONTEXT_MENU_ID_COUNTER = 0;

	void PropertyGroup::Begin()
	{
		PROPERTIES = std::make_unique<PropertyNode>();
		ACTIVE_PROPERTY_NODE = PROPERTIES.get();
		CONTEXT_MENU_ID_COUNTER = 0;
	}

	void PropertyGroup::End()
	{
		// NOP
	}

	static bool BeginContextMenu()
	{
		return ImGui::BeginPopupContextItem(("##" + std::to_string(CONTEXT_MENU_ID_COUNTER++)).c_str());
	}
	
	bool PropertyGroup::Append(std::unique_ptr<IPropertyView>&& prop)
	{
		if (ACTIVE_PROPERTY_NODE)
		{
			IPropertyView* p = ACTIVE_PROPERTY_NODE->Append(std::move(prop));
			bool dirty = false;
			if (BeginContextMenu())
			{
				dirty = PropertyClipboard::ContextMenuItems(std::span<IPropertyView*>(&p, 1));
				ImGui::EndPopup();
			}
			return dirty;
		}
		else
			BreakoutError::Throw("PropertyGroup::Append(): active property node is null");
	}

	// TODO v9.1 Remove Indent. If a tree node is collapsed, its values should still be copied even though the draw functions never execute -> so replace Append() with CheckValue(datapath), and pass datapaths to CheckRow()/CheckHeader() -> no data structure should be stored here.

	PropertyGroup::Indent::Indent()
	{
		if (ACTIVE_PROPERTY_NODE)
		{
			ACTIVE_PROPERTY_NODE = ACTIVE_PROPERTY_NODE->AppendNode();
			_active = true;
		}
		else
			BreakoutError::Throw("PropertyGroup::Indent::Indent(): active property node is null");
	}

	PropertyGroup::Indent::Indent(Indent&& o) noexcept
		: _active(o._active)
	{
		o._active = false;
	}

	PropertyGroup::Indent::~Indent()
	{
		if (_active)
		{
			if (ACTIVE_PROPERTY_NODE)
			{
				ACTIVE_PROPERTY_NODE = ACTIVE_PROPERTY_NODE->parent;
				CheckHeader();
			}
			else
				BreakoutError::Throw("PropertyGroup::Indent::~Indent(): active property node is null");
		}
	}

	bool PropertyGroup::CheckRow()
	{
		if (ACTIVE_PROPERTY_NODE)
		{
			ACTIVE_PROPERTY_NODE->EndRow();
			if (auto row = ACTIVE_PROPERTY_NODE->GetLastRow())
			{
				bool dirty = false;
				if (BeginContextMenu())
				{
					std::vector<IPropertyView*> properties;
					properties.reserve(row->size());
					for (auto& prop : *row)
						properties.push_back(prop.get());

					dirty = PropertyClipboard::ContextMenuItems(properties);
					ImGui::EndPopup();
				}
				return dirty;
			}
			else
				return false;
		}
		else
			BreakoutError::Throw("PropertyGroup::CheckRow(): active property node is null");
	}

	static void AddHeaderProperties(PropertyNode& node, std::vector<IPropertyView*>& properties)
	{
		for (auto& s : node.subnodes)
		{
			if (auto row = std::get_if<PropertyNode::Row>(&s))
			{
				for (auto& prop : *row)
					properties.push_back(prop.get());
			}
			else if (auto subnode = std::get_if<std::unique_ptr<PropertyNode>>(&s))
				AddHeaderProperties(**subnode, properties);
		}
	}

	bool PropertyGroup::CheckHeader()
	{
		if (ACTIVE_PROPERTY_NODE)
		{
			if (auto subnode = ACTIVE_PROPERTY_NODE->GetLastHeader())
			{
				bool dirty = false;
				if (BeginContextMenu())
				{
					std::vector<IPropertyView*> properties;
					AddHeaderProperties(*subnode, properties);

					dirty = PropertyClipboard::ContextMenuItems(properties);
					ImGui::EndPopup();
				}
				return dirty;
			}
			else
				return false;
		}
		else
			BreakoutError::Throw("PropertyGroup::CheckHeader(): active property node is null");
	}
}
