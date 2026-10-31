#pragma once

#include "desc/PreferencesDesc.h"

#include "documents/IDocument.h"

namespace oly::editor
{
    class PreferencesDocument
        : public IDocument
    {
        imtk::desc::doubler<PreferencesDesc> desc_;

    public:
        PreferencesDocument();

    public:
        void InitImpl() override;
        void Draw() override;
        void DrawMenuBar() override;
        void LoadImpl() override;
        void DumpImpl() override;
        void ResetAssetImpl() override;
        const imtk::desc::idoubler& GetDoubleDescriptor() const override;
        imtk::desc::idoubler& GetDoubleDescriptor() override;

    public:
        static const char* GetVersion();

        void ApplyEditorPreferences();
        void RevertEditorPreferences();

    private:
        void ActiveDescChanged();

        void Draw(PreferencesDesc& desc);
        void Draw(EditSettingsDesc& desc);
        void Draw(UndoHistorySettingsDesc& desc);
        void Draw(ContentBrowserSettingsDesc& desc);
        void Draw(TreeViewSettingsDesc& desc);
        void Draw(TreeViewAdvancedSettingsDesc& desc);
        void Draw(FilesystemSettingsDesc& desc);

        void Load(imtk::toml_node node, PreferencesDesc& desc);
        void Load(imtk::toml_node node, EditSettingsDesc& desc);
        void Load(imtk::toml_node node, UndoHistorySettingsDesc& desc);
        void Load(imtk::toml_node node, ContentBrowserSettingsDesc& desc);
        void Load(imtk::toml_node node, TreeViewSettingsDesc& desc);
        void Load(imtk::toml_node node, TreeViewAdvancedSettingsDesc& desc);
        void Load(imtk::toml_node node, FilesystemSettingsDesc& desc);

        void Dump(toml::table& table, PreferencesDesc& desc);
        void Dump(toml::table& table, EditSettingsDesc& desc);
        void Dump(toml::table& table, UndoHistorySettingsDesc& desc);
        void Dump(toml::table& table, ContentBrowserSettingsDesc& desc);
        void Dump(toml::table& table, TreeViewSettingsDesc& desc);
        void Dump(toml::table& table, TreeViewAdvancedSettingsDesc& desc);
        void Dump(toml::table& table, FilesystemSettingsDesc& desc);
    };
}
