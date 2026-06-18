#pragma once

#include <filesystem>

namespace oly::editor
{
	class ProjectInfo
	{
		std::filesystem::path _project_file;

	public:
		static ProjectInfo& Instance();

		static const char* GetVersion();

		void Init(const std::filesystem::path& project_file);

		std::string ProjectName() const;
		std::filesystem::path ProjectPath() const;
		std::filesystem::path ProjectRoot() const;
		std::filesystem::path EditorRoot() const;
		std::filesystem::path ResourceRoot() const;
		std::filesystem::path SourceRoot() const;
		std::filesystem::path GenSourceRoot() const;
	};
}
