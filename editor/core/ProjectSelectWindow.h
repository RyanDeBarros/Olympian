#pragma once

#include <string>

namespace oly::editor
{
	class ProjectSelectWindow
	{
		std::string _project_folder;
		bool _valid_project_folder = false;

	public:
		void Open();
		void Draw();

	private:
		void DrawOpenExistingGroup();

		void CheckProjectFolder();
		void OpenProject();
	};
}
