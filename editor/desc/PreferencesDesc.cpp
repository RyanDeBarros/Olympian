#include "PreferencesDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	TreeViewAdvancedSettingsDesc::TreeViewAdvancedSettingsDesc() :
		analysis_interval(10.f, detail::Key::AnalysisInterval, "Analysis Interval")
	{
	}

	const detail::Key TreeViewSettingsDesc::advanced_key = detail::Key::Advanced;

	const detail::Key PreferencesDesc::tree_view_key = detail::Key::TreeView;
}
