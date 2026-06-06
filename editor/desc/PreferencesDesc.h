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
		static const detail::Key advanced_key;

		void Reset(TreeViewSettingsDesc& source);
		void Isolate();
	};

	struct PreferencesDesc
	{
		TreeViewSettingsDesc tree_view;
		static const detail::Key tree_view_key;

		void Reset(PreferencesDesc& source);
		void Isolate();
	};
}
