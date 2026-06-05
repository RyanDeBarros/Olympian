#include "IPanel.h"

namespace oly::editor
{
	DrawDockedWindowImpl::DrawDockedWindowImpl(bool call_end, bool visible, bool request_close)
		: _call_end(call_end), _visible(visible), _request_close(request_close)
	{
	}

	DrawDockedWindowImpl::~DrawDockedWindowImpl()
	{
		if (_call_end)
			ImGui::End();
	}

	bool DrawDockedWindowImpl::IsVisible() const
	{
		return _visible;
	}

	bool DrawDockedWindowImpl::RequestsClose() const
	{
		return _request_close;
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
			return DrawDockedWindowImpl(true, visible, !_open);
		}
		else
			return DrawDockedWindowImpl(false, false, false);
	}
}
