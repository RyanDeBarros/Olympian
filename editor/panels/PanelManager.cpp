#include "PanelManager.h"

#include "IPanel.h"

namespace oly::editor
{
	void PanelManager::Init()
	{
		for (const auto& [_, panel] : _panels)
			panel->Init();
	}

	void PanelManager::Draw()
	{
		for (const auto& [_, panel] : _panels)
		{
			if (panel->IsOpen())
				panel->Draw();
		}
	}

	IPanel& PanelManager::Add(std::type_index index, std::unique_ptr<IPanel>&& panel)
	{
		return *_panels.try_emplace(index, std::move(panel)).first->second.get();
	}

	IPanel* PanelManager::Get(std::type_index index)
	{
		auto it = _panels.find(index);
		if (it != _panels.end())
			return it->second.get();
		else
			return nullptr;
	}
}
