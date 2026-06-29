#pragma once

#include "gui/DrawResult.h"

#include <tuple>

namespace oly::editor
{
	template<typename T>
	struct EditSession
	{
		T& truth;
		T buffer = T();
		T original = T();

		bool editing = false;
		bool seen_this_frame = false;
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

			seen_this_frame = true;
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

		void DrawFinalize()
		{
			if (!seen_this_frame && editing)
			{
				editing = false;
				truth = buffer;
				published = true;
			}

			seen_this_frame = false;
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
