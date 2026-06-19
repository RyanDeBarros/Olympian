#include "Outline.h"

namespace oly::editor::gui
{
	Outline::Outline(ImU32 color, float border)
		: _start_pos(ImGui::GetCursorScreenPos()), _color(color), _border(border), _active(true)
	{
	}

	Outline::Outline(Outline&& o) noexcept
		: _start_pos(o._start_pos), _color(o._color), _border(o._border), _active(o._active)
	{
		o._active = false;
	}

	Outline::~Outline()
	{
		Consume();
	}

	Outline& Outline::operator=(Outline&& o) noexcept
	{
		if (this != &o)
		{
			Consume();
			_start_pos = o._start_pos;
			_color = o._color;
			_border = o._border;
			_active = o._active;
			o._active = false;
		}

		return *this;
	}

	Outline::operator bool() const
	{
		return _active;
	}

	void Outline::Draw() const
	{
		if (_active)
		{
			ImVec2 end_pos(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y + ImGui::GetFrameHeight());
			ImGui::GetWindowDrawList()->AddRect(_start_pos, end_pos, _color, 0.f, 0, _border);
		}
	}

	void Outline::Consume()
	{
		Draw();
		Cancel();
	}

	void Outline::Cancel()
	{
		_active = false;
	}
}
