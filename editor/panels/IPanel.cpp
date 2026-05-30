#include "IPanel.h"

void IPanel::Open()
{
	_open = true;
}

void IPanel::Close()
{
	_open = false;
}

bool IPanel::IsOpen() const
{
	return _open;
}
