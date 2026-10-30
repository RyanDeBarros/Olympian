#pragma once

#include "documents/PreferencesDocument.h"

#include "panels/IPanel.h"

namespace oly::editor
{
    class PreferencesPanel
        : public IPanel
    {
        PreferencesDocument doc_;
        imtk::unsaved_changes_modal window_unsaved_changes_modal_;
        imtk::unsaved_changes_modal shutdown_unsaved_changes_modal_;

    public:
        PreferencesPanel();

    public:
        void InitImpl() override;
        const char* GetTitle() const override;
        void Draw() override;

    public:
        static PreferencesPanel& Instance();

        bool RequestShutdown();

    private:
        bool DrawUnsavedChangesModal(imtk::unsaved_changes_modal& modal);
    };
}
