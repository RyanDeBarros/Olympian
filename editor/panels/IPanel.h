#pragma once

#include <imgui.h>

namespace oly::editor
{
	class DrawDockedWindowImpl
	{
		bool _call_end;
		bool _visible;

	public:
		DrawDockedWindowImpl(bool call_end, bool visible);
		DrawDockedWindowImpl(const DrawDockedWindowImpl&) = delete;
		DrawDockedWindowImpl(DrawDockedWindowImpl&&) = delete;
		~DrawDockedWindowImpl();

		operator bool() const;
	};

	class IPanel
	{
		bool _open = false;

	public:
		virtual ~IPanel() = default;
	
		virtual void Init() = 0;
		virtual const char* GetTitle() const = 0;
		virtual void Draw() = 0;

		void Open();
		void Close();
		bool IsOpen() const;

	protected:
		DrawDockedWindowImpl DrawDockedWindow(ImGuiWindowFlags flags);
	};
}
