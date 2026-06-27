#pragma once

#include <string>

#include "documents/ActiveDocument.h"

#include "core/UndoHistory.h"

#include "gui/properties/PropertyGrid.h"

#include "desc/DoubleDescriptor.h"

#include "assets/ResourcePath.h"

namespace oly::editor
{
	class IDocument
	{
	protected:
		detail::ResourcePath _oly_path;
		std::optional<UndoHistory> _undo_history;

	private:
		bool _dirty = false;

	public:
		IDocument(detail::ResourcePath&& oly_path);
		virtual ~IDocument() = default;

		void Init();
		virtual void InitImpl() = 0;
		virtual void Draw() = 0;
		virtual void DrawMenuBar();
		virtual void Load() = 0;
		virtual void Dump() = 0;
		virtual const IDoubleDescriptor& GetDoubleDescriptor() const = 0;
		virtual IDoubleDescriptor& GetDoubleDescriptor() = 0;

		void* PathGet(DataPath path, std::type_index type);
		void PrintPath(std::ostream& os, DataPath path) const;
		std::string PathString(DataPath path) const;
		void DrawFinalize();

		const detail::ResourcePath& GetOlyPath() const;
		virtual std::string TabName() const;

		void MarkDirty();
		void MarkClean();
		bool IsDirty() const;
		void QueryDirty();

		void Undo();
		void Redo();

	private:
		class PreDrawImpl
		{
			IDocument& _doc;
			gui::PropertyGrid _grid;
			UndoHistoryActiveScope _uh_scope;
			ActiveDocument _active_instance;

		public:
			PreDrawImpl(IDocument& doc);
			PreDrawImpl(const PreDrawImpl&) = delete;
			PreDrawImpl(PreDrawImpl&&) = delete;
			~PreDrawImpl();
		};

	protected:
		PreDrawImpl PreDraw();
	};
}
