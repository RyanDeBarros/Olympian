#pragma once

#include "desc/Fields.h"

namespace oly::editor
{
	struct TreeViewAdvancedSettingsDesc
	{
		FloatField<MakeOpt(0.1f), MakeOpt<float>()> analysis_interval;

		TreeViewAdvancedSettingsDesc();

		void Reset(TreeViewAdvancedSettingsDesc& source);
		void Isolate();
	};

#define TREE_NODE_ADVANCED_SETTINGS_GENERATOR(M) \
	M(analysis_interval)

	struct TreeViewSettingsDesc
	{
		TreeViewAdvancedSettingsDesc advanced;

		void Reset(TreeViewSettingsDesc& source);
		void Isolate();
	};

	struct PreferencesDesc
	{
		TreeViewSettingsDesc tree_view;

		void Reset(PreferencesDesc& source);
		void Isolate();
	};
}
