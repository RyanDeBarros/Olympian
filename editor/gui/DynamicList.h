#pragma once

#include <imgui.h>

#include <optional>

namespace oly::editor::gui
{
	struct DynamicListState
	{
		size_t index = 0;

		void Clamp(size_t count);
		void SetLast(size_t count);
	};

	class DynamicRow
	{
		bool _visible = false;
		DynamicListState& _state;
		ImVec2 _cursor, _size;
		size_t _index;
		std::optional<size_t> _dropped_src;

	public:
		DynamicRow(size_t index, const char* str_id, DynamicListState& state);
		DynamicRow(const DynamicRow&) = delete;
		DynamicRow(DynamicRow&&) = delete;
		~DynamicRow();

		operator bool() const;

		std::optional<size_t> GetDroppedSource() const;
	};
}
