#pragma once

#include "desc/Fields.h"

#include "core/MemoryUnit.h"

namespace oly::editor
{
#define UNDO_HISTORY_SETTINGS_GENERATOR(M) \
		M((IntField<MakeOpt(1), MakeOpt<int>()>), count_limit) \
		M((IntField<MakeOpt(1), MakeOpt<int>()>), size_limit) \
		M((EnumField<MemoryUnit>), size_limit_unit)

	struct UndoHistorySettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(UndoHistorySettingsDesc, UNDO_HISTORY_SETTINGS_GENERATOR);

		UndoHistorySettingsDesc(imtk::datapath_link link = {});

		size_t CountLimit() const;
		size_t SizeLimit() const;
	};

#define EDIT_SETTINGS_GENERATOR(M) \
		M((UndoHistorySettingsDesc), undo_history)

	struct EditSettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(EditSettingsDesc, EDIT_SETTINGS_GENERATOR);

		static const detail::Key undo_history_key; // TODO v9.3 some way of attaching key directly - perhaps with imtk::subdesc<UndoHistorySettingsDesc> and doing undo_history(link, imtk::subdesc_key<detail::Key::UndoHistory>) in constructor in cpp. Then can simply do undo_history.key or use */-> to access subdesc

		EditSettingsDesc(imtk::datapath_link link = {});
	};

#define CONTENT_BROWSER_SETTINGS_GENERATOR(M) \
		M((IntField<MakeOpt(1), MakeOpt<int>()>), folder_history_limit)

	struct ContentBrowserSettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(ContentBrowserSettingsDesc, CONTENT_BROWSER_SETTINGS_GENERATOR);

		ContentBrowserSettingsDesc(imtk::datapath_link link = {});
	};

#define TREE_VIEW_ADVANCED_SETTINGS_GENERATOR(M) \
		M((FloatField<MakeOpt(0.1f), MakeOpt<float>()>), analysis_interval)

	struct TreeViewAdvancedSettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(TreeViewAdvancedSettingsDesc, TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);

		TreeViewAdvancedSettingsDesc(imtk::datapath_link link = {});
		
		float AnalysisInterval() const;
	};

#define TREE_VIEW_SETTINGS_GENERATOR(M) \
		M((TreeViewAdvancedSettingsDesc), advanced)

	struct TreeViewSettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(TreeViewSettingsDesc, TREE_VIEW_SETTINGS_GENERATOR);

		static const detail::Key advanced_key;

		TreeViewSettingsDesc(imtk::datapath_link link = {});
	};

#define FILESYSTEM_SETTINGS_GENERATOR(M) \
		M((IntField<MakeOpt(1), MakeOpt<int>()>), trash_limit) \
		M((EnumField<MemoryUnit>), trash_limit_unit)

	struct FilesystemSettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(FilesystemSettingsDesc, FILESYSTEM_SETTINGS_GENERATOR);

		FilesystemSettingsDesc(imtk::datapath_link link = {});

		size_t TrashLimit() const;
	};

#define PREFERENCES_GENERATOR(M) \
		M((EditSettingsDesc), edit) \
		M((ContentBrowserSettingsDesc), content_browser) \
		M((TreeViewSettingsDesc), tree_view) \
		M((FilesystemSettingsDesc), filesystem)

	struct PreferencesDesc
	{
		IMTK_DESCRIPTOR_BODY(PreferencesDesc, PREFERENCES_GENERATOR);

		static const detail::Key edit_key;
		static const detail::Key content_browser_key;
		static const detail::Key tree_view_key;
		static const detail::Key filesystem_key;

		PreferencesDesc(imtk::datapath_link link = {});
	};
}
