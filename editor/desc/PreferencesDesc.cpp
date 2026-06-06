#include "PreferencesDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	TreeViewAdvancedSettingsDesc::TreeViewAdvancedSettingsDesc() :
		analysis_interval(10.f, detail::Key::AnalysisInterval, "Analysis Interval")
	{
	}

	void TreeViewAdvancedSettingsDesc::Reset(TreeViewAdvancedSettingsDesc& source)
	{
		RESET_FIELDS(TREE_NODE_ADVANCED_SETTINGS_GENERATOR);
	}

	void TreeViewAdvancedSettingsDesc::Isolate()
	{
		ISOLATE_FIELDS(TREE_NODE_ADVANCED_SETTINGS_GENERATOR);
	}

	const detail::Key TreeViewSettingsDesc::advanced_key = detail::Key::Advanced;

	void TreeViewSettingsDesc::Reset(TreeViewSettingsDesc& source)
	{
		RESET_FIELD(advanced);
	}

	void TreeViewSettingsDesc::Isolate()
	{
		ISOLATE_FIELD(advanced);
	}

	const detail::Key PreferencesDesc::tree_view_key = detail::Key::TreeView;

	void PreferencesDesc::Reset(PreferencesDesc& source)
	{
		RESET_FIELD(tree_view);
	}

	void PreferencesDesc::Isolate()
	{
		ISOLATE_FIELD(tree_view);
	}
}
