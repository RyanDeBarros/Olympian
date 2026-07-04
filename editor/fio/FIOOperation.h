#pragma once

#include "core/UndoHistory.h"

#include <filesystem>

namespace oly::editor::fio
{
	struct RenamePathAction : public UndoAction
	{
		std::filesystem::path old_path;
		std::filesystem::path new_path;

		bool Forward() override;
		bool Backward() override;
		size_t EmpiricalSize() const override;
	};

	// TODO v9.2 use .editor/.trash folder to store deleted files/folders. Keep a manifest file in .editor/.trash to track the timeline of added files/folders-of-files so that a maximum trash size can be enforced (+ clear trash button in advanced settings). Use local folder structure of res/ in .trash, but paths should be folders with version-number files. For example, res/a/b.txt -> .trash/a/b.txt/1
}
