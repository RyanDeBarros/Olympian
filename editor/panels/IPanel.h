#pragma once

#include <imgui.h>

namespace oly::editor
{
	class DrawDockedWindowImpl
	{
		bool _call_end;
		bool _visible;
		bool _request_close;

	public:
		DrawDockedWindowImpl(bool call_end, bool visible, bool request_close);
		DrawDockedWindowImpl(const DrawDockedWindowImpl&) = delete;
		DrawDockedWindowImpl(DrawDockedWindowImpl&&) = delete;
		~DrawDockedWindowImpl();

		bool IsVisible() const;
		bool RequestsClose() const;
	};

	class IPanel
	{
		bool _open = false;
		static IPanel* _gain_next;

	public:
		virtual ~IPanel() = default;
	
		void Init();
		virtual void InitImpl() = 0;
		void Terminate();
		virtual void TerminateImpl() {}

		virtual const char* GetTitle() const = 0;
		virtual void Draw() = 0;

		void Open();
		void Close();
		bool IsOpen() const;
		void GainFocus();

	protected:
		DrawDockedWindowImpl DrawDockedWindow(ImGuiWindowFlags flags = 0);
	};
}
