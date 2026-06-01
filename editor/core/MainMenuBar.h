#pragma once

namespace oly::editor
{
	class MainMenuBar
	{
	public:
		void Init();
		void Draw();

	private:
		void DrawFileMenu();

	public:
		void OpenFile();
	};
}
