#pragma once

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
			RightDoubleClicked = 1 << 5
		};

		Flags flags = Flags::None;

		friend Flags operator|(Flags lhs, Flags rhs);
		friend Flags operator&(Flags lhs, Flags rhs);
		friend Flags operator^(Flags lhs, Flags rhs);
		friend Flags operator~(Flags value);
		friend Flags& operator|=(Flags& lhs, Flags rhs);
		friend Flags& operator&=(Flags& lhs, Flags rhs);
		friend Flags& operator^=(Flags& lhs, Flags rhs);

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
	};

	// TODO v8 macro for bitmask enums
	inline DrawResult::Flags operator|(DrawResult::Flags lhs, DrawResult::Flags rhs)
	{
		return static_cast<DrawResult::Flags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	inline DrawResult::Flags operator&(DrawResult::Flags lhs, DrawResult::Flags rhs)
	{
		return static_cast<DrawResult::Flags>(static_cast<int>(lhs) & static_cast<int>(rhs));
	}

	inline DrawResult::Flags operator^(DrawResult::Flags lhs, DrawResult::Flags rhs)
	{
		return static_cast<DrawResult::Flags>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
	}

	inline DrawResult::Flags operator~(DrawResult::Flags value)
	{
		return static_cast<DrawResult::Flags>(~static_cast<int>(value));
	}

	inline DrawResult::Flags& operator|=(DrawResult::Flags& lhs, DrawResult::Flags rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	inline DrawResult::Flags& operator&=(DrawResult::Flags& lhs, DrawResult::Flags rhs)
	{
		lhs = lhs & rhs;
		return lhs;
	}

	inline DrawResult::Flags& operator^=(DrawResult::Flags& lhs, DrawResult::Flags rhs)
	{
		lhs = lhs ^ rhs;
		return lhs;
	}
}
