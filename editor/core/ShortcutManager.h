#pragma once

class ShortcutManager
{
public:
	static ShortcutManager& Instance();

	void PollShortcuts();
	void HandlePathDrop(int count, const char** paths);
};
