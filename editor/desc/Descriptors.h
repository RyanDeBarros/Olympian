#pragma once

#include <vector>

namespace oly::editor
{
	template<typename Descriptor>
	struct VectorDesc
	{
		std::vector<Descriptor> vector;

		void PushBack()
		{
			vector.push_back({});
		}

		void PushBack(const Descriptor& desc)
		{
			vector.push_back(desc);
		}

		void PushBack(Descriptor&& desc)
		{
			vector.push_back(std::move(desc));
		}

		void Remove(size_t i)
		{
			vector.erase(vector.begin() + i);
		}
		
		void Clear()
		{
			vector.clear();
		}

		size_t Size() const
		{
			return vector.size();
		}

		void Visit(auto&& fn)
		{
			for (Descriptor& desc : vector)
				fn(desc);
		}

		void VisitIndexed(auto&& fn)
		{
			for (size_t i = 0; i < vector.size(); ++i)
				fn(i, vector[i]);
		}
	};
}
