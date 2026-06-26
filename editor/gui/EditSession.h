#pragma once

#include <imgui.h>

namespace oly::editor
{
	template<typename T>
	struct EditSession
	{
		T& truth;
		T buffer;

		bool editing = false;
		bool seen_this_frame = false;
		bool published = false;

		EditSession(T& truth)
			: truth(truth), buffer(truth)
		{
		}

		void PreEdit()
		{
			if (!editing)
				buffer = truth;

			seen_this_frame = true;
			published = false;
		}

		void PostEdit()
		{
			if (ImGui::IsItemActivated())
				editing = true;

			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				editing = false;
				std::swap(truth, buffer);
				published = true;
			}
		}

		// TODO v9.1 document should call finalize on all properties, regardless of whether they were drawn
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

		bool Modified()
		{
			return published && buffer != truth;
		}

		void SetAll(T to)
		{
			truth = to;
			buffer = std::move(to);
		}
	};
}
