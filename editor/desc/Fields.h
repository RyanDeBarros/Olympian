#pragma once

#include "desc/DescIO.h"

namespace oly::editor
{
	struct Field
	{
		virtual ~Field() = default;

		virtual bool Draw() = 0;
		virtual void Load(TOMLNode node) = 0;
		virtual void Dump(toml::table& table) = 0;
	};

#define DRAW_FIELD(field) if (desc.field.Draw()) MarkDirty();
#define DRAW_FIELDS(generator) generator(DRAW_FIELD);
#define LOAD_FIELD(field) desc.field.Load(node);
#define LOAD_FIELDS(generator) generator(LOAD_FIELD)
#define DUMP_FIELD(field) desc.field.Dump(table);
#define DUMP_FIELDS(generator) generator(DUMP_FIELD)
#define RESET_FIELD(field) field.Reset(source.field);
#define RESET_FIELDS(generator) generator(RESET_FIELD)

	template<typename T>
	struct PrimitiveField : public Field
	{
		T def;
		T scratch;
		T* disk = nullptr;
		detail::Key key;
		const char* label;

		PrimitiveField(T def, detail::Key key, const char* label) : def(def), scratch(def), key(key), label(label) {}

		void Load(TOMLNode node) override
		{
			scratch = static_cast<T>(node[detail::encode_key(key)].value_or(def));
		}

		void Dump(toml::table& table) override
		{
			table.insert_or_assign(detail::encode_key(key), scratch);
		}

		void Reset(PrimitiveField<T>& source)
		{
			disk = &source.scratch;
			source.disk = &scratch;
			scratch = source.scratch;
		}
	};

	struct BoolField : public PrimitiveField<bool>
	{
		using PrimitiveField<bool>::PrimitiveField;

		bool Draw() override
		{
			return DescIO::Draw(label, scratch, disk);
		}
	};

	template<OptionalPrimitive<int> Min, OptionalPrimitive<int> Max>
	struct IntField : public PrimitiveField<int>
	{
		using PrimitiveField<int>::PrimitiveField;

		bool Draw() override
		{
			return DescIO::Draw(label, scratch, disk, Min.Opt(), Max.Opt());
		}
	};

	template<OptionalPrimitive<float> Min, OptionalPrimitive<float> Max>
	struct FloatField : public PrimitiveField<float>
	{
		using PrimitiveField<float>::PrimitiveField;

		bool Draw() override
		{
			return DescIO::Draw(label, scratch, disk, Min.Opt(), Max.Opt());
		}
	};

	template<typename E>
	struct EnumField : public PrimitiveField<E>
	{
		static_assert(std::is_enum_v<E>);

		using PrimitiveField<E>::PrimitiveField;

		bool Draw() override
		{
			return DescIO::Draw(this->label, this->scratch, this->disk);
		}
	};

	struct GLenumField : public Field
	{
		GLenum def;
		int scratch;
		int* disk = nullptr;
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

		bool Draw() override
		{
			return DescIO::Draw(label, scratch, disk, names, count);
		}

		void Load(TOMLNode node) override
		{
			if (disk)
				*disk = Index(static_cast<GLenum>(node[detail::encode_key(key)].value_or(def)));
			else
				scratch = Index(static_cast<GLenum>(node[detail::encode_key(key)].value_or(def)));
		}

		void Dump(toml::table& table) override
		{
			table.insert_or_assign(detail::encode_key(key), Scratch());
		}

		void Reset(GLenumField& source)
		{
			disk = &source.scratch;
			source.disk = &scratch;
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
}
