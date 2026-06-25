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

		DrawResult(bool dirty = false);

		DrawResult& Query();

		operator bool() const;
		DrawResult operator|(const DrawResult& o);
		DrawResult& operator|=(const DrawResult& o);

		void SetDirty(bool dirty);
		bool IsDirty() const;
		bool IsHovered() const;
		bool IsLeftClicked() const;
		bool IsRightClicked() const;
		bool IsClicked() const;
		bool IsFocused() const;
		bool IsDeactivatedAfterEdit() const;
	};

	OLY_DETAIL_IMPLEMENT_BITMASK(DrawResult::Flags);
}
