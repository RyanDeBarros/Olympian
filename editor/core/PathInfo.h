#pragma once

#include <filesystem>

namespace oly::editor
{
	struct PathInfo
	{
		static bool IsImportFile(const std::filesystem::path& path);
	};
}
