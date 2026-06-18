#pragma once

namespace oly::editor
{
	class ShortcutManager
	{
	public:
		static ShortcutManager& Instance();

		void PollShortcuts();
		void HandlePathDrop(int count, const char** paths);
	};
}
