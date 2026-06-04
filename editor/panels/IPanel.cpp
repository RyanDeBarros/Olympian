#include "IPanel.h"

namespace oly::editor
{
	DrawDockedWindowImpl::DrawDockedWindowImpl(bool call_end, bool visible)
		: _call_end(call_end), _visible(visible)
	{
	}

	DrawDockedWindowImpl::~DrawDockedWindowImpl()
	{
		if (_call_end)
			ImGui::End();
	}

	DrawDockedWindowImpl::operator bool() const
	{
		return _visible;
	}

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

	DrawDockedWindowImpl IPanel::DrawDockedWindow(ImGuiWindowFlags flags)
	{
		if (_open)
		{
			bool visible = ImGui::Begin(GetTitle(), &_open, flags);
			return DrawDockedWindowImpl(true, visible);
		}
		else
			return DrawDockedWindowImpl(false, false);
	}
}
