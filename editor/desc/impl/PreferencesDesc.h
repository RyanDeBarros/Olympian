#pragma once

#include "desc/Fields.h"

#include "core/MemoryUnit.h"

namespace oly::editor
{
#define UNDO_HISTORY_SETTINGS_GENERATOR(M) \
		M(count_limit) \
		M(size_limit) \
		M(size_limit_unit)

	struct UndoHistorySettingsDesc
	{
		DESCRIPTOR_BODY(UndoHistorySettingsDesc, UNDO_HISTORY_SETTINGS_GENERATOR);

		IntField<MakeOpt(1), MakeOpt<int>()> count_limit;
		IntField<MakeOpt(1), MakeOpt<int>()> size_limit;
		EnumField<MemoryUnit> size_limit_unit;

		UndoHistorySettingsDesc(imtk::datapath_link link = {});

		size_t CountLimit() const;
		size_t SizeLimit() const;
	};

#define EDIT_SETTINGS_GENERATOR(M) \
		M(undo_history)

	struct EditSettingsDesc
	{
		DESCRIPTOR_BODY(EditSettingsDesc, EDIT_SETTINGS_GENERATOR);

		UndoHistorySettingsDesc undo_history;
		static const detail::Key undo_history_key;

		EditSettingsDesc(imtk::datapath_link link = {});
	};

#define CONTENT_BROWSER_SETTINGS_GENERATOR(M) \
		M(folder_history_limit)

	struct ContentBrowserSettingsDesc
	{
		DESCRIPTOR_BODY(ContentBrowserSettingsDesc, CONTENT_BROWSER_SETTINGS_GENERATOR);

		IntField<MakeOpt(1), MakeOpt<int>()> folder_history_limit;

		ContentBrowserSettingsDesc(imtk::datapath_link link = {});
	};

#define TREE_VIEW_ADVANCED_SETTINGS_GENERATOR(M) \
		M(analysis_interval)

	struct TreeViewAdvancedSettingsDesc
	{
		DESCRIPTOR_BODY(TreeViewAdvancedSettingsDesc, TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);

		FloatField<MakeOpt(0.1f), MakeOpt<float>()> analysis_interval;

		TreeViewAdvancedSettingsDesc(imtk::datapath_link link = {});
		
		float AnalysisInterval() const;
	};

#define TREE_VIEW_SETTINGS_GENERATOR(M) \
		M(advanced)

	struct TreeViewSettingsDesc
	{
		DESCRIPTOR_BODY(TreeViewSettingsDesc, TREE_VIEW_SETTINGS_GENERATOR);

		TreeViewAdvancedSettingsDesc advanced;
		static const detail::Key advanced_key;

		TreeViewSettingsDesc(imtk::datapath_link link = {});
	};

#define FILESYSTEM_SETTINGS_GENERATOR(M) \
		M(trash_limit) \
		M(trash_limit_unit)

	struct FilesystemSettingsDesc
	{
		DESCRIPTOR_BODY(FilesystemSettingsDesc, FILESYSTEM_SETTINGS_GENERATOR);

		IntField<MakeOpt(1), MakeOpt<int>()> trash_limit;
		EnumField<MemoryUnit> trash_limit_unit;

		FilesystemSettingsDesc(imtk::datapath_link link = {});

		size_t TrashLimit() const;
	};

#define PREFERENCES_GENERATOR(M) \
		M(edit) \
		M(content_browser) \
		M(tree_view) \
		M(filesystem)

	struct PreferencesDesc
	{
		DESCRIPTOR_BODY(PreferencesDesc, PREFERENCES_GENERATOR);

		EditSettingsDesc edit;
		static const detail::Key edit_key;
		ContentBrowserSettingsDesc content_browser;
		static const detail::Key content_browser_key;
		TreeViewSettingsDesc tree_view;
		static const detail::Key tree_view_key;
		FilesystemSettingsDesc filesystem;
		static const detail::Key filesystem_key;

		PreferencesDesc(imtk::datapath_link link = {});
	};
}
