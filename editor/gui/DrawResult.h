#pragma once

#include "util/Macros.h"

#include <imgui.h>

namespace oly::editor
{
	struct DrawResult
	{
		enum Flags
		{
			None = 0,
			Dirty = 1,
			Hovered = 1 << 1,
			LeftClicked = 1 << 2,
			RightClicked = 1 << 3,
			Focused = 1 << 4,
			DeactivatedAfterEdit = 1 << 5
		};

		OLY_DETAIL_DECLARE_NESTED_BITMASK(Flags);

		Flags flags = Flags::None;

		DrawResult(bool dirty = false)
		{
			SetDirty(dirty);
		}

		DrawResult& Query()
		{
			if (ImGui::IsItemHovered())
				flags |= Flags::Hovered;
			
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				flags |= Flags::LeftClicked;
			
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				flags |= Flags::RightClicked;

			if (ImGui::IsItemFocused())
				flags |= Flags::Focused;

			if (ImGui::IsItemDeactivatedAfterEdit())
				flags |= Flags::DeactivatedAfterEdit;

			return *this;
		}

		operator bool() const
		{
			return flags & Flags::Dirty;
		}

		DrawResult& operator|=(const DrawResult& result)
		{
			flags |= result.flags;
			return *this;
		}

		void SetDirty(bool dirty)
		{
			if (dirty)
				flags |= Flags::Dirty;
		}

		bool IsDirty() const
		{
			return flags & Flags::Dirty;
		}

		bool IsHovered() const
		{
			return flags & Flags::Hovered;
		}

		bool IsLeftClicked() const
		{
			return flags & Flags::LeftClicked;
		}

		bool IsRightClicked() const
		{
			return flags & Flags::RightClicked;
		}

		bool IsClicked() const
		{
			return IsLeftClicked() || IsRightClicked();
		}

		bool IsFocused() const
		{
			return flags & Flags::Focused;
		}

		bool IsDeactivatedAfterEdit() const
		{
			return flags & Flags::DeactivatedAfterEdit;
		}
	};

	OLY_DETAIL_IMPLEMENT_BITMASK(DrawResult::Flags);
}
