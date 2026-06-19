#pragma once

#include <imgui.h>

namespace oly::editor::gui
{
	class Outline
	{
		ImVec2 _start_pos;
		ImU32 _color;
		float _border;
		bool _active = false;

	public:
		Outline(ImU32 color, float border = 1.f);
		Outline(const Outline&) = delete;
		Outline(Outline&&) noexcept;
		~Outline();
		Outline& operator=(Outline&&) noexcept;

		operator bool() const;
		void Draw() const;
	};
}
