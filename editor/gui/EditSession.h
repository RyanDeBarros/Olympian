#pragma once

#include "gui/DrawResult.h"
#include "core/editor/Editor.h"

#include <tuple>

namespace oly::editor
{
	// TODO v9.3 remove in favour of imtk::edit_session
	template<typename T>
	struct EditSession : public imtk::tick_processor
	{
		T& truth;
		T buffer = T();
		T original = T();

		bool editing = false;
		bool published = false;

		EditSession(T& truth)
			: truth(truth)
		{
		}

		void PreEdit()
		{
			if (!editing)
			{
				buffer = truth;
				original = buffer;
			}

			published = false;
		}

		void PostEdit(DrawResult result)
		{
			if (result.IsDeactivatedAfterEdit())
			{
				editing = false;
				truth = buffer;
				published = true;
			}

			if (result.IsActivated())
				editing = true;
		}

		void on_last_process_frame() override
		{
			if (editing)
			{
				editing = false;
				truth = buffer;
				published = true;
			}
		}

		bool Modified() const
		{
			return published && buffer != original;
		}

		bool ConsumeModified()
		{
			const bool modified = Modified();
			published = false;
			return modified;
		}

		void PublishReset(T to)
		{
			truth = to;
			buffer = std::move(to);
			published = true;
		}

		void CancelEditing()
		{
			buffer = truth;
			original = buffer;
			editing = false;
			published = false;
		}
	};
}
