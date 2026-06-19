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
			LeftDoubleClicked = 1 << 4,
			RightDoubleClicked = 1 << 5,
			Focused = 1 << 6
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
			
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				flags |= Flags::LeftClicked;
			
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				flags |= Flags::RightClicked;

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				flags |= Flags::LeftDoubleClicked;

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right))
				flags |= Flags::RightDoubleClicked;

			if (ImGui::IsItemFocused())
				flags |= Flags::Focused;

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

		bool IsLeftDoubleClicked() const
		{
			return flags & Flags::LeftDoubleClicked;
		}

		bool IsRightDoubleClicked() const
		{
			return flags & Flags::RightDoubleClicked;
		}

		bool IsClicked() const
		{
			return IsLeftClicked() || IsRightClicked() || IsLeftDoubleClicked() || IsRightDoubleClicked();
		}

		bool IsFocused() const
		{
			return flags & Flags::Focused;
		}
	};

	OLY_DETAIL_IMPLEMENT_BITMASK(DrawResult::Flags);
}
