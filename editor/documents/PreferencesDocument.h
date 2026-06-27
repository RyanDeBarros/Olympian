#pragma once

#include "documents/IDocument.h"

#include "desc/impl/PreferencesDesc.h"

namespace oly::editor
{
	class PreferencesDocument : public IDocument
	{
		PreferencesDesc _scratch;
		PreferencesDesc _disk;

	public:
		static const char* GetVersion();

		PreferencesDocument();

		void InitImpl() override;
		void Draw() override;
		void DrawMenuBar() override;
		void Load() override;
		void Dump() override;
		void* VisitPath(DataPath path, std::type_index type) override;
		bool DrawFinalizeImpl() override;

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
