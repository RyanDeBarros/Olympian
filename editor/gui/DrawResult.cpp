#include "DrawResult.h"

namespace oly::editor
{
	DrawResult::DrawResult(bool dirty)
	{
		SetDirty(dirty);
	}

	DrawResult& DrawResult::Query()
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			flags |= Flags::Hovered;

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			flags |= Flags::LeftClicked;

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			flags |= Flags::RightClicked;

		if (ImGui::IsItemFocused())
			flags |= Flags::Focused;

		if (ImGui::IsItemActivated())
			flags |= Flags::Activated;

		if (ImGui::IsItemDeactivatedAfterEdit())
			flags |= Flags::DeactivatedAfterEdit;

		return *this;
	}

	DrawResult::operator bool() const
	{
		return IsDirty();
	}

	DrawResult DrawResult::operator|(const DrawResult& o)
	{
		DrawResult result = *this;
		result |= o;
		return result;
	}

	DrawResult& DrawResult::operator|=(const DrawResult& o)
	{
		flags |= o.flags;
		return *this;
	}

	void DrawResult::SetDirty(bool dirty)
	{
		if (dirty)
			flags |= Flags::Dirty;
	}

	bool DrawResult::IsDirty() const
	{
		return flags & Flags::Dirty;
	}

	bool DrawResult::IsHovered() const
	{
		return flags & Flags::Hovered;
	}

	bool DrawResult::IsLeftClicked() const
	{
		return flags & Flags::LeftClicked;
	}

	bool DrawResult::IsRightClicked() const
	{
		return flags & Flags::RightClicked;
	}

	bool DrawResult::IsClicked() const
	{
		return IsLeftClicked() || IsRightClicked();
	}

	bool DrawResult::IsFocused() const
	{
		return flags & Flags::Focused;
	}

	bool DrawResult::IsActivated() const
	{
		return flags & Flags::Activated;
	}

	bool DrawResult::IsDeactivatedAfterEdit() const
	{
		return flags & Flags::DeactivatedAfterEdit;
	}
}
