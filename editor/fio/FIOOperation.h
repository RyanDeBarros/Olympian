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
}
