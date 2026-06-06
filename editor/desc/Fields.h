#pragma once

#include "desc/DescIO.h"
#include "desc/OptionalPrimitive.h"

#include "external/TOML.h"

#include "assets/TranslateKey.h"

namespace oly::editor
{
#define DRAW_FIELD(field) if (desc.field.Draw()) MarkDirty();
#define DRAW_FIELDS(generator) generator(DRAW_FIELD);
#define LOAD_FIELD(field) desc.field.Load(node);
#define LOAD_FIELDS(generator) generator(LOAD_FIELD)
#define DUMP_FIELD(field) desc.field.Dump(table);
#define DUMP_FIELDS(generator) generator(DUMP_FIELD)

#define RESET_FIELD(field) field.Reset(source.field);
#define RESET_FIELDS(generator) Isolate(); generator(RESET_FIELD)
#define ISOLATE_FIELD(field) field.Isolate();
#define ISOLATE_FIELDS(generator) generator(ISOLATE_FIELD)

#define DISK_FIELD(disk) (disk ? &disk->scratch : nullptr)

	template<typename T, typename Self, typename NodeType = T>
	struct PrimitiveField
	{
		T def;
		T scratch;
		Self* disk = nullptr;
		detail::Key key;
		const char* label;

		PrimitiveField(T def, detail::Key key, const char* label) : def(def), scratch(def), key(key), label(label) {}

		void Load(TOMLNode node)
		{
			if (key != detail::Key::_)
				scratch = static_cast<T>(node[detail::encode_key(key)].value_or(static_cast<NodeType>(def)));
			else
				scratch = def;
		}

		void Dump(toml::table& table) const
		{
			if (key != detail::Key::_)
				table.insert_or_assign(detail::encode_key(key), static_cast<NodeType>(scratch));
		}

		void Isolate()
		{
			if (disk)
			{
				auto d = disk;
				disk = nullptr;
				d->Isolate();
			}
		}

		void Reset(Self& source)
		{
			disk = &source;
			source.disk = static_cast<Self*>(this);
			scratch = source.scratch;
		}
	};

	struct BoolField : public PrimitiveField<bool, BoolField>
	{
		using PrimitiveField<bool, BoolField>::PrimitiveField;

		bool Draw()
		{
			return DescIO::Draw(label, scratch, DISK_FIELD(this->disk));
		}
	};

	template<typename T, OptionalPrimitive<T> _Min, OptionalPrimitive<T> _Max>
	struct RangeField : public PrimitiveField<T, RangeField<T, _Min, _Max>>
	{
		inline static const OptionalPrimitive<T> Min = _Min;
		inline static const OptionalPrimitive<T> Max = _Max;

		using PrimitiveField<T, RangeField<T, _Min, _Max>>::PrimitiveField;

		bool Draw()
		{
			return DescIO::Draw(this->label, this->scratch, DISK_FIELD(this->disk), Min, Max);
		}
	};

	template<OptionalInt Min, OptionalInt Max>
	using IntField = RangeField<int, Min, Max>;

	template<OptionalFloat Min, OptionalFloat Max>
	using FloatField = RangeField<float, Min, Max>;

	template<typename E>
	struct EnumField : public PrimitiveField<E, EnumField<E>, int64_t>
	{
		static_assert(std::is_enum_v<E>);

		using PrimitiveField<E, EnumField<E>, int64_t>::PrimitiveField;

		bool Draw()
		{
			return DescIO::Draw(this->label, this->scratch, DISK_FIELD(this->disk));
		}
	};

	struct GLenumField
	{
		GLenum def;
		int scratch;
		GLenumField* disk = nullptr;
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
		}

		bool Draw()
		{
			return DescIO::Draw(label, scratch, DISK_FIELD(disk), names, count);
		}

		void Load(TOMLNode node)
		{
			scratch = Index(static_cast<GLenum>(node[detail::encode_key(key)].value_or(def)));
		}

		void Dump(toml::table& table) const
		{
			table.insert_or_assign(detail::encode_key(key), Scratch());
		}

		void Isolate()
		{
			if (disk)
			{
				auto d = disk;
				disk = nullptr;
				d->Isolate();
			}
		}

		void Reset(GLenumField& source)
		{
			disk = &source;
			source.disk = this;
			scratch = source.scratch;
		}

		GLenum Scratch() const
		{
			return Value(scratch);
		}

		void SetScratch(const GLenum val)
		{
			scratch = Index(val);
		}

		GLenum Value(int index) const
		{
			return values[index];
		}

		int Index(const GLenum val) const
		{
			for (size_t i = 0; i < count; ++i)
			{
				if (val == values[i])
					return i;
			}

			return -1;
		}
	};

	template<typename T, OptionalPrimitive<T> _Min, OptionalPrimitive<T> _Max, typename NodeType = T>
	struct OptionalRangeField
	{
		using Self = OptionalRangeField<T, _Min, _Max, NodeType>;
		inline static const OptionalPrimitive<T> Min = _Min;
		inline static const OptionalPrimitive<T> Max = _Max;

		OptionalPrimitive<T> def;
		OptionalPrimitive<T> scratch;
		Self* disk = nullptr;
		detail::Key value_key;
		detail::Key enable_key;
		const char* label;

		OptionalRangeField(OptionalPrimitive<T> def, detail::Key value_key, detail::Key enable_key, const char* label)
			: def(def), scratch(def), value_key(value_key), enable_key(enable_key), label(label) {}

		void Load(TOMLNode node)
		{
			scratch = def;
			if (enable_key != detail::Key::_ && value_key != detail::Key::_)
			{
				if (auto v = node[detail::encode_key(value_key)].value<NodeType>())
					scratch = MakeOpt(static_cast<T>(*v));

				scratch.has_value &= node[detail::encode_key(enable_key)].value_or(false);
			}
		}

		void Dump(toml::table& table) const
		{
			if (enable_key != detail::Key::_ && value_key != detail::Key::_)
			{
				table.insert_or_assign(detail::encode_key(enable_key), scratch.has_value);
				table.insert_or_assign(detail::encode_key(value_key), static_cast<NodeType>(scratch.value));
			}
		}

		void Isolate()
		{
			if (disk)
			{
				auto d = disk;
				disk = nullptr;
				d->Isolate();
			}
		}

		void Reset(Self& source)
		{
			disk = &source;
			source.disk = this;
			scratch = source.scratch;
		}

		bool Draw()
		{
			return DescIO::Draw(label, scratch, DISK_FIELD(this->disk), Min, Max);
		}
	};

	template<OptionalInt Min, OptionalInt Max>
	using OptionalIntField = OptionalRangeField<int, Min, Max, int64_t>;

	template<OptionalFloat Min, OptionalFloat Max>
	using OptionalFloatField = OptionalRangeField<float, Min, Max, double>;

}
