#pragma once

#include "desc/Fields.h"

namespace oly::editor
{
#define TREE_VIEW_ADVANCED_SETTINGS_GENERATOR(M) \
		M(analysis_interval)

	struct TreeViewAdvancedSettingsDesc
	{
		FloatField<MakeOpt(0.1f), MakeOpt<float>()> analysis_interval;

		TreeViewAdvancedSettingsDesc();

		DESC_CHAIN_METHODS(TreeViewAdvancedSettingsDesc, TREE_VIEW_ADVANCED_SETTINGS_GENERATOR);
	};

#define TREE_VIEW_SETTINGS_GENERATOR(M) \
		M(advanced)

	struct TreeViewSettingsDesc
	{
		TreeViewAdvancedSettingsDesc advanced;
		static const detail::Key advanced_key;

		DESC_CHAIN_METHODS(TreeViewSettingsDesc, TREE_VIEW_SETTINGS_GENERATOR);
	};

#define PREFERENCES_GENERATOR(M) \
		M(tree_view)

	struct PreferencesDesc
	{
		TreeViewSettingsDesc tree_view;
		static const detail::Key tree_view_key;

		DESC_CHAIN_METHODS(PreferencesDesc, PREFERENCES_GENERATOR);
	};
}
