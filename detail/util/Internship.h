#pragma once

#include "util/Hash.h"

#include <unordered_map>
#include <vector>

namespace oly
{
	template<typename Type>
	class Internship
	{
	public:
		using Handle = size_t;

	private:
		std::vector<Type> data;
		std::unordered_multimap<size_t, Handle> lut;

	public:
		template<typename View, typename ViewHash = std::hash<View>, typename ViewEquals = std::equal_to<View>, typename ConvertType = void>
		Handle Intern(const View& view)
		{
			size_t hash = Hasher().with<ViewHash>(view);
			auto range = lut.equal_range(hash);

			for (auto it = range.first; it != range.second; ++it)
			{
				auto handle = it->second;
				if (ViewEquals{}(data[handle], view))
					return handle;
			}

			Handle handle = data.size();
			if constexpr (std::is_same_v<ConvertType, void>)
				data.emplace_back(view);
			else
				data.emplace_back(ConvertType{}(view));
			lut.emplace(hash, handle);
			return handle;
		}

		const Type& Get(const Handle handle) const
		{
			return data[handle];
		}
	};
}
