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
		const IDoubleDescriptor& GetDoubleDescriptor() const override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

		void ApplyEditorPreferences();
		void RevertEditorPreferences();

	private:
		void ActiveDescChanged();

		void Draw(DataPath path, PreferencesDesc& desc);
		void Draw(DataPath path, EditSettingsDesc& desc);
		void Draw(DataPath path, UndoHistorySettingsDesc& desc);
		void Draw(DataPath path, TreeViewSettingsDesc& desc);
		void Draw(DataPath path, TreeViewAdvancedSettingsDesc& desc);

		void Load(TOMLNode node, PreferencesDesc& desc);
		void Load(TOMLNode node, EditSettingsDesc& desc);
		void Load(TOMLNode node, UndoHistorySettingsDesc& desc);
		void Load(TOMLNode node, TreeViewSettingsDesc& desc);
		void Load(TOMLNode node, TreeViewAdvancedSettingsDesc& desc);

		void Dump(toml::table& table, PreferencesDesc& desc);
		void Dump(toml::table& table, EditSettingsDesc& desc);
		void Dump(toml::table& table, UndoHistorySettingsDesc& desc);
		void Dump(toml::table& table, TreeViewSettingsDesc& desc);
		void Dump(toml::table& table, TreeViewAdvancedSettingsDesc& desc);
	};
}
