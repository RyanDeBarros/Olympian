#include "PanelManager.h"

#include "IPanel.h"

namespace oly::editor
{
    void PanelManager::Init()
    {
        for (const auto& [_, panel] : panels_)
            panel->Init();
    }

    void PanelManager::Draw()
    {
        for (const auto& [_, panel] : panels_)
            panel->Draw();
    }

    IPanel& PanelManager::Add(imp::type_erasure index, std::unique_ptr<IPanel>&& panel)
    {
        return *panels_.try_emplace(index, std::move(panel)).first->second.get();
    }

    IPanel* PanelManager::Get(imp::type_erasure index)
    {
        auto it = panels_.find(index);
        if (it != panels_.end())
            return it->second.get();
        else
            return nullptr;
    }
}
