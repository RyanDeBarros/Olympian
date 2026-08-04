#include "IPanel.h"

namespace oly::editor
{
	DrawDockedWindowImpl::DrawDockedWindowImpl(std::unique_ptr<imtk::window>&& window, bool request_close)
		: _window(std::move(window)), _request_close(request_close)
	{
	}

	bool DrawDockedWindowImpl::IsVisible() const
	{
		return _window && *_window;
	}

	bool DrawDockedWindowImpl::RequestsClose() const
	{
		return _request_close;
	}

	IPanel* IPanel::_gain_next = nullptr;

	void IPanel::Init()
	{
		InitImpl();
	}

	void IPanel::Terminate()
	{
		TerminateImpl();
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

	void IPanel::ToggleOpen()
	{
		_open = !_open;
	}

	void IPanel::GainFocus()
	{
		_gain_next = this;
	}

	DrawDockedWindowImpl IPanel::DrawDockedWindow(ImGuiWindowFlags flags)
	{
		if (_open)
		{
			auto window = std::make_unique<imtk::window>(GetTitle(), flags, &_open);
			
			if (_gain_next == this)
			{
				if (window)
					ImGui::SetWindowFocus();
				_gain_next = nullptr;
			}

			return DrawDockedWindowImpl(std::move(window), !_open);
		}
		else
		{
			if (_gain_next == this)
				_gain_next = nullptr;

			return DrawDockedWindowImpl(nullptr, false);
		}
	}
}
