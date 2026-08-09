#pragma once

#include "desc/Serializer.h"

#include "assets/TranslateKey.h"

namespace oly::editor
{
#define LOAD_SIMPLE_FIELD(F) F.Load(node);
#define LOAD_SIMPLE_FIELDS(GENERATOR) GENERATOR(LOAD_SIMPLE_FIELD)

#define DUMP_SIMPLE_FIELD(F) F.Dump(table);
#define DUMP_SIMPLE_FIELDS(GENERATOR) GENERATOR(DUMP_SIMPLE_FIELD)

#define LOAD_SIMPLE_FIELDS_IMPL(GENERATOR) void Load(TOMLNode node) { LOAD_SIMPLE_FIELDS(GENERATOR) }
#define DUMP_SIMPLE_FIELDS_IMPL(GENERATOR) void Dump(toml::table& table) { DUMP_SIMPLE_FIELDS(GENERATOR) }
#define LOAD_DUMP_SIMPLE_FIELDS_IMPL(GENERATOR) LOAD_SIMPLE_FIELDS_IMPL(GENERATOR) DUMP_SIMPLE_FIELDS_IMPL(GENERATOR)

	template<typename T>
	struct SimpleField
	{
		T value;
		detail::Key key;

		SimpleField(T value, detail::Key key)
			: value(std::move(value)), key(key)
		{
		}

		void Load(TOMLNode node)
		{
			Serializer<T>{}.load(value, node[detail::encode_key(key)]);
		}

		void Dump(toml::table& table)
		{
			table.insert_or_assign(detail::encode_key(key), Serializer<T>{}.dump(value));
		}

		const T& operator*() const
		{
			return value;
		}

		T& operator*()
		{
			return value;
		}

		const T* operator->() const
		{
			return &value;
		}

		T* operator->()
		{
			return &value;
		}

		const T* ValuePtr() const
		{
			return &value;
		}

		T* ValuePtr()
		{
			return &value;
		}
	};

	template<typename Desc>
	struct SimpleDesc
	{
		Desc desc;
		detail::Key key;

		SimpleDesc(detail::Key key)
			: key(key)
		{
		}

		void Load(TOMLNode node)
		{
			desc.Load(node[detail::encode_key(key)]);
		}

		void Dump(toml::table& table)
		{
			toml::table subtable;
			desc.Dump(subtable);
			table.insert_or_assign(detail::encode_key(key), std::move(subtable));
		}

		const Desc& operator*() const
		{
			return desc;
		}

		Desc& operator*()
		{
			return desc;
		}

		const Desc* operator->() const
		{
			return &desc;
		}

		Desc* operator->()
		{
			return &desc;
		}

		const Desc* ValuePtr() const
		{
			return &desc;
		}

		Desc* ValuePtr()
		{
			return &desc;
		}
	};

	template<typename Desc>
	struct SimpleArrayDesc
	{
		std::vector<Desc> descs;
		detail::Key key;

		SimpleArrayDesc(detail::Key key)
			: key(key)
		{
		}

		void Load(TOMLNode node)
		{
			descs.clear();
			if (auto array = node[detail::encode_key(key)].as_array())
			{
				descs.resize(array->size());
				for (size_t i = 0; i < descs.size(); ++i)
					descs[i].Load(TOMLNode(*array->get(i)));
			}
		}

		void Dump(toml::table& table)
		{
			toml::array array;
			for (size_t i = 0; i < descs.size(); ++i)
			{
				toml::table subtable;
				descs[i].Dump(subtable);
				array.push_back(std::move(subtable));
			}
			table.insert_or_assign(detail::encode_key(key), std::move(array));
		}

		const Desc& operator[](size_t i) const
		{
			return descs[i];
		}

		Desc& operator[](size_t i)
		{
			return descs[i];
		}

		auto begin() const noexcept
		{
			return descs.begin();
		}

		auto end() const noexcept
		{
			return descs.end();
		}

		auto begin() noexcept
		{
			return descs.begin();
		}

		auto end() noexcept
		{
			return descs.end();
		}

		auto cbegin() const noexcept
		{
			return descs.cbegin();
		}

		auto cend() const noexcept
		{
			return descs.cend();
		}
	};
}
