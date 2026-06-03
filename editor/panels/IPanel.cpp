#include "IPanel.h"

namespace oly::editor
{
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
}
