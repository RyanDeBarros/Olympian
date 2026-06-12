#pragma once

#include "desc/DescIO.h"
#include "desc/Serializer.h"

#include "assets/TranslateKey.h"

#include "gui/DynamicList.h"

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

		bool Draw()
		{
			return DescIO::Draw(label, scratch, def);
		}
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

		bool Draw()
		{
			return DescIO::Draw(label, scratch, def);
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

	template<typename T, size_t N>
	struct ArrayField : public PrimitiveField<std::array<T, N>>
	{
		const char** sublabels;

		ArrayField(std::array<T, N> def, detail::Key key, const char* label, const char* (&sublabels)[N])
			: PrimitiveField<std::array<T, N>>(def, key, label), sublabels(sublabels) {}

		bool Draw()
		{
			return DescIO::Draw(this->label, this->scratch.data(), this->def.data(), sublabels, N);
		}
	};

	template<size_t N>
	using BoolArrayField = ArrayField<bool, N>;

	template<typename T, size_t N>
	struct AnonArrayField : public PrimitiveField<std::array<T, N>>
	{
		using PrimitiveField<std::array<T, N>>::PrimitiveField;

		bool Draw()
		{
			return DescIO::Draw(this->label, this->scratch.data(), this->def.data(), N);
		}
	};

	template<size_t N>
	using StringArrayField = AnonArrayField<std::string, N>;

	template<typename T>
	struct VectorField : public PrimitiveField<std::vector<T>>
	{
		gui::DynamicListState ui_state;

		using PrimitiveField<std::vector<T>>::PrimitiveField;

		bool Draw()
		{
			return DescIO::Draw(this->label, this->scratch, this->def, ui_state);
		}
	};

	using StringVectorField = VectorField<std::string>;

	template<typename E>
	struct DisjointEnumField
	{
		E def;
		int scratch;
		int def_index;
		detail::Key key;
		const char* label;
		const E* values;
		const char** names;
		size_t count;

		template<size_t Count>
		DisjointEnumField(E def, detail::Key key, const char* label, const E (&values)[Count], const char* (&names)[Count])
			: def(def), key(key), label(label), values(values), names(names), count(Count)
		{
			SetScratch(def);
			def_index = Index(def);
		}

		bool Draw()
		{
			return DescIO::Draw(label, scratch, def_index, names, count);
		}

		void Load(TOMLNode node)
		{
			scratch = Index(static_cast<E>(node[detail::encode_key(key)].value_or(def)));
		}
		
		void Dump(toml::table& table) const
		{
			table.insert_or_assign(detail::encode_key(key), Scratch());
		}

		E Scratch() const
		{
			return Value(scratch);
		}
		
		void SetScratch(const E val)
		{
			scratch = Index(val);
		}

		E Value(int index) const
		{
			return values[index];
		}
		
		int Index(const E val) const
		{
			for (size_t i = 0; i < count; ++i)
			{
				if (val == values[i])
					return i;
			}

			return -1;
		}
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

	template<OptionalFloat Min, OptionalFloat Max, glm::length_t L>
	using VecField = RangeField<glm::vec<L, float>, float, Min, Max>;

	template<OptionalFloat Min, OptionalFloat Max>
	using Vec2Field = VecField<Min, Max, 2>;
	
	template<OptionalFloat Min, OptionalFloat Max>
	using Vec3Field = VecField<Min, Max, 3>;
	
	template<OptionalFloat Min, OptionalFloat Max>
	using Vec4Field = VecField<Min, Max, 4>;

	template<typename E, size_t Count>
	struct BitsetField
	{
		E def;
		E scratch;
		detail::Key key;
		const char* label;
		const E* values;
		const char** names;

		static const inline size_t Count = Count;

		BitsetField(E def, detail::Key key, const char* label, const E(&values)[Count], const char* (&names)[Count])
			: def(def), scratch(def), key(key), label(label), values(values), names(names)
		{
		}

		bool Draw(const bool (&disabled)[Count])
		{
			return Draw(static_cast<const bool*>(disabled));
		}

		bool Draw()
		{
			return Draw(nullptr);
		}

	private:
		bool Draw(const bool* disabled)
		{
			return DescIO::Draw(label, scratch, def, values, names, disabled, Count);
		}

	public:
		void Load(TOMLNode node)
		{
			scratch = static_cast<E>(node[detail::encode_key(key)].value_or(def));
		}

		void Dump(toml::table& table) const
		{
			table.insert_or_assign(detail::encode_key(key), scratch);
		}
	};
}
