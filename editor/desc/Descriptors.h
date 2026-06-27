#pragma once

#include "gui/ListModel.h"

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

		bool Empty() const
		{
			return vector.empty();
		}

		Descriptor& Back()
		{
			return vector.back();
		}

		const Descriptor& Back() const
		{
			return vector.back();
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

		auto begin() const
		{
			return vector.begin();
		}

		auto begin()
		{
			return vector.begin();
		}

		auto end() const
		{
			return vector.end();
		}

		auto end()
		{
			return vector.end();
		}

		std::unique_ptr<gui::IListAdapter> ListAdapter()
		{
			return std::make_unique<gui::VectorAdapter<Descriptor>>(vector);
		}

		void* VisitPath(DataPath path, std::type_index type)
		{
			if (path.Empty())
				return nullptr;

			int index = path.Step().v;
			if (index >= 0 && index < vector.size())
				return vector[index].VisitPath(path.Next(), type);
			else
				return nullptr;
		}

		DataPathStep Subpath(size_t index)
		{
			return DataPathStep(index);
		}

		bool DrawFinalize(DataPath path)
		{
			bool dirty = false;
			for (size_t i = 0; i < vector.size(); ++i)
				dirty |= vector[i].DrawFinalize(path / Subpath(i));
			return dirty;
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

		void* VisitPath(DataPath path, std::type_index type)
		{
			return std::visit([path, type](auto& desc) { return desc.VisitPath(path, type); }, variant);
		}

		bool DrawFinalize(DataPath path)
		{
			return std::visit([path](auto& desc) { return desc.DrawFinalize(path); }, variant);
		}
	};

	template<typename Key, typename ValueDescriptor>
	struct MapDesc
	{
		std::unordered_map<Key, ValueDescriptor> map;

		void Clear()
		{
			map.clear();
		}

		ValueDescriptor& operator[](Key key)
		{
			return map[key];
		}

		auto begin()
		{
			return map.begin();
		}

		auto end()
		{
			return map.end();
		}

		void* VisitPath(DataPath path, std::type_index type)
		{
			if (path.Empty())
				return nullptr;

			auto it = map.find(static_cast<Key>(path.Step().v));
			if (it != map.end())
				return it->second.VisitPath(path.Next(), type);
			else
				return nullptr;
		}

		DataPathStep Subpath(Key key)
		{
			return DataPathStep(key);
		}

		bool DrawFinalize(DataPath path)
		{
			bool dirty = false;
			for (auto& [key, desc] : map)
				dirty |= desc.DrawFinalize(path / Subpath(key));
			return dirty;
		}
	};
}
