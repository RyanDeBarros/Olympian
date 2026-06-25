#pragma once

#include "core/editor/LabelRegistry.h"

#include "desc/DescIO.h"
#include "desc/Serializer.h"
#include "desc/UndoActions.h"

#include "assets/TranslateKey.h"

#include "gui/DynamicList.h"

#include <array>

namespace oly::editor
{
#define _DRAW_FIELD(field) desc.field.Draw(path / desc.subpaths.field);
#define DRAW_FIELDS(GENERATOR) GENERATOR(_DRAW_FIELD);

#define _LOAD_FIELD(field) desc.field.Load(node);
#define LOAD_FIELDS(GENERATOR) GENERATOR(_LOAD_FIELD)

#define _DUMP_FIELD(field) desc.field.Dump(table);
#define DUMP_FIELDS(GENERATOR) GENERATOR(_DUMP_FIELD)

#define _SUBPATH_ENUM_ENTRY(field) _E_##field,
#define _SUBPATH_STRUCT_ENTRY(field) static constexpr DataPathStep field = _E_##field;
#define _SUBPATH_VISIT_PATH(field) case _E_##field: return field.VisitPath(path.Next(), type);
#define GENERATE_SUBPATHS(GENERATOR) \
		private: enum : DataPathStep { GENERATOR(_SUBPATH_ENUM_ENTRY) }; \
		public: struct { GENERATOR(_SUBPATH_STRUCT_ENTRY) } subpaths; \
		void* VisitPath(DataPath path, std::type_index type) \
		{ \
			if (path.Empty()) \
				return nullptr; \
			switch (path.Step()) \
			{ \
				GENERATOR(_SUBPATH_VISIT_PATH); \
			default: \
				return nullptr; \
			} \
		}

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
			else
				Serializer<T>{}.Load(scratch, node);
		}

		void Dump(toml::table& table) const
		{
			if (key != NullKey())
				table.insert_or_assign(detail::encode_key(key), Serializer<T>{}.Dump(scratch));
		}

		void Dump(toml::array& array) const
		{
			array.push_back(Serializer<T>{}.Dump(scratch));
		}

		void* VisitPath(DataPath path, std::type_index type)
		{
			if (type == typeid(decltype(scratch)) && path.Empty())
				return reinterpret_cast<void*>(&scratch);
			else
				return nullptr;
		}
	};

	struct BoolField : public PrimitiveField<bool>
	{
		using PrimitiveField<bool>::PrimitiveField;

		void Draw(DataPath path)
		{
			auto initial = scratch;
			DescIO::Draw(label, scratch, def);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, scratch);
		}
	};

	template<typename T, typename U, OptionalPrimitive<U> _Min, OptionalPrimitive<U> _Max>
	struct RangeField : public PrimitiveField<T>
	{
		inline static const OptionalPrimitive<U> Min = _Min;
		inline static const OptionalPrimitive<U> Max = _Max;

		using PrimitiveField<T>::PrimitiveField;

		void Draw(DataPath path)
		{
			auto initial = this->scratch;
			DescIO::Draw(this->label, this->scratch, this->def, Min, Max);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, this->scratch);
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

		void Draw(DataPath path)
		{
			auto initial = this->scratch;
			DescIO::Draw(this->label, this->scratch, this->def);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, this->scratch);
		}
	};

	struct StringField : public PrimitiveField<std::string>
	{
		using PrimitiveField<std::string>::PrimitiveField;

		void Draw(DataPath path)
		{
			auto initial = scratch;
			DescIO::Draw(label, scratch, def);
			if (gui::PropertyGrid::DirtyRow()) // TODO v9.1 && gui::PropertyGrid::Value::GetDrawResult().ItemDeactivatedAfterEdit()? -> also for StringVector/ArrayField, etc.
				PushFieldSetAction(path, initial, scratch);
		}
	};

	struct Color4Field : public PrimitiveField<Color4>
	{
		using PrimitiveField<Color4>::PrimitiveField;

		void Draw(DataPath path)
		{
			auto initial = scratch;
			DescIO::Draw(label, scratch, def);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, scratch);
		}
	};

	struct RectField : public PrimitiveField<Rect>
	{
		using PrimitiveField<Rect>::PrimitiveField;

		void Draw(DataPath path)
		{
			auto initial = scratch;
			DescIO::Draw(label, scratch, def);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, scratch);
		}
	};

	struct UVRectField : public PrimitiveField<UVRect>
	{
		using PrimitiveField<UVRect>::PrimitiveField;

		void Draw(DataPath path)
		{
			auto initial = scratch;
			DescIO::Draw(label, scratch, def);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, scratch);
		}
	};

	struct TopSidePaddingField : public PrimitiveField<TopSidePadding>
	{
		using PrimitiveField<TopSidePadding>::PrimitiveField;

		void Draw(DataPath path)
		{
			auto initial = scratch;
			DescIO::Draw(label, scratch, def);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, scratch);
		}
	};

	template<typename T, size_t N>
	struct ArrayField : public PrimitiveField<std::array<T, N>>
	{
		const char** sublabels;

		ArrayField(std::array<T, N> def, detail::Key key, const char* label, const char* (&sublabels)[N])
			: PrimitiveField<std::array<T, N>>(def, key, label), sublabels(sublabels) {}

		void Draw(DataPath path)
		{
			auto initial = this->scratch;
			DescIO::Draw(this->label, this->scratch.data(), this->def.data(), sublabels, N);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, this->scratch);
		}
	};

	template<size_t N>
	using BoolArrayField = ArrayField<bool, N>;

	template<typename T, size_t N>
	struct AnonArrayField : public PrimitiveField<std::array<T, N>>
	{
		using PrimitiveField<std::array<T, N>>::PrimitiveField;

		void Draw(DataPath path)
		{
			auto initial = this->scratch;
			DescIO::Draw(this->label, this->scratch.data(), this->def.data(), N);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, this->scratch);
		}
	};

	template<size_t N>
	using StringArrayField = AnonArrayField<std::string, N>;

	template<typename T>
	struct VectorField : public PrimitiveField<std::vector<T>>
	{
		gui::DynamicListState ui_state;

		using PrimitiveField<std::vector<T>>::PrimitiveField;

		void Draw(DataPath path)
		{
			auto initial = this->scratch;
			DescIO::Draw(this->label, this->scratch, this->def, ui_state);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, this->scratch);
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
		LabelSpanRegistry::Handle names;
		size_t count;

		template<size_t Count>
		DisjointEnumField(E def, detail::Key key, const char* label, const E (&values)[Count], const char* (&names)[Count])
			: def(def), key(key), label(label), values(values), names(LabelSpanRegistry::Intern(std::span<const char*>(names, Count))), count(Count)
		{
			SetScratch(def);
			def_index = Index(def);
		}

		void Draw(DataPath path)
		{
			auto initial = scratch;
			DescIO::Draw(label, scratch, def_index, names);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, scratch);
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

		void* VisitPath(DataPath path, std::type_index type)
		{
			if (type == typeid(decltype(scratch)) && path.Empty())
				return reinterpret_cast<void*>(&scratch);
			else
				return nullptr;
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

		void Draw(DataPath path)
		{
			auto initial = scratch;
			DescIO::Draw(label, scratch, def, Min, Max);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, scratch);
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

		void* VisitPath(DataPath path, std::type_index type)
		{
			if (type == typeid(decltype(scratch)) && path.Empty())
				return reinterpret_cast<void*>(&scratch);
			else
				return nullptr;
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

		void Draw(DataPath path)
		{
			scratch.has_value = scratch.value != nullopt;
			auto initial = scratch;
			DescIO::Draw(label, scratch, def, Min, Max);
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSetAction(path, initial, scratch);
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

		void* VisitPath(DataPath path, std::type_index type)
		{
			if (type == typeid(decltype(scratch)) && path.Empty())
				return reinterpret_cast<void*>(&scratch);
			else
				return nullptr;
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
		bool def_flags[Count];
		bool scratch_flags[Count];
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
			SetFlags();
		}

		void Draw(DataPath path, const bool (&disabled)[Count])
		{
			return Draw(path, static_cast<const bool*>(disabled));
		}

		void Draw(DataPath path)
		{
			return Draw(path, nullptr);
		}

	private:
		void Draw(DataPath path, const bool* disabled)
		{
			auto initial = scratch;
			SetFlags();
			DescIO::Draw(label, scratch_flags, def_flags, names, disabled, Count);
			SetEnum();
			if (gui::PropertyGrid::DirtyRow())
				PushFieldSyncSetAction(path, initial, scratch, [this]() { SetFlags(); });
		}

		void SetFlags()
		{
			for (size_t i = 0; i < Count; ++i)
			{
				scratch_flags[i] = static_cast<bool>(scratch & values[i]);
				def_flags[i] = static_cast<bool>(def & values[i]);
			}
		}

		void SetEnum()
		{
			for (size_t i = 0; i < Count; ++i)
			{
				if (scratch_flags[i])
					scratch |= values[i];
				else
					scratch &= ~values[i];
			}
		}

	public:
		void Load(TOMLNode node)
		{
			scratch = def;
			Serializer<E>{}.Load(scratch, node[detail::encode_key(key)]);
		}

		void Dump(toml::table& table) const
		{
			table.insert_or_assign(detail::encode_key(key), Serializer<E>{}.Dump(scratch));
		}

		void* VisitPath(DataPath path, std::type_index type)
		{
			if (type == typeid(decltype(scratch)) && path.Empty())
				return reinterpret_cast<void*>(&scratch);
			else
				return nullptr;
		}
	};
}
