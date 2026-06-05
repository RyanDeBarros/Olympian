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

	template<typename T>
	struct PrimitiveField
	{
		T def;
		T scratch;
		PrimitiveField<T>* disk = nullptr;
		detail::Key key;
		const char* label;

		PrimitiveField(T def, detail::Key key, const char* label) : def(def), scratch(def), key(key), label(label) {}

		void Load(TOMLNode node)
		{
			if (key != detail::Key::_)
				scratch = static_cast<T>(node[detail::encode_key(key)].value_or(def));
			else
				scratch = def;
		}

		void Dump(toml::table& table) const
		{
			if (key != detail::Key::_)
				table.insert_or_assign(detail::encode_key(key), scratch);
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

		void Reset(PrimitiveField<T>& source)
		{
			disk = &source;
			source.disk = this;
			scratch = source.scratch;
		}
	};

	struct BoolField : public PrimitiveField<bool>
	{
		using PrimitiveField<bool>::PrimitiveField;

		bool Draw()
		{
			return DescIO::Draw(label, scratch, DISK_FIELD(this->disk));
		}
	};

	template<OptionalPrimitive<int> Min, OptionalPrimitive<int> Max>
	struct IntField : public PrimitiveField<int>
	{
		inline static OptionalPrimitive<int> Min = Min;
		inline static OptionalPrimitive<int> Max = Max;

		using PrimitiveField<int>::PrimitiveField;

		bool Draw()
		{
			return DescIO::Draw(label, scratch, DISK_FIELD(this->disk), Min.Opt(), Max.Opt());
		}
	};

	template<OptionalPrimitive<float> Min, OptionalPrimitive<float> Max>
	struct FloatField : public PrimitiveField<float>
	{
		inline static OptionalPrimitive<float> Min = Min;
		inline static OptionalPrimitive<float> Max = Max;

		using PrimitiveField<float>::PrimitiveField;

		bool Draw()
		{
			return DescIO::Draw(label, scratch, DISK_FIELD(this->disk), Min.Opt(), Max.Opt());
		}
	};

	template<typename E>
	struct EnumField : public PrimitiveField<E>
	{
		static_assert(std::is_enum_v<E>);

		using PrimitiveField<E>::PrimitiveField;

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
}
