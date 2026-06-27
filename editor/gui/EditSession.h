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
				buffer = truth;

			seen_this_frame = true;
			published = false;
		}

		void PostEdit(DrawResult result)
		{
			if (result.IsDeactivatedAfterEdit())
			{
				editing = false;
				std::swap(truth, buffer);
				published = true;
			}

			if (result.IsActivated())
				editing = true;
		}

		// TODO v9.1 document should call finalize on all properties, regardless of whether they were drawn. After Finalize(), ConsumeModified() = true should push the undo action
		void Finalize()
		{
			if (!seen_this_frame && editing)
			{
				editing = false;
				std::swap(truth, buffer);
				published = true;
			}

			seen_this_frame = false;
		}

		bool Modified() const
		{
			return published && buffer != truth;
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
	};
}
