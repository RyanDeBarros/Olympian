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

	void TreeViewSettingsDesc::Reset(TreeViewSettingsDesc& source)
	{
		RESET_FIELD(advanced);
	}

	void TreeViewSettingsDesc::Isolate()
	{
		ISOLATE_FIELD(advanced);
	}

	void PreferencesDesc::Reset(PreferencesDesc& source)
	{
		RESET_FIELD(tree_view);
	}

	void PreferencesDesc::Isolate()
	{
		ISOLATE_FIELD(tree_view);
	}
}
