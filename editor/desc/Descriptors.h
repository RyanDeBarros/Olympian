#pragma once

#include <imtk.hpp>

// TODO v9.3 move fields and descriptors to imtk

namespace oly::editor
{
	template<typename Descriptor>
	class VectorDesc
	{
		std::vector<Descriptor> _vector;

	public:
		imtk::datapath_link link;

		VectorDesc(imtk::datapath_link link)
			: link(std::move(link))
		{
		}

		Descriptor& PushBack()
		{
			_vector.push_back(Descriptor(imtk::datapath_link(link, imtk::datapath::step(_vector.size()))));
			return _vector.back();
		}

		void Insert(size_t i, Descriptor element)
		{
			for (auto it = _vector.begin() + i; it != _vector.end(); ++it)
				it->link.set_step(*it->link.step() + 1);

			element.link = imtk::datapath_link(link, imtk::datapath::step(i));
			_vector.insert(_vector.begin() + i, std::move(element));
		}

		void Remove(size_t i)
		{
			_vector.erase(_vector.begin() + i);
			for (auto it = _vector.begin() + i; it != _vector.end(); ++it)
				it->link.set_step(*it->link.step() - 1);
		}
		
		void Clear()
		{
			_vector.clear();
		}

		size_t Size() const
		{
			return _vector.size();
		}

		void Resize(size_t new_size)
		{
			if (new_size < _vector.size())
				_vector.erase(_vector.begin() + new_size, _vector.end());
			else if (new_size > _vector.size())
			{
				for (size_t i = _vector.size(); i < new_size; ++i)
					_vector.push_back(Descriptor(imtk::datapath_link(link, imtk::datapath::step(i))));
			}
		}

		bool Empty() const
		{
			return _vector.empty();
		}

		Descriptor& Back()
		{
			return _vector.back();
		}

		const Descriptor& Back() const
		{
			return _vector.back();
		}

		void Visit(auto&& fn)
		{
			for (Descriptor& desc : _vector)
				fn(desc);
		}

		void VisitIndexed(auto&& fn)
		{
			for (size_t i = 0; i < _vector.size(); ++i)
				fn(i, _vector[i]);
		}

		Descriptor& operator[](size_t i)
		{
			return _vector[i];
		}

		const Descriptor& operator[](size_t i) const
		{
			return _vector[i];
		}

		auto begin() const
		{
			return _vector.begin();
		}

		auto begin()
		{
			return _vector.begin();
		}

		auto end() const
		{
			return _vector.end();
		}

		auto end()
		{
			return _vector.end();
		}

		void* PathGet(imtk::datapath_view path, std::type_index type)
		{
			if (path.empty())
				return typeid(decltype(*this)) == type ? static_cast<void*>(this) : nullptr;
			
			int index = path.step();
			if (index >= 0 && index < _vector.size())
				return _vector[index].PathGet(path.next(), type);
			else
				return nullptr;
		}

		void PrintPath(std::ostream& os, imtk::datapath_view path) const
		{
			if (path.empty())
				os << "<error>";
			else
			{
				int index = path.step();
				if (index >= 0 && index < _vector.size())
				{
					path = path.next();
					if (path.empty())
						os << index;
					else
					{
						os << index << ".";
						_vector[index].PrintPath(os, path);
					}
				}
				else
					os << "<error>";
			}
		}

		bool QueryDirty(const VectorDesc<Descriptor>& disk) const
		{
			if (_vector.size() != disk._vector.size())
				return true;

			for (size_t i = 0; i < _vector.size(); ++i)
			{
				if (_vector[i].QueryDirty(disk._vector[i]))
					return true;
			}

			return false;
		}

		void CopyData(const VectorDesc<Descriptor>& o)
		{
			Resize(o.Size());
			for (size_t i = 0; i < _vector.size(); ++i)
				_vector[i].CopyData(o._vector[i]);
		}
	};

	template<typename... Descriptors>
	class VariantDesc
	{
	public:
		imtk::datapath_link link;

	private:
		std::variant<Descriptors...> _variant;

	public:
		VariantDesc(imtk::datapath_link link)
			: link(std::move(link)), _variant(std::in_place_index<0>, this->link.share())
		{
		}

		VariantDesc(std::variant<Descriptors...> descriptor)
			: link(std::visit([](const auto& desc) { return desc.link.share(); }, descriptor)), _variant(std::move(descriptor))
		{
		}

