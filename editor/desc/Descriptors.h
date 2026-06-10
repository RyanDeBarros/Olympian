#pragma once

#include "core/ListModel.h"

#include <variant>

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

		Descriptor& operator[](size_t i)
		{
			return vector[i];
		}

		const Descriptor& operator[](size_t i) const
		{
			return vector[i];
		}

		auto begin()
		{
			return vector.begin();
		}

		auto end()
		{
			return vector.end();
		}

		std::unique_ptr<IListAdapter> ListAdapter()
		{
			return std::make_unique<VectorAdapter<Descriptor>>(vector);
		}
	};

	template<typename... Descriptors>
	struct VariantDesc
	{
		std::variant<Descriptors...> variant;

		void Reset()
		{
			std::visit([](auto& v) { v = std::decay_t<decltype(v)>(); }, variant);
		}

		template<typename Descriptor>
		Descriptor& Set()
		{
			variant = Descriptor();
			return std::get<Descriptor>(variant);
		}

		template<typename Descriptor>
		void Set(Descriptor&& desc)
		{
			variant = std::forward<Descriptor>(desc);
		}

		auto Visit(auto&& visitor)
		{
			return std::visit([&visitor](auto& desc) { return visitor(desc); }, variant);
		}

		auto Visit(auto&& visitor) const
		{
			return std::visit([&visitor](const auto& desc) { return visitor(desc); }, variant);
		}

		template<typename Descriptor>
		Descriptor* TryGet()
		{
			return std::get_if<Descriptor>(&variant);
		}

		template<typename Descriptor>
		const Descriptor* TryGet() const
		{
			return std::get_if<Descriptor>(&variant);
		}
	};
}
