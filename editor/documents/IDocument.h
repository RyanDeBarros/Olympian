#pragma once

#include <string>

#include "documents/ActiveDocument.h"

#include "assets/ResourcePath.h"

#include <imtk.hpp>

#include <imp/undo_history.hpp>

namespace oly::editor
{
	struct IDoubleDescriptor;

	class IDocument : public imtk::tick_processor
	{
	protected:
		detail::ResourcePath _oly_path;

	private:
		imp::checkpoint_undo_history _undo_history;
		imp::event_listener _preferences_listener;
		imp::event_listener _uh_listener;

		bool _dirty = false;
		bool _initialized = false;

	public:
		IDocument(detail::ResourcePath&& oly_path);
		virtual ~IDocument() = default;

		void Init();
		virtual void InitImpl() = 0;
		virtual void Draw() = 0;
		virtual void DrawMenuBar();

		void LoadAsset();
		virtual void LoadImpl() = 0;
		void DumpAsset();
		virtual void DumpImpl() = 0;
		void ResetAsset();
		virtual void ResetAssetImpl() = 0;
		bool Exists();

		virtual const IDoubleDescriptor& GetDoubleDescriptor() const = 0;
		virtual IDoubleDescriptor& GetDoubleDescriptor() = 0;

		void* resolve(imtk::datapath_view path, std::type_index type);
		void describe(std::ostream& os, imtk::datapath_view path) const;
		std::string PathString(imtk::datapath_view path) const;
		void on_last_process_frame() override;

		const detail::ResourcePath& GetOlyPath() const;
		void Rename(const detail::ResourcePath& new_path);
		virtual std::string TabName() const;

		void MarkDirty();
		void MarkClean();
		bool IsDirty() const;
		void query_dirty();

		void Undo();
		void Redo();

	private:
		class PreDrawImpl
		{
			IDocument& _doc;
			imp::active_undo_history _uh;
			ActiveDocument _active_instance;

		public:
			imtk::prop::grid grid;

			PreDrawImpl(IDocument& doc);
			PreDrawImpl(const PreDrawImpl&) = delete;
			PreDrawImpl(PreDrawImpl&&) = delete;
			~PreDrawImpl();
		};

	protected:
		PreDrawImpl PreDraw();
	};
}
