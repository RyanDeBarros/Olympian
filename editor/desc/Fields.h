#pragma once

#include "desc/DescIO.h"

#include "external/TOML.h"

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

#define RESET_FIELD(field) field.Reset(source.field);
#define RESET_FIELDS(generator) Isolate(); generator(RESET_FIELD)
#define ISOLATE_FIELD(field) field.Isolate();
#define ISOLATE_FIELDS(generator) generator(ISOLATE_FIELD)

#define DISK_FIELD(disk) (disk ? &disk->scratch : nullptr)

	extern bool KeyIsNull(detail::Key key);
	extern bool KeyIsNotNull(detail::Key key);

	template<typename T>
	void IsolateField(T& obj)
	{
		if (obj.disk)
		{
			auto disk = obj.disk;
			obj.disk = nullptr;
			disk->Isolate();
		}
	}

	template<typename T, typename Self>
	void ResetField(T& obj, Self& source)
	{
		obj.disk = &source;
		source.disk = static_cast<Self*>(&obj);
		obj.scratch = source.scratch;
	}

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
			if (KeyIsNotNull(key))
				scratch = static_cast<T>(node[detail::encode_key(key)].value_or(static_cast<NodeType>(def)));
			else
				scratch = def;
		}

		void Dump(toml::table& table) const
		{
			if (KeyIsNotNull(key))
				table.insert_or_assign(detail::encode_key(key), static_cast<NodeType>(scratch));
		}

		void Isolate()
		{
			IsolateField(*this);
		}

		void Reset(Self& source)
		{
			ResetField(*this, source);
		}
	};

	struct BoolField : public PrimitiveField<bool, BoolField>
	{
		using PrimitiveField<bool, BoolField>::PrimitiveField;

		bool Draw();
	};

	template<typename T, OptionalPrimitive<T> _Min, OptionalPrimitive<T> _Max, typename NodeType = T>
	struct RangeField : public PrimitiveField<T, RangeField<T, _Min, _Max>, NodeType>
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

	template<OptionalDouble Min, OptionalDouble Max>
	using DoubleField = RangeField<double, Min, Max>;

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

		bool Draw();
		void Load(TOMLNode node);
		void Dump(toml::table& table) const;
		void Isolate();
		void Reset(GLenumField& source);

		GLenum Scratch() const;
		void SetScratch(const GLenum val);

		GLenum Value(int index) const;
		int Index(const GLenum val) const;
	};
	
	struct StringField : public PrimitiveField<std::string, StringField>
	{
		using PrimitiveField<std::string, StringField>::PrimitiveField;

		bool Draw();
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
			: def(def), scratch(def), value_key(value_key), enable_key(enable_key), label(label)
		{
		}

		bool Draw()
		{
			return DescIO::Draw(label, scratch, DISK_FIELD(this->disk), Min, Max);
		}

		void Load(TOMLNode node)
		{
			scratch = def;
			if (KeyIsNotNull(enable_key) && KeyIsNotNull(value_key))
			{
				if (auto v = node[detail::encode_key(value_key)].value<NodeType>())
					scratch = MakeOpt(static_cast<T>(*v));

				scratch.has_value &= node[detail::encode_key(enable_key)].value_or(false);
			}
		}

		void Dump(toml::table& table) const
		{
			if (KeyIsNotNull(enable_key) && KeyIsNotNull(value_key))
			{
				table.insert_or_assign(detail::encode_key(enable_key), scratch.has_value);
				table.insert_or_assign(detail::encode_key(value_key), static_cast<NodeType>(scratch.value));
			}
		}

		void Isolate()
		{
			IsolateField(*this);
		}

		void Reset(Self& source)
		{
			ResetField(*this, source);
		}
	};

	template<OptionalInt Min, OptionalInt Max>
	using OptionalIntField = OptionalRangeField<int, Min, Max, int64_t>;

	template<OptionalFloat Min, OptionalFloat Max>
	using OptionalFloatField = OptionalRangeField<float, Min, Max, double>;

	template<OptionalDouble Min, OptionalDouble Max>
	using OptionalDoubleField = OptionalRangeField<double, Min, Max, double>;

	template<typename T, OptionalPrimitive<T> _Min, OptionalPrimitive<T> _Max, typename NodeType = T>
	struct CompactOptionalRangeField
	{
		using Self = CompactOptionalRangeField<T, _Min, _Max, NodeType>;
		inline static const OptionalPrimitive<T> Min = _Min;
		inline static const OptionalPrimitive<T> Max = _Max;

		OptionalPrimitive<T> def;
		OptionalPrimitive<T> scratch;
		Self* disk = nullptr;
		T nullopt;
		detail::Key key;
		const char* label;

		CompactOptionalRangeField(OptionalPrimitive<T> def, T nullopt, detail::Key key, const char* label)
			: def(def), scratch(def), nullopt(nullopt), key(key), label(label)
		{
		}

		bool Draw()
		{
			return DescIO::Draw(label, scratch, DISK_FIELD(this->disk), Min, Max);
		}

		void Load(TOMLNode node)
		{
			scratch = def;
			if (KeyIsNotNull(key))
			{
				if (auto v = node[detail::encode_key(key)].value<NodeType>())
					scratch = *v == nullopt ? MakeOpt<T>() : MakeOpt<T>(*v);
			}
		}

		void Dump(toml::table& table) const
		{
			if (KeyIsNotNull(key))
				table.insert_or_assign(detail::encode_key(key), scratch.has_value ? scratch.value : nullopt);
		}

		void Isolate()
		{
			IsolateField(*this);
		}

		void Reset(Self& source)
		{
			ResetField(*this, source);
		}
	};

	template<OptionalInt Min, OptionalInt Max>
	using CompactOptionalIntField = CompactOptionalRangeField<int, Min, Max, int>;

	template<OptionalFloat Min, OptionalFloat Max>
	using CompactOptionalFloatField = CompactOptionalRangeField<float, Min, Max, float>;

	struct ColorField
	{
		glm::vec4 def;
		glm::vec4 scratch;
		ColorField* disk = nullptr;
		detail::Key key;
		const char* label;

		ColorField(glm::vec4 def, detail::Key key, const char* label);

		bool Draw();
		void Load(TOMLNode node);
		void Dump(toml::table& table) const;
		void Isolate();
		void Reset(ColorField& source);
	};

	extern void LoadStringArray(TOMLNode node, std::string* strings, size_t count);
	extern toml::array DumpStringArray(const std::string* strings, size_t count);

	template<size_t N>
	struct StringArrayField
	{
		using Self = StringArrayField<N>;
		std::array<std::string, N> def;
		std::array<std::string, N> scratch;
		Self* disk = nullptr;
		detail::Key key;
		const char* label;

		StringArrayField(std::array<std::string, N>&& def, detail::Key key, const char* label) : def(def), scratch(std::move(def)), key(key), label(label) {}

		bool Draw()
		{
			return DescIO::Draw(label, scratch.data(), disk ? disk->scratch.data() : nullptr, N);
		}

		void Load(TOMLNode node)
		{
			scratch = def;
			if (KeyIsNotNull(key))
				LoadStringArray(node[detail::encode_key(key)], scratch.data(), N);
		}

		void Dump(toml::table& table) const
		{
			if (KeyIsNotNull(key))
				table.insert_or_assign(detail::encode_key(key), DumpStringArray(scratch.data(), N));
		}

		void Isolate()
		{
			IsolateField(*this);
		}

		void Reset(Self& source)
		{
			ResetField(*this, source);
		}
	};
}
