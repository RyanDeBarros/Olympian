#pragma once

#include <imtk.hpp>

namespace oly::editor
{
	class DrawDockedWindowImpl
	{
		std::unique_ptr<imtk::window> _window;
		bool _request_close;

	public:
		DrawDockedWindowImpl(std::unique_ptr<imtk::window>&& window, bool request_close);

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
		void ToggleOpen();
		void GainFocus();

	protected:
		DrawDockedWindowImpl DrawDockedWindow(ImGuiWindowFlags flags = 0);
	};
}
