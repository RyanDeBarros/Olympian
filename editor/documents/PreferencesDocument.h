#pragma once

#include "documents/IDocument.h"

#include "desc/impl/PreferencesDesc.h"

namespace oly::editor
{
	class PreferencesDocument : public IDocument
	{
		PreferencesDesc _scratch;
		PreferencesDesc _disk;
		// TODO v9.3 third Desc for _in_effect. That way, you can apply settings without saving them. Replace GetSavedDesc() with GetActiveDesc() that returns _in_effect

	public:
		static const char* GetVersion();

		PreferencesDocument();

		void Init() override;
		void Draw() override;
		void Load() override;
		void Dump() override;

		const PreferencesDesc& GetSavedDesc() const;

	private:
		void Draw(PreferencesDesc& desc);
		void Draw(TreeViewSettingsDesc& desc);
		void Draw(TreeViewAdvancedSettingsDesc& desc);

		void Load(TOMLNode node, PreferencesDesc& desc);
		void Load(TOMLNode node, TreeViewSettingsDesc& desc);
		void Load(TOMLNode node, TreeViewAdvancedSettingsDesc& desc);

		void Dump(toml::table& table, PreferencesDesc& desc);
		void Dump(toml::table& table, TreeViewSettingsDesc& desc);
		void Dump(toml::table& table, TreeViewAdvancedSettingsDesc& desc);
	};
}
