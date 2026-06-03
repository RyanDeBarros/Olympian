#pragma once

namespace oly::editor
{
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
	};
}
