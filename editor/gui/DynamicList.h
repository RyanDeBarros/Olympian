#pragma once

#include <imgui.h>

#include <optional>

namespace oly::editor::gui
{
	struct DynamicListState
	{
		const DynamicListState* const self;
		size_t index = 0;

		DynamicListState();
		DynamicListState(const DynamicListState& o);
		DynamicListState(DynamicListState&& o) noexcept;
		DynamicListState& operator=(const DynamicListState& o);
		DynamicListState& operator=(DynamicListState&& o) noexcept;

		void Clamp(size_t count);
		void SetLast(size_t count);
		void SendPayload(const char* type);
	};

	class DynamicRow
	{
		bool _visible = false;
		ImVec2 _cursor, _size;
		DynamicListState& _state;
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
