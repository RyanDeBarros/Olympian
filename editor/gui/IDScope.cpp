#include "IDScope.h"

#include <imgui.h>

namespace oly::editor::gui
{
	IDScope::IDScope(const void* ptr_id)
	{
		Push(ptr_id);
	}
	
	IDScope::IDScope(int int_id)
	{
		Push(int_id);
	}
	
	IDScope::IDScope(const char* str_id)
	{
		Push(str_id);
	}

	IDScope::IDScope(IDScope&& o) noexcept
		: _depth(o._depth)
	{
		o._depth = 0;
	}
	
	IDScope::~IDScope()
	{
		PopAll();
	}
	
	IDScope& IDScope::operator=(IDScope&& o) noexcept
	{
		if (this != &o)
		{
			PopAll();
			_depth = o._depth;
			o._depth = 0;
		}
		return *this;
	}

	IDScope::operator bool() const
	{
		return _depth > 0;
	}

	IDScope& IDScope::Push(const void* ptr_id)
	{
		ImGui::PushID(ptr_id);
		++_depth;
		return *this;
	}
	
	IDScope& IDScope::Push(int int_id)
	{
		ImGui::PushID(int_id);
		++_depth;
		return *this;
	}
	
	IDScope& IDScope::Push(const char* str_id)
	{
		ImGui::PushID(str_id);
		++_depth;
		return *this;
	}

	void IDScope::Pop()
	{
		ImGui::PopID();
		--_depth;
	}
	
	void IDScope::PopAll()
	{
		while (_depth > 0)
			Pop();
	}
}
