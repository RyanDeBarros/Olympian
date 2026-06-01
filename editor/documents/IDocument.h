#pragma once

#include <string>

#include "assets/ResourcePath.h"

namespace oly::editor
{
	class IDocument
	{
	protected:
		detail::ResourcePath _oly_path;

	private:
		bool _dirty = false;

	public:
		IDocument(detail::ResourcePath&& oly_path);
		virtual ~IDocument() = default;

		virtual void Draw() = 0;

		const detail::ResourcePath& GetOlyPath() const;
		void MarkDirty();
		void MarkClean();
		bool IsDirty() const;
	};
}
