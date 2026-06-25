#pragma once

#include <string>
#include <vector>

namespace oly::editor::gui
{
	enum class UnsavedChangesModalResult
	{
		None,
		SaveChanges,
		DiscardChanges,
		CancelClose
	};

	extern UnsavedChangesModalResult DrawUnsavedChangesModal(const char* popup, std::vector<std::string>& description);
}
