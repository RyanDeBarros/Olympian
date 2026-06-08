#pragma once

#include "desc/Fields.h"

namespace oly::editor
{
	template<typename Descriptor>
	struct ArrayDesc
	{
		std::vector<std::unique_ptr<Descriptor>> array;
		IntField<MakeOpt(1), MakeOpt<int>()> size;

		ArrayDesc() : size(0, NullKey(), "") {}

		void Reset(ArrayDesc<Descriptor>& source)
		{
			Clear();
			array.resize(source.array.size());
			for (size_t i = 0; i < array.size(); ++i)
			{
				array[i] = std::make_unique<Descriptor>();
				array[i]->Reset(*source.array[i]);
			}
			size.Reset(source.size);
		}

		void Isolate()
		{
			for (auto& desc : array)
				desc->Isolate();
			size.Isolate();
		}

		void PushBack()
		{
			array.push_back(std::make_unique<Descriptor>());
			++size.scratch;
		}

		void Remove(size_t i)
		{
			array[i]->Isolate();
			array.erase(array.begin() + i);
			--size.scratch;
		}

		void Clear()
		{
			Isolate();
			array.clear();
			size.scratch = 0;
		}

		void Resize(ArrayDesc<Descriptor>& source)
		{
			if (array.size() < source.array.size())
			{
				for (size_t i = array.size(); i < source.array.size(); ++i)
				{
					array.push_back(std::make_unique<Descriptor>());
					array[i]->Reset(*source.array[i]);
				}
			}
			else if (source.array.size() < array.size())
			{
				for (size_t i = source.array.size(); i < array.size(); ++i)
					array[i]->Isolate();
				array.erase(array.begin() + source.array.size());
			}
			size.Reset(source.size);
		}

		void Visit(auto&& fn)
		{
			for (auto& el : array)
				fn(*el);
		}

		void VisitIndexed(auto&& fn)
		{
			for (size_t i = 0; i < size.scratch; ++i)
				fn(i, *array[i]);
		}
	};
}
