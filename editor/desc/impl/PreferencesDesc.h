#pragma once

#include "desc/Fields.h"

#include "core/MemoryUnit.h"

namespace oly::editor
{
#define UNDO_HISTORY_SETTINGS_GENERATOR(M) \
		M((IntField<1, imp::nullpotential>), count_limit) \
		M((IntField<1, imp::nullpotential>), size_limit) \
		M((EnumField<MemoryUnit>), size_limit_unit)

	struct undo_historySettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(undo_historySettingsDesc, UNDO_HISTORY_SETTINGS_GENERATOR);

		undo_historySettingsDesc(imtk::datapath_link link = {});

		size_t CountLimit() const;
		size_t SizeLimit() const;
	};

#define EDIT_SETTINGS_GENERATOR(M) \
		M((imtk::desc::sub<undo_historySettingsDesc>), undo_history)

	struct EditSettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(EditSettingsDesc, EDIT_SETTINGS_GENERATOR);

		EditSettingsDesc(imtk::datapath_link link = {});
	};

#define CONTENT_BROWSER_SETTINGS_GENERATOR(M) \
		M((IntField<1, imp::nullpotential>), folder_history_limit)

	struct ContentBrowserSettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(ContentBrowserSettingsDesc, CONTENT_BROWSER_SETTINGS_GENERATOR);

		ContentBrowserSettingsDesc(imtk::datapath_link link = {});
	};

#define TREE_VIEW_ADVANCED_SETTINGS_GENERATOR(M) \
		M((FloatField<0.1f, imp::nullpotential>), analysis_interval)

	struct TreeViewAdvancedSettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(TreeViewAdvancedSettingsDesc, TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);

		TreeViewAdvancedSettingsDesc(imtk::datapath_link link = {});
		
		float AnalysisInterval() const;
	};

#define TREE_VIEW_SETTINGS_GENERATOR(M) \
		M((imtk::desc::sub<TreeViewAdvancedSettingsDesc>), advanced)

	struct TreeViewSettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(TreeViewSettingsDesc, TREE_VIEW_SETTINGS_GENERATOR);

		TreeViewSettingsDesc(imtk::datapath_link link = {});
	};

#define FILESYSTEM_SETTINGS_GENERATOR(M) \
		M((IntField<1, imp::nullpotential>), trash_limit) \
		M((EnumField<MemoryUnit>), trash_limit_unit)

	struct FilesystemSettingsDesc
	{
		IMTK_DESCRIPTOR_BODY(FilesystemSettingsDesc, FILESYSTEM_SETTINGS_GENERATOR);

		FilesystemSettingsDesc(imtk::datapath_link link = {});

		size_t TrashLimit() const;
	};

#define PREFERENCES_GENERATOR(M) \
		M((imtk::desc::sub<EditSettingsDesc>), edit) \
		M((imtk::desc::sub<ContentBrowserSettingsDesc>), content_browser) \
		M((imtk::desc::sub<TreeViewSettingsDesc>), tree_view) \
		M((imtk::desc::sub<FilesystemSettingsDesc>), filesystem)

	struct PreferencesDesc
	{
		IMTK_DESCRIPTOR_BODY(PreferencesDesc, PREFERENCES_GENERATOR);

		PreferencesDesc(imtk::datapath_link link = {});
	};
}
