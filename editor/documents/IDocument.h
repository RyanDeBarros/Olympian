#pragma once

#include <string>

namespace oly::editor
{
	class IDocument
	{
		bool _dirty = false;

	public:
		virtual ~IDocument() = default;

		virtual std::string GetTitle() const = 0;
		virtual void Draw() = 0;

		void MarkDirty();
		void MarkClean();
		bool IsDirty() const;
	};
}
