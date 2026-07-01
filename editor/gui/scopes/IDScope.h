#pragma once

namespace oly::editor::gui
{
	class IDScope
	{
		int _depth = 0;

	public:
		IDScope() = default;
		IDScope(const void* ptr_id);
		IDScope(int int_id);
		IDScope(const char* str_id);

		IDScope(const IDScope&) = delete;
		IDScope(IDScope&& o) noexcept;
		~IDScope();
		IDScope& operator=(IDScope&& o) noexcept;

		operator bool() const;

		IDScope& Push(const void* ptr_id);
		IDScope& Push(int int_id);
		IDScope& Push(const char* str_id);
		void Pop();
		void PopAll();
	};
}
