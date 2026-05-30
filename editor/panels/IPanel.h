#pragma once

class IPanel
{
	bool _open = false;

public:
	virtual ~IPanel() = default;
	
	virtual const char* GetTitle() const = 0;
	virtual void Draw() = 0;

	void Open();
	void Close();
	bool IsOpen() const;
};
