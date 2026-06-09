#pragma once

#include "desc/DescIO.h"
#include "desc/Serializer.h"

#include "assets/TranslateKey.h"

#include <array>

namespace oly::editor
{
#define DRAW_FIELD(field) if (desc.field.Draw()) MarkDirty();
#define DRAW_FIELDS(generator) generator(DRAW_FIELD);
#define LOAD_FIELD(field) desc.field.Load(node);
#define LOAD_FIELDS(generator) generator(LOAD_FIELD)
#define DUMP_FIELD(field) desc.field.Dump(table);
#define DUMP_FIELDS(generator) generator(DUMP_FIELD)

	extern detail::Key NullKey();

	template<typename T>
	struct PrimitiveField
	{
		T def;
		T scratch;
		detail::Key key;
		const char* label;

		PrimitiveField(T def, detail::Key key, const char* label) : def(def), scratch(def), key(key), label(label) {}

		void Load(TOMLNode node)
		{
			scratch = def;
			if (key != NullKey())
				Serializer<T>{}.Load(scratch, node[detail::encode_key(key)]);
		}

		void Dump(toml::table& table) const
		{
			if (key != NullKey())
				table.insert_or_assign(detail::encode_key(key), Serializer<T>{}.Dump(scratch));
		}
	};

	struct BoolField : public PrimitiveField<bool>
	{
		using PrimitiveField<bool>::PrimitiveField;

		bool Draw();
	};

	template<typename T, typename U, OptionalPrimitive<U> _Min, OptionalPrimitive<U> _Max>
	struct RangeField : public PrimitiveField<T>
	{
		inline static const OptionalPrimitive<U> Min = _Min;
		inline static const OptionalPrimitive<U> Max = _Max;

		using PrimitiveField<T>::PrimitiveField;

		bool Draw()
		{
			return DescIO::Draw(this->label, this->scratch, this->def, Min, Max);
		}
	};

	template<OptionalInt Min, OptionalInt Max>
	using IntField = RangeField<int, int, Min, Max>;

	template<OptionalFloat Min, OptionalFloat Max>
	using FloatField = RangeField<float, float, Min, Max>;

	template<OptionalDouble Min, OptionalDouble Max>
	using DoubleField = RangeField<double, double, Min, Max>;

	template<typename E>
	struct EnumField : public PrimitiveField<E>
	{
		static_assert(std::is_enum_v<E>);

		using PrimitiveField<E>::PrimitiveField;

		bool Draw()
		{
			return DescIO::Draw(this->label, this->scratch, this->def);
		}
	};

	struct StringField : public PrimitiveField<std::string>
	{
		using PrimitiveField<std::string>::PrimitiveField;

		bool Draw();
	};

	template<size_t N>
	struct BoolArrayField : public PrimitiveField<std::array<bool, N>>
	{
		const char** sublabels;

		BoolArrayField(std::array<bool, N> def, detail::Key key, const char* label, const char* (&sublabels)[N])
			: PrimitiveField<std::array<bool, N>>(def, key, label), sublabels(sublabels) {}

		bool Draw()
		{
			return DescIO::Draw(this->label, this->scratch.data(), this->def.data(), sublabels, N);
		}
	};

	struct ColorField : public PrimitiveField<Color>
	{
		using PrimitiveField<Color>::PrimitiveField;

		bool Draw()
		{
			return DescIO::Draw(label, scratch, def);
		}
	};

	struct GLenumField
	{
		GLenum def;
		int scratch;
		int def_index;
		detail::Key key;
		const char* label;
		const GLenum* values;
		const char** names;
		size_t count;

		template<size_t Count>
		GLenumField(GLenum def, detail::Key key, const char* label, const GLenum(&values)[Count], const char* (&names)[Count])
			: def(def), key(key), label(label), values(values), names(names), count(Count)
		{
			SetScratch(def);
			def_index = Index(def);
		}

		bool Draw();
		void Load(TOMLNode node);
		void Dump(toml::table& table) const;

		GLenum Scratch() const;
		void SetScratch(const GLenum val);

		GLenum Value(int index) const;
		int Index(const GLenum val) const;
	};
	
	template<typename T, OptionalPrimitive<T> _Min, OptionalPrimitive<T> _Max>
	struct OptionalRangeField
	{
		using Self = OptionalRangeField<T, _Min, _Max>;
		inline static const OptionalPrimitive<T> Min = _Min;
		inline static const OptionalPrimitive<T> Max = _Max;

		OptionalPrimitive<T> def;
		OptionalPrimitive<T> scratch;
		detail::Key value_key;
		detail::Key enable_key;
		const char* label;

