#pragma once

#include <imgui.h>

#include <variant>
#include <vector>

namespace oly::editor::gui
{
	struct StyleColorCtor
	{
		ImGuiCol idx;
		ImU32 col;
	};

	class StyleColor
	{
		bool _active = false;

	public:
		StyleColor(ImGuiCol idx, ImU32 col);
		StyleColor(StyleColorCtor ctor);
		StyleColor(const StyleColor&) = delete;
		StyleColor(StyleColor&&) noexcept;
		~StyleColor();
		StyleColor& operator=(StyleColor&&) noexcept;
	};

	struct StyleVar1DCtor
	{
		ImGuiStyleVar idx;
		float value;
	};

	class StyleVar1D
	{
		bool _active = false;

	public:
		StyleVar1D(ImGuiStyleVar idx, float value);
		StyleVar1D(StyleVar1DCtor ctor);
		StyleVar1D(const StyleVar1D&) = delete;
		StyleVar1D(StyleVar1D&&) noexcept;
		~StyleVar1D();
		StyleVar1D& operator=(StyleVar1D&&) noexcept;
	};

	struct StyleVar2DCtor
	{
		ImGuiStyleVar idx;
		ImVec2 value;
	};

	class StyleVar2D
	{
		bool _active = false;

	public:
		StyleVar2D(ImGuiStyleVar idx, ImVec2 value);
		StyleVar2D(StyleVar2DCtor ctor);
		StyleVar2D(const StyleVar2D&) = delete;
		StyleVar2D(StyleVar2D&&) noexcept;
		~StyleVar2D();
		StyleVar2D& operator=(StyleVar2D&&) noexcept;
	};

	using StyleCtorVariant = std::variant<StyleColorCtor, StyleVar1DCtor, StyleVar2DCtor>;
	using StyleVariant = std::variant<StyleColor, StyleVar1D, StyleVar2D>;

	extern std::vector<StyleVariant> ApplyStyles(std::vector<StyleCtorVariant>& ctors);
}
