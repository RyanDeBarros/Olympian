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
			Serializer<T>{}.Load(value, node[detail::encode_key(key)]);
		}

		void Dump(toml::table& table)
		{
			table.insert_or_assign(detail::encode_key(key), Serializer<T>{}.Dump(value));
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
	};
}