		template<typename Descriptor>
		Descriptor& Set()
		{
			_variant = Descriptor(link.share());
			return std::get<Descriptor>(_variant);
		}

		template<typename Descriptor>
		void Set(Descriptor&& desc)
		{
			_variant = std::forward<Descriptor>(desc);
			std::visit([this](auto& v) { v.link = link.share(); }, _variant);
		}

		auto Visit(auto&& visitor)
		{
			return std::visit([&visitor](auto& desc) { return visitor(desc); }, _variant);
		}

		auto Visit(auto&& visitor) const
		{
			return std::visit([&visitor](const auto& desc) { return visitor(desc); }, _variant);
		}

		template<typename Descriptor>
		Descriptor* TryGet()
		{
			return std::get_if<Descriptor>(&_variant);
		}

		template<typename Descriptor>
		const Descriptor* TryGet() const
		{
			return std::get_if<Descriptor>(&_variant);
		}

		void* PathGet(imtk::datapath_view path, std::type_index type)
		{
			return std::visit([path, type](auto& desc) { return desc.PathGet(path, type); }, _variant);
		}

		void PrintPath(std::ostream& os, imtk::datapath_view path) const
		{
			return std::visit([&os, path](auto& desc) { return desc.PrintPath(os, path); }, _variant);
		}

		bool QueryDirty(const VariantDesc<Descriptors...>& disk) const
		{
			return std::visit([](const auto& lhs, const auto& rhs) {
				using L = std::decay_t<decltype(lhs)>;
				using R = std::decay_t<decltype(rhs)>;

				if constexpr (std::is_same_v<L, R>)
					return lhs.QueryDirty(rhs);
				else
					return true;
			}, _variant, disk._variant);
		}

		void CopyData(const VariantDesc<Descriptors...>& o)
		{
			std::visit([this](auto& lhs, const auto& rhs) {
				using L = std::decay_t<decltype(lhs)>;
				using R = std::decay_t<decltype(rhs)>;

				if constexpr (std::is_same_v<L, R>)
					lhs.CopyData(rhs);
				else
					Set<R>().CopyData(rhs);
			}, _variant, o._variant);
		}
	};

	template<typename Key, typename ValueDescriptor>
	class MapDesc
	{
		std::unordered_map<Key, ValueDescriptor> _map;

	public:
		imtk::datapath_link link;

		MapDesc(imtk::datapath_link link)
			: link(std::move(link))
		{
		}

		void Clear()
		{
			_map.clear();
		}

		ValueDescriptor& operator[](Key key)
		{
			auto it = _map.find(key);
			if (it != _map.end())
				return it->second;
			else
				return _map.emplace(key, imtk::datapath_link(link, imtk::datapath::step(key))).first->second;
		}

		auto begin()
		{
			return _map.begin();
		}

		auto end()
		{
			return _map.end();
		}

		void* PathGet(imtk::datapath_view path, std::type_index type)
		{
			if (path.empty())
				return typeid(decltype(*this)) == type ? static_cast<void*>(this) : nullptr;

			auto it = _map.find(static_cast<Key>((int)path.step()));
			if (it != _map.end())
				return it->second.PathGet(path.next(), type);
			else
				return nullptr;
		}

		void PrintPath(std::ostream& os, imtk::datapath_view path) const
		{
			if (path.empty())
				os << "<error>";
			else
			{
				auto key = static_cast<Key>((int)path.step());
				auto it = _map.find(key);
				if (it != _map.end())
				{
					path = path.next();
					if (path.empty())
						os << key;
					else
					{
						os << key << ".";
						it->second.PrintPath(os, path);
					}
				}
				else
					os << "<error>";
			}
		}

		bool QueryDirty(const MapDesc<Key, ValueDescriptor>& disk) const
		{
			if (_map.size() != disk._map.size())
				return true;

			for (const auto& [key, desc] : _map)
			{
				auto it = disk._map.find(key);
				if (it == disk._map.end())
					return true;

				if (desc.QueryDirty(it->second))
					return true;
			}

			return false;
		}

		void CopyData(const MapDesc<Key, ValueDescriptor>& o)
		{
			for (auto it = _map.begin(); it != _map.end(); ++it)
			{
				auto o_it = o._map.find(it->first);
				if (o_it != o._map.end())
					it->second.CopyData(o_it->second);
				else
					it = _map.erase(it);
			}

			for (const auto& [o_key, o_desc] : o._map)
			{
				if (!_map.contains(o_key))
					(*this)[o_key].CopyData(o_desc);
			}
		}
	};
}
