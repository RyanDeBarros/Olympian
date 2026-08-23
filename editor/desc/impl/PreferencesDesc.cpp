#include "PreferencesDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	undo_historySettingsDesc::undo_historySettingsDesc(imtk::datapath_link link) :
		link(std::move(link)),
		count_limit(IMTK_DATAPATH_SUBLINK(subpaths.count_limit), 500, detail::Key::UndoHistoryCountLimit, "Count limit"),
		size_limit(IMTK_DATAPATH_SUBLINK(subpaths.size_limit), 32, detail::Key::UndoHistorySizeLimit, "Size limit"),
		size_limit_unit(IMTK_DATAPATH_SUBLINK(subpaths.size_limit_unit), MemoryUnit::MiB, detail::Key::UndoHistorySizeLimitUnit, "Size unit")
	{
	}

	size_t undo_historySettingsDesc::CountLimit() const
	{
		return count_limit.value;
	}

	size_t undo_historySettingsDesc::SizeLimit() const
	{
		return MemorySize(size_limit.value, size_limit_unit.value);
	}

	EditSettingsDesc::EditSettingsDesc(imtk::datapath_link link) :
		link(std::move(link)),
		undo_history(detail::Key::UndoHistory, IMTK_DATAPATH_SUBLINK(subpaths.undo_history))
	{
	}

	ContentBrowserSettingsDesc::ContentBrowserSettingsDesc(imtk::datapath_link link) :
		link(std::move(link)),
		folder_history_limit(IMTK_DATAPATH_SUBLINK(subpaths.folder_history_limit), 30, detail::Key::FolderHistoryLimit, "Folder history limit")
	{
	}

	TreeViewAdvancedSettingsDesc::TreeViewAdvancedSettingsDesc(imtk::datapath_link link) :
		link(std::move(link)),
		analysis_interval(IMTK_DATAPATH_SUBLINK(subpaths.analysis_interval), 10.f, detail::Key::AnalysisInterval, "Analysis interval")
	{
	}

	float TreeViewAdvancedSettingsDesc::AnalysisInterval() const
	{
		return analysis_interval.value;
	}

	TreeViewSettingsDesc::TreeViewSettingsDesc(imtk::datapath_link link) :
		link(std::move(link)),
		advanced(detail::Key::Advanced, IMTK_DATAPATH_SUBLINK(subpaths.advanced))
	{
	}

	FilesystemSettingsDesc::FilesystemSettingsDesc(imtk::datapath_link link) :
		link(std::move(link)),
		trash_limit(IMTK_DATAPATH_SUBLINK(subpaths.trash_limit), 5, detail::Key::TrashLimit, "Trash limit"),
		trash_limit_unit(IMTK_DATAPATH_SUBLINK(subpaths.trash_limit_unit), MemoryUnit::GiB, detail::Key::TrashLimitUnit, "Trash limit unit")
	{
	}

	size_t FilesystemSettingsDesc::TrashLimit() const
	{
		return MemorySize(trash_limit.value, trash_limit_unit.value);
	}

	PreferencesDesc::PreferencesDesc(imtk::datapath_link link) :
		link(std::move(link)),
		edit(detail::Key::Edit, IMTK_DATAPATH_SUBLINK(subpaths.edit)),
		content_browser(detail::Key::ContentBrowser, IMTK_DATAPATH_SUBLINK(subpaths.content_browser)),
		tree_view(detail::Key::TreeView, IMTK_DATAPATH_SUBLINK(subpaths.tree_view)),
		filesystem(detail::Key::Filesystem, IMTK_DATAPATH_SUBLINK(subpaths.filesystem))
	{
	}
}
