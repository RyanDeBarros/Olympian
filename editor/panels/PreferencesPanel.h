#pragma once

#include "panels/IPanel.h"
#include "desc/PreferencesDesc.h"

#include "gui/scopes/Form.h"

namespace oly::editor
{
	class PreferencesPanel : public IPanel
	{
		bool _dirty = false;

		PreferencesDesc _scratch;
		PreferencesDesc _disk;
		
	public:
		static PreferencesPanel& Instance();

		void Init() override;
		const char* GetTitle() const override;
		void Draw() override;

		std::filesystem::path GetPath() const;

	private:
		void Load();
		void Dump();

	public:
		void MarkDirty();
		void MarkClean();
		bool IsDirty() const;

		const PreferencesDesc& GetSavedDesc() const;

	private:
		void Draw(PreferencesDesc& desc);
		void Draw(Form& form, TreeViewSettingsDesc& desc);
		void Draw(Form& form, TreeViewAdvancedSettingsDesc& desc);

		void Load(TOMLNode node, PreferencesDesc& desc);
		void Load(TOMLNode node, TreeViewSettingsDesc& desc);
		void Load(TOMLNode node, TreeViewAdvancedSettingsDesc& desc);
		
		void Dump(toml::table& table, PreferencesDesc& desc);
		void Dump(toml::table& table, TreeViewSettingsDesc& desc);
		void Dump(toml::table& table, TreeViewAdvancedSettingsDesc& desc);
	};
}
