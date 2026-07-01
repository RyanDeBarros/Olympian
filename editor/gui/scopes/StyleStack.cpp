#include "StyleStack.h"

namespace oly::editor::gui
{
	StyleColor::StyleColor(ImGuiCol idx, ImU32 col)
	{
		ImGui::PushStyleColor(idx, col);
		_active = true;
	}

	StyleColor::StyleColor(StyleColorCtor ctor)
	{
		ImGui::PushStyleColor(ctor.idx, ctor.col);
		_active = true;
	}

	StyleColor::StyleColor(StyleColor&& o) noexcept
		: _active(o._active)
	{
		o._active = false;
	}
	
	StyleColor::~StyleColor()
	{
		if (_active)
			ImGui::PopStyleColor();
	}
	
	StyleColor& StyleColor::operator=(StyleColor&& o) noexcept
	{
		if (this != &o)
		{
			if (_active)
				ImGui::PopStyleColor();
			_active = o._active;
			o._active = false;
		}
		return *this;
	}

	StyleVar1D::StyleVar1D(ImGuiStyleVar idx, float value)
	{
		ImGui::PushStyleVar(idx, value);
		_active = true;
	}

	StyleVar1D::StyleVar1D(StyleVar1DCtor ctor)
	{
		ImGui::PushStyleVar(ctor.idx, ctor.value);
		_active = true;
	}

	StyleVar1D::StyleVar1D(StyleVar1D&& o) noexcept
		: _active(o._active)
	{
		o._active = false;
	}

	StyleVar1D::~StyleVar1D()
	{
		if (_active)
			ImGui::PopStyleVar();
	}

	StyleVar1D& StyleVar1D::operator=(StyleVar1D&& o) noexcept
	{
		if (this != &o)
		{
			if (_active)
				ImGui::PopStyleVar();
			_active = o._active;
			o._active = false;
		}
		return *this;
	}

	StyleVar2D::StyleVar2D(ImGuiStyleVar idx, ImVec2 value)
	{
		ImGui::PushStyleVar(idx, value);
		_active = true;
	}

	StyleVar2D::StyleVar2D(StyleVar2DCtor ctor)
	{
		ImGui::PushStyleVar(ctor.idx, ctor.value);
		_active = true;
	}

	StyleVar2D::StyleVar2D(StyleVar2D&& o) noexcept
		: _active(o._active)
	{
		o._active = false;
	}

	StyleVar2D::~StyleVar2D()
	{
		if (_active)
			ImGui::PopStyleVar();
	}

	StyleVar2D& StyleVar2D::operator=(StyleVar2D&& o) noexcept
	{
		if (this != &o)
		{
			if (_active)
				ImGui::PopStyleVar();
			_active = o._active;
			o._active = false;
		}
		return *this;
	}

	std::vector<StyleVariant> ApplyStyles(std::vector<StyleCtorVariant>& ctors)
	{
		std::vector<StyleVariant> styles;
		styles.reserve(ctors.size());
		for (const auto& ctor : ctors)
		{
			std::visit([&styles](auto&& ctor) {
				using C = std::decay_t<decltype(ctor)>;
				if constexpr (std::is_same_v<C, StyleColorCtor>)
					styles.push_back(StyleColor(ctor));

				if constexpr (std::is_same_v<C, StyleVar1DCtor>)
					styles.push_back(StyleVar1D(ctor));

				if constexpr (std::is_same_v<C, StyleVar2DCtor>)
					styles.push_back(StyleVar2D(ctor));
			}, ctor);
		}
		return styles;
	}
}
