#pragma once

#include <filesystem>

class ProjectInfo
{
	std::filesystem::path _project_root;

public:
	static ProjectInfo& Instance();

	void Init(const std::filesystem::path& project_root);

	std::filesystem::path ProjectRoot() const;
	std::filesystem::path EditorRoot() const;
	std::filesystem::path ResourceRoot() const;
	std::filesystem::path SourceRoot() const;
	std::filesystem::path GenSourceRoot() const;
};
