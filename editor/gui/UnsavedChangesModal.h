#pragma once

#include <string>
#include <vector>

// TODO v9.3 move to imtk, then put gui::Widgets into a folder like imtk_import/ and remove gui namespace folder.
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
