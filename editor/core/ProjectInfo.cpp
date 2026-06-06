#include "ProjectInfo.h"

#include "core/Editor.h"

#include "assets/ResourcePath.h"

namespace oly::editor
{
	ProjectInfo& ProjectInfo::Instance()
	{
		return Editor::Instance().GetProjectInfo();
	}

	void ProjectInfo::Init(const std::filesystem::path& project_root)
	{
		_project_root = project_root;
		oly::detail::ResourcePath::set_resource_root(ResourceRoot());
	}

	std::string ProjectInfo::ProjectName() const
	{
		// TODO v8 this should return the filename of project file. Store _project_file instead of _project_root
		return _project_root.filename().generic_string();
	}

	std::filesystem::path ProjectInfo::ProjectRoot() const
	{
		return _project_root;
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
