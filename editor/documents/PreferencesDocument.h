#pragma once

#include "documents/IDocument.h"

#include "desc/impl/PreferencesDesc.h"
#include "desc/DoubleDescriptor.h"

namespace oly::editor
{
	class PreferencesDocument : public IDocument
	{
		DoubleDescriptor<PreferencesDesc> _desc;

	public:
		static const char* GetVersion();

		PreferencesDocument();

		void InitImpl() override;
		void Draw() override;
		void DrawMenuBar() override;
		void LoadImpl() override;
		void DumpImpl() override;
		void ResetAssetImpl() override;
		const IDoubleDescriptor& GetDoubleDescriptor() const override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

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

		void Load(TOMLNode node, PreferencesDesc& desc);
		void Load(TOMLNode node, EditSettingsDesc& desc);
		void Load(TOMLNode node, UndoHistorySettingsDesc& desc);
		void Load(TOMLNode node, ContentBrowserSettingsDesc& desc);
		void Load(TOMLNode node, TreeViewSettingsDesc& desc);
		void Load(TOMLNode node, TreeViewAdvancedSettingsDesc& desc);
		void Load(TOMLNode node, FilesystemSettingsDesc& desc);

		void Dump(toml::table& table, PreferencesDesc& desc);
		void Dump(toml::table& table, EditSettingsDesc& desc);
		void Dump(toml::table& table, UndoHistorySettingsDesc& desc);
		void Dump(toml::table& table, ContentBrowserSettingsDesc& desc);
		void Dump(toml::table& table, TreeViewSettingsDesc& desc);
		void Dump(toml::table& table, TreeViewAdvancedSettingsDesc& desc);
		void Dump(toml::table& table, FilesystemSettingsDesc& desc);
	};
}
