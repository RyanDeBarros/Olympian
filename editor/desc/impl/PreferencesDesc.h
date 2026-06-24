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
		IntField<MakeOpt(1), MakeOpt<int>()> count_limit;
		IntField<MakeOpt(1), MakeOpt<int>()> size_limit;
		EnumField<MemoryUnit> size_limit_unit;

		UndoHistorySettingsDesc();

		size_t CountLimit() const;
		size_t SizeLimit() const;
	};

	struct EditSettingsDesc
	{
		UndoHistorySettingsDesc undo_history;
		static const detail::Key undo_history_key;
	};

#define TREE_VIEW_ADVANCED_SETTINGS_GENERATOR(M) \
		M(analysis_interval)

	struct TreeViewAdvancedSettingsDesc
	{
		FloatField<MakeOpt(0.1f), MakeOpt<float>()> analysis_interval;

		TreeViewAdvancedSettingsDesc();

		float AnalysisInterval() const;
	};

#define TREE_VIEW_SETTINGS_GENERATOR(M) \
		M(advanced)

	struct TreeViewSettingsDesc
	{
		TreeViewAdvancedSettingsDesc advanced;
		static const detail::Key advanced_key;
	};

#define PREFERENCES_GENERATOR(M) \
		M(tree_view)

	struct PreferencesDesc
	{
		EditSettingsDesc edit;
		static const detail::Key edit_key;
		TreeViewSettingsDesc tree_view;
		static const detail::Key tree_view_key;
	};
}
