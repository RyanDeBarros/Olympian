#include "ProjectInfo.h"

#include "core/Editor.h"

#include "assets/ResourcePath.h"

namespace oly::editor
{
	ProjectInfo& ProjectInfo::Instance()
	{
		return Editor::Instance().GetProjectInfo();
	}

	const char* ProjectInfo::GetVersion()
	{
		return "1.0";
	}

	void ProjectInfo::Init(const std::filesystem::path& project_file)
	{
		_project_file = project_file;
		oly::detail::ResourcePath::set_resource_root(ResourceRoot());
	}

	std::string ProjectInfo::ProjectName() const
	{
		return ProjectPath().stem().generic_string();
	}

	std::filesystem::path ProjectInfo::ProjectPath() const
	{
		return _project_file;
	}

	std::filesystem::path ProjectInfo::ProjectRoot() const
	{
		return ProjectPath().parent_path();
	}

	std::filesystem::path ProjectInfo::EditorRoot() const
	{
		return ProjectRoot() / ".editor/";
	}

	std::filesystem::path ProjectInfo::ResourceRoot() const
	{
		return ProjectRoot() / "res/";
	}

	std::filesystem::path ProjectInfo::SourceRoot() const
	{
		return ProjectRoot() / "src/";
	}

	std::filesystem::path ProjectInfo::GenSourceRoot() const
	{
		return SourceRoot() / ".gen/";
	}
}
