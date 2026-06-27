#include "PreferencesDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	UndoHistorySettingsDesc::UndoHistorySettingsDesc() :
		count_limit(500, detail::Key::UndoHistoryCountLimit, "Count limit"),
		size_limit(32, detail::Key::UndoHistorySizeLimit, "Size limit"),
		size_limit_unit(MemoryUnit::MiB, detail::Key::UndoHistorySizeLimitUnit, "Size unit")
	{
	}

	size_t UndoHistorySettingsDesc::CountLimit() const
	{
		return count_limit.value;
	}

	size_t UndoHistorySettingsDesc::SizeLimit() const
	{
		return MemorySize(size_limit.value, size_limit_unit.value);
	}

	const detail::Key EditSettingsDesc::undo_history_key = detail::Key::UndoHistory;

	TreeViewAdvancedSettingsDesc::TreeViewAdvancedSettingsDesc() :
		analysis_interval(10.f, detail::Key::AnalysisInterval, "Analysis interval")
	{
	}

	float TreeViewAdvancedSettingsDesc::AnalysisInterval() const
	{
		return analysis_interval.value;
	}

	const detail::Key TreeViewSettingsDesc::advanced_key = detail::Key::Advanced;

	const detail::Key PreferencesDesc::edit_key = detail::Key::Edit;
	const detail::Key PreferencesDesc::tree_view_key = detail::Key::TreeView;
}
