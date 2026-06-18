#pragma once

#include <string>

namespace oly::editor
{
	class ProjectSelectWindow
	{
		std::string _project_file;
		bool _valid_project_file = false;

	public:
		void Open();
		void Draw();

	private:
		void DrawOpenExistingGroup();

		void CheckProjectFile();
		void OpenProject();
	};
}