		OptionalRangeField(OptionalPrimitive<T> def, detail::Key value_key, detail::Key enable_key, const char* label)
			: def(def), scratch(def), value_key(value_key), enable_key(enable_key), label(label)
		{
		}

		bool Draw()
		{
			return DescIO::Draw(label, scratch, this->def, Min, Max);
		}

		void Load(TOMLNode node)
		{
			scratch = def;
			if (enable_key != NullKey() && value_key != NullKey())
			{
				Serializer<T>{}.Load(scratch.value, node[detail::encode_key(value_key)]);
				Serializer<bool>{}.Load(scratch.has_value, node[detail::encode_key(enable_key)]);
			}
		}

		void Dump(toml::table& table) const
		{
			if (enable_key != NullKey() && value_key != NullKey())
			{
				table.insert_or_assign(detail::encode_key(enable_key), Serializer<bool>{}.Dump(scratch.has_value));
				table.insert_or_assign(detail::encode_key(value_key), Serializer<T>{}.Dump(scratch.value));
			}
		}
	};

	template<OptionalInt Min, OptionalInt Max>
	using OptionalIntField = OptionalRangeField<int, Min, Max>;

	template<OptionalFloat Min, OptionalFloat Max>
	using OptionalFloatField = OptionalRangeField<float, Min, Max>;

	template<OptionalDouble Min, OptionalDouble Max>
	using OptionalDoubleField = OptionalRangeField<double, Min, Max>;

	template<typename T, OptionalPrimitive<T> _Min, OptionalPrimitive<T> _Max>
	struct CompactOptionalRangeField
	{
		inline static const OptionalPrimitive<T> Min = _Min;
		inline static const OptionalPrimitive<T> Max = _Max;

		OptionalPrimitive<T> def;
		OptionalPrimitive<T> scratch;
		T nullopt;
		detail::Key key;
		const char* label;

		CompactOptionalRangeField(OptionalPrimitive<T> def, T nullopt, detail::Key key, const char* label)
			: def(def), scratch(def), nullopt(nullopt), key(key), label(label)
		{
		}

		bool Draw()
		{
			scratch.has_value = scratch.value != nullopt;
			return DescIO::Draw(label, scratch, def, Min, Max);
		}

		void Load(TOMLNode node)
		{
			scratch = def;
			if (key != NullKey())
			{
				Serializer<T>{}.Load(scratch.value, node[detail::encode_key(key)]);
				scratch.has_value = scratch.value != nullopt;
			}
		}

		void Dump(toml::table& table) const
		{
			if (key != NullKey())
				table.insert_or_assign(detail::encode_key(key), Serializer<T>{}.Dump(scratch.has_value ? scratch.value : nullopt));
		}
	};

	template<OptionalInt Min, OptionalInt Max>
	using CompactOptionalIntField = CompactOptionalRangeField<int, Min, Max>;

	template<OptionalFloat Min, OptionalFloat Max>
	using CompactOptionalFloatField = CompactOptionalRangeField<float, Min, Max>;

	extern void LoadStringArray(TOMLNode node, std::string* strings, size_t count);
	extern toml::array DumpStringArray(const std::string* strings, size_t count);

	template<size_t N>
	struct StringArrayField
	{
		std::array<std::string, N> def;
		std::array<std::string, N> scratch;
		detail::Key key;
		const char* label;

		StringArrayField(std::array<std::string, N>&& def, detail::Key key, const char* label) : def(def), scratch(std::move(def)), key(key), label(label) {}

		bool Draw()
		{
			return DescIO::Draw(label, scratch.data(), def.data(), N);
		}

		void Load(TOMLNode node)
		{
			scratch = def;
			if (key != NullKey())
				LoadStringArray(node[detail::encode_key(key)], scratch.data(), N);
		}

		void Dump(toml::table& table) const
		{
			if (key != NullKey())
				table.insert_or_assign(detail::encode_key(key), DumpStringArray(scratch.data(), N));
		}
	};

	template<OptionalFloat Min, OptionalFloat Max, glm::length_t L>
	using VecField = RangeField<glm::vec<L, float>, float, Min, Max>;

	template<OptionalFloat Min, OptionalFloat Max>
	using Vec2Field = VecField<Min, Max, 2>;
	
	template<OptionalFloat Min, OptionalFloat Max>
	using Vec3Field = VecField<Min, Max, 3>;
	
	template<OptionalFloat Min, OptionalFloat Max>
	using Vec4Field = VecField<Min, Max, 4>;
}
