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
#define DRAW_FIELD(field) desc.field.Draw(path / desc.subpaths.field);
#define DRAW_FIELDS(GENERATOR) GENERATOR(DRAW_FIELD);

#define LOAD_FIELD(field) desc.field.Load(node);
#define LOAD_FIELDS(GENERATOR) GENERATOR(LOAD_FIELD)

#define DUMP_FIELD(field) desc.field.Dump(table);
#define DUMP_FIELDS(GENERATOR) GENERATOR(DUMP_FIELD)

#define _SUBPATH_ENUM_ENTRY(field) _E_##field,
#define _SUBPATH_STRUCT_ENTRY(field) static constexpr DataPathStep field = DataPathStep(_E_##field);
#define _SUBPATH_VISIT_PATH(field) case _E_##field: return field.VisitPath(path.Next(), type);
#define _SUBPATH_DRAW_FINALIZE(field) dirty |= field.DrawFinalize(path / subpaths.field);
#define _SUBPATH_QUERY_DIRTY(field) if (field.QueryDirty(disk.field)) return true;
#define DESCRIPTOR_BODY(Klass, GENERATOR) \
		private: enum : int { GENERATOR(_SUBPATH_ENUM_ENTRY) }; \
		public: struct { GENERATOR(_SUBPATH_STRUCT_ENTRY) } subpaths; \
		void* VisitPath(DataPath path, std::type_index type) \
		{ \
			if (path.Empty()) \
				return nullptr; \
			switch (path.Step().v) \
			{ \
				GENERATOR(_SUBPATH_VISIT_PATH); \
			default: \
				return nullptr; \
			} \
		} \
		bool DrawFinalize(DataPath path) { bool dirty = false; GENERATOR(_SUBPATH_DRAW_FINALIZE); return dirty; } \
		bool QueryDirty(const Klass& disk) const { GENERATOR(_SUBPATH_QUERY_DIRTY); return false; }

	extern detail::Key NullKey();

	template<typename T, typename Self>
	struct PrimitiveField
	{
		using SelfType = std::conditional_t<std::is_void_v<Self>, PrimitiveField<T, Self>, Self>;

		T def;
		T scratch; // TODO v9.1 rename 'scratch' to 'value'
		detail::Key key;
		const char* label;

		PrimitiveField(T def, detail::Key key, const char* label) : def(def), scratch(def), key(key), label(label) {}

		bool QueryDirty(const SelfType& disk) const
		{
			return scratch != disk.scratch;
		}

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
		
		bool DrawFinalize(DataPath path)
		{
			return false;
		}
	};

	struct BoolField : public PrimitiveField<bool, BoolField>
	{
		using PrimitiveField<bool, BoolField>::PrimitiveField;

		void Draw(DataPath path)
		{
			const auto initial = scratch;
			DescIO::Draw(label, scratch, def);
			if (initial != scratch)
				PushFieldSetAction(path, initial, scratch);
		}
	};

	template<typename T, typename U, OptionalPrimitive<U> _Min, OptionalPrimitive<U> _Max>
	struct RangeField : public PrimitiveField<T, RangeField<T, U, _Min, _Max>>
	{
		using Super = PrimitiveField<T, RangeField<T, U, _Min, _Max>>;

		inline static const OptionalPrimitive<U> Min = _Min;
		inline static const OptionalPrimitive<U> Max = _Max;

		EditSession<T> edit;

		RangeField(T def, detail::Key key, const char* label) : Super(def, key, label), edit(this->scratch) {}

		RangeField(const RangeField& o)
			: Super(o), edit(this->scratch)
		{
		}

		RangeField(RangeField&& o) noexcept
			: Super(std::move(o)), edit(this->scratch)
		{
		}

		RangeField& operator=(const RangeField& o)
		{
			if (this != &o)
				Super::operator=(o);

			return *this;
		}

		RangeField& operator=(RangeField&& o) noexcept
		{
			if (this != &o)
				Super::operator=(std::move(o));

			return *this;
		}

		void Draw(DataPath path)
		{
			DescIO::Draw(this->label, this->edit, this->def, Min, Max);
			CheckUndoAction(path);
		}

		bool CheckUndoAction(DataPath path)
		{
			if (edit.ConsumeModified())
			{
				PushFieldSetAction(path, edit.buffer, this->scratch);
				return true;
			}
			else
				return false;
		}

		bool DrawFinalize(DataPath path)
		{
			edit.DrawFinalize();
			return CheckUndoAction(path);
		}
	};

	template<OptionalInt Min, OptionalInt Max>
	using IntField = RangeField<int, int, Min, Max>;

	template<OptionalFloat Min, OptionalFloat Max>
	using FloatField = RangeField<float, float, Min, Max>;

	template<OptionalDouble Min, OptionalDouble Max>
	using DoubleField = RangeField<double, double, Min, Max>;

	template<typename E>
	struct EnumField : public PrimitiveField<E, EnumField<E>>
	{
		static_assert(std::is_enum_v<E>);

		using PrimitiveField<E, EnumField<E>>::PrimitiveField;

		void Draw(DataPath path)
		{
			const auto initial = this->scratch;
			DescIO::Draw(this->label, this->scratch, this->def);
			if (initial != this->scratch)
				PushFieldSetAction(path, initial, this->scratch);
		}
	};

	struct StringField : public PrimitiveField<std::string, StringField>
	{
		EditSession<std::string> edit;

		StringField(std::string def, detail::Key key, const char* label) : PrimitiveField(std::move(def), key, label), edit(scratch) {}

		StringField(const StringField& o)
			: PrimitiveField(o), edit(scratch)
		{
		}

		StringField(StringField&& o) noexcept
			: PrimitiveField(std::move(o)), edit(scratch)
		{
		}

		StringField& operator=(const StringField& o)
		{
			if (this != &o)
				PrimitiveField::operator=(o);

			return *this;
		}

		StringField& operator=(StringField&& o) noexcept
		{
			if (this != &o)
				PrimitiveField::operator=(std::move(o));

			return *this;
		}

		void Draw(DataPath path)
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction(path);
		}

		bool CheckUndoAction(DataPath path)
		{
			if (edit.ConsumeModified())
			{
				PushFieldSetAction(path, std::move(edit.buffer), scratch);
				return true;
			}
			else
				return false;
		}

		bool DrawFinalize(DataPath path)
		{
			edit.DrawFinalize();
			return CheckUndoAction(path);
		}
	};

	struct Color4Field : public PrimitiveField<Color4, Color4Field>
	{
		EditSession<Color4> edit;

		Color4Field(Color4 def, detail::Key key, const char* label) : PrimitiveField(def, key, label), edit(scratch)
		{
		}

		Color4Field(const Color4Field& o)
			: PrimitiveField(o), edit(scratch)
		{
		}

		Color4Field(Color4Field&& o) noexcept
			: PrimitiveField(std::move(o)), edit(scratch)
		{
		}

		Color4Field& operator=(const Color4Field& o)
		{
			if (this != &o)
				PrimitiveField::operator=(o);

			return *this;
		}

		Color4Field& operator=(Color4Field&& o) noexcept
		{
			if (this != &o)
				PrimitiveField::operator=(std::move(o));

			return *this;
		}

		void Draw(DataPath path)
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction(path);
		}

		bool CheckUndoAction(DataPath path)
		{
			if (edit.ConsumeModified())
			{
				PushFieldSetAction(path, edit.buffer, scratch);
				return true;
			}
			else
				return false;
		}

		bool DrawFinalize(DataPath path)
		{
			edit.DrawFinalize();
			return CheckUndoAction(path);
		}
	};

	struct RectField : public PrimitiveField<Rect, RectField>
	{
		EditSession<Rect> edit;

		RectField(Rect def, detail::Key key, const char* label) : PrimitiveField(def, key, label), edit(scratch) {}

		RectField(const RectField& o)
			: PrimitiveField(o), edit(scratch)
		{
		}

		RectField(RectField&& o) noexcept
			: PrimitiveField(std::move(o)), edit(scratch)
		{
		}

		RectField& operator=(const RectField& o)
		{
			if (this != &o)
				PrimitiveField::operator=(o);

			return *this;
		}

		RectField& operator=(RectField&& o) noexcept
		{
			if (this != &o)
				PrimitiveField::operator=(std::move(o));

			return *this;
		}

		void Draw(DataPath path)
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction(path);
		}

		bool CheckUndoAction(DataPath path)
		{
			if (edit.ConsumeModified())
			{
				PushFieldSetAction(path, edit.buffer, scratch);
				return true;
			}
			else
				return false;
		}

		bool DrawFinalize(DataPath path)
		{
			edit.DrawFinalize();
			return CheckUndoAction(path);
		}
	};

	struct UVRectField : public PrimitiveField<UVRect, UVRectField>
	{
		EditSession<UVRect> edit;

		UVRectField(UVRect def, detail::Key key, const char* label) : PrimitiveField(def, key, label), edit(scratch) {}

		UVRectField(const UVRectField& o)
			: PrimitiveField(o), edit(scratch)
		{
		}

		UVRectField(UVRectField&& o) noexcept
			: PrimitiveField(std::move(o)), edit(scratch)
		{
		}

		UVRectField& operator=(const UVRectField& o)
		{
			if (this != &o)
				PrimitiveField::operator=(o);

			return *this;
		}

		UVRectField& operator=(UVRectField&& o) noexcept
		{
			if (this != &o)
				PrimitiveField::operator=(std::move(o));

			return *this;
		}

		void Draw(DataPath path)
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction(path);
		}

		bool CheckUndoAction(DataPath path)
		{
			if (edit.ConsumeModified())
			{
				PushFieldSetAction(path, edit.buffer, scratch);
				return true;
			}
			else
				return false;
		}

		bool DrawFinalize(DataPath path)
		{
			edit.DrawFinalize();
			return CheckUndoAction(path);
		}
	};

	struct TopSidePaddingField : public PrimitiveField<TopSidePadding, TopSidePaddingField>
	{
		EditSession<TopSidePadding> edit;

		TopSidePaddingField(TopSidePadding def, detail::Key key, const char* label) : PrimitiveField(def, key, label), edit(scratch) {}

		TopSidePaddingField(const TopSidePaddingField& o)
			: PrimitiveField(o), edit(scratch)
		{
		}

		TopSidePaddingField(TopSidePaddingField&& o) noexcept
			: PrimitiveField(std::move(o)), edit(scratch)
		{
		}

		TopSidePaddingField& operator=(const TopSidePaddingField& o)
		{
			if (this != &o)
				PrimitiveField::operator=(o);

			return *this;
		}

		TopSidePaddingField& operator=(TopSidePaddingField&& o) noexcept
		{
			if (this != &o)
				PrimitiveField::operator=(std::move(o));

			return *this;
		}

		void Draw(DataPath path)
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction(path);
		}

		bool CheckUndoAction(DataPath path)
		{
			if (edit.ConsumeModified())
			{
				PushFieldSetAction(path, edit.buffer, scratch);
				return true;
			}
			else
				return false;
		}

		bool DrawFinalize(DataPath path)
		{
			edit.DrawFinalize();
			return CheckUndoAction(path);
		}
	};

	template<typename T, size_t N>
	struct ArrayField : public PrimitiveField<std::array<T, N>, ArrayField<T, N>>
	{
		using Super = PrimitiveField<std::array<T, N>, ArrayField<T, N>>;

		const char** sublabels;

		ArrayField(std::array<T, N> def, detail::Key key, const char* label, const char* (&sublabels)[N])
			: Super(def, key, label), sublabels(sublabels) {}

		void Draw(DataPath path)
		{
			const auto initial = this->scratch;
			DescIO::Draw(this->label, this->scratch.data(), this->def.data(), sublabels, N);
			if (initial != this->scratch)
				PushFieldSetAction(path, initial, this->scratch);
		}
	};

	template<size_t N>
	using BoolArrayField = ArrayField<bool, N>;

	template<typename T, size_t N>
	struct AnonArrayField : public PrimitiveField<std::array<T, N>, AnonArrayField<T, N>>
	{
		using Super = PrimitiveField<std::array<T, N>, AnonArrayField<T, N>>;

		std::array<EditSession<T>, N> edits;

		AnonArrayField(std::array<T, N> def, detail::Key key, const char* label)
			: Super(def, key, label), edits(_MakeEdits(this->scratch, std::make_index_sequence<N>{})) {}

	private:
		template<size_t... Is>
		static std::array<EditSession<T>, N> _MakeEdits(std::array<T, N>& scratch, std::index_sequence<Is...>)
		{
			return { EditSession<T>{scratch[Is]}... };
		}

	public:
		AnonArrayField(const AnonArrayField& o)
			: Super(o), edits(_MakeEdits(this->scratch, std::make_index_sequence<N>{}))
		{
		}

		AnonArrayField(AnonArrayField&& o) noexcept
			: Super(std::move(o)), edits(_MakeEdits(this->scratch, std::make_index_sequence<N>{}))
		{
		}

		AnonArrayField& operator=(const AnonArrayField& o)
		{
			if (this != &o)
				Super::operator=(o);

			return *this;
		}

		AnonArrayField& operator=(AnonArrayField&& o) noexcept
		{
			if (this != &o)
				Super::operator=(std::move(o));

			return *this;
		}

		void Draw(DataPath path)
		{
			DescIO::Draw(this->label, edits.data(), this->def.data(), N);
			CheckUndoAction(path);
		}

		bool CheckUndoAction(DataPath path)
		{
			bool modified = false;
			for (size_t i = 0; i < N; ++i)
			{
				if (edits[i].ConsumeModified())
				{
					modified = true;
					PushFieldSetAction(path / DataPathStep(i), std::move(edits[i].buffer), this->scratch[i]);
				}
			}
			return modified;
		}

		void* VisitPath(DataPath path, std::type_index type)
		{
			if (path.Empty())
				return nullptr;

			auto index = path.Step().v;
			if (index >= 0 && index < N)
			{
				path = path.Next();
				if (type == typeid(this->scratch[index]) && path.Empty())
					return reinterpret_cast<void*>(&this->scratch[index]);
				else
					return nullptr;
			}
			else
				return nullptr;
		}

		bool DrawFinalize(DataPath path)
		{
			for (auto& edit : edits)
				edit.DrawFinalize();
			return CheckUndoAction(path);
		}
	};

	template<size_t N>
	using StringArrayField = AnonArrayField<std::string, N>;

	template<typename T>
	struct VectorField : public PrimitiveField<std::vector<T>, VectorField<T>>
	{
		gui::DynamicListState ui_state;

		using PrimitiveField<std::vector<T>, VectorField<T>>::PrimitiveField;
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
			const auto initial = scratch;
			DescIO::Draw(label, scratch, def_index, names);
			if (initial != scratch)
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

		bool DrawFinalize(DataPath path)
		{
			return false;
		}

		bool QueryDirty(const DisjointEnumField<E>& disk) const
		{
			return scratch != disk.scratch;
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
		EditSession<OptionalPrimitive<T>> edit;
		detail::Key value_key;
		detail::Key enable_key;
		const char* label;

		OptionalRangeField(OptionalPrimitive<T> def, detail::Key value_key, detail::Key enable_key, const char* label)
			: def(def), scratch(def), edit(scratch), value_key(value_key), enable_key(enable_key), label(label)
		{
		}

		OptionalRangeField(const OptionalRangeField& o)
			: def(o.def), scratch(o.scratch), edit(scratch), value_key(o.value_key), enable_key(o.enable_key), label(o.label)
		{
		}

		OptionalRangeField(OptionalRangeField&& o)
			: def(std::move(o.def)), scratch(std::move(o.scratch)), edit(scratch), value_key(o.value_key), enable_key(o.enable_key), label(o.label)
		{
		}

		OptionalRangeField& operator=(const OptionalRangeField& o)
		{
			if (this != &o)
			{
				def = o.def;
				scratch = o.scratch;
				value_key = o.value_key;
				enable_key = o.enable_key;
				label = o.label;
			}

			return *this;
		}

		OptionalRangeField& operator=(OptionalRangeField&& o)
		{
			if (this != &o)
			{
				def = std::move(o.def);
				scratch = std::move(o.scratch);
				value_key = o.value_key;
				enable_key = o.enable_key;
				label = o.label;
			}

			return *this;
		}

		void Draw(DataPath path)
		{
			DescIO::Draw(label, edit, def, Min, Max);
			CheckUndoAction(path);
		}

		bool CheckUndoAction(DataPath path)
		{
			if (edit.ConsumeModified())
			{
				PushFieldSetAction(path, edit.buffer, scratch);
				return true;
			}
			else
				return false;
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

		bool DrawFinalize(DataPath path)
		{
			edit.DrawFinalize();
			return CheckUndoAction(path);
		}

		bool QueryDirty(const OptionalRangeField& disk) const
		{
			return scratch != disk.scratch;
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
		EditSession<OptionalPrimitive<T>> edit;
		T nullopt;
		detail::Key key;
		const char* label;

		CompactOptionalRangeField(OptionalPrimitive<T> def, T nullopt, detail::Key key, const char* label)
			: def(def), scratch(def), edit(scratch), nullopt(nullopt), key(key), label(label)
		{
		}

		CompactOptionalRangeField(const CompactOptionalRangeField& o)
			: def(o.def), scratch(o.scratch), edit(scratch), nullopt(o.nullopt), key(o.key), label(o.label)
		{
		}

		CompactOptionalRangeField(CompactOptionalRangeField&& o) noexcept
			: def(std::move(o.def)), scratch(std::move(o.scratch)), edit(scratch), nullopt(o.nullopt), key(o.key), label(o.label)
		{
		}

		CompactOptionalRangeField& operator=(const CompactOptionalRangeField& o)
		{
			if (this != &o)
			{
				def = o.def;
				scratch = o.scratch;
				nullopt = o.nullopt;
				key = o.key;
				label = o.label;
			}

			return *this;
		}

		CompactOptionalRangeField& operator=(CompactOptionalRangeField&& o) noexcept
		{
			if (this != &o)
			{
				std::move(def) = std::move(o.def);
				std::move(scratch) = std::move(o.scratch);
				nullopt = o.nullopt;
				key = o.key;
				label = o.label;
			}

			return *this;
		}

		void Draw(DataPath path)
		{
			DescIO::Draw(label, edit, def, Min, Max);
			CheckUndoAction(path);
		}

		bool CheckUndoAction(DataPath path)
		{
			if (edit.ConsumeModified())
			{
				PushFieldSetAction(path, edit.buffer, scratch);
				return true;
			}
			else
				return false;
		}

		void Load(TOMLNode node)
		{
			scratch = def;
			if (key != NullKey())
			{
				T value = def.value;
				if (Serializer<T>{}.Load(value, node[detail::encode_key(key)]))
				{
					scratch.has_value = value != nullopt;
					if (scratch.has_value)
						scratch.value = value;
				}
				else
					scratch.has_value = false;
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

		bool DrawFinalize(DataPath path)
		{
			edit.DrawFinalize();
			return CheckUndoAction(path);
		}

		bool QueryDirty(const CompactOptionalRangeField& disk) const
		{
			return scratch != disk.scratch;
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
			const auto initial = scratch;
			SetFlags();
			DescIO::Draw(label, scratch_flags, def_flags, names, disabled, Count);
			SetEnum();
			if (initial != scratch)
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

		bool DrawFinalize(DataPath path)
		{
			return false;
		}

		bool QueryDirty(const BitsetField<E, Count>& disk) const
		{
			return scratch != disk.scratch;
		}
	};
}
