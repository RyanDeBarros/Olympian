#pragma once

#include "core/editor/LabelRegistry.h"

#include "desc/DescIO.h"
#include "desc/Serializer.h"
#include "desc/FieldSetAction.h"

#include "assets/TranslateKey.h"

#include "gui/DynamicList.h"

#include <array>

namespace oly::editor
{
#define DRAW_FIELD(field) desc.field.Draw();
#define DRAW_FIELDS(GENERATOR) GENERATOR(DRAW_FIELD);

#define LOAD_FIELD(field) desc.field.Load(node);
#define LOAD_FIELDS(GENERATOR) GENERATOR(LOAD_FIELD)

#define DUMP_FIELD(field) desc.field.Dump(table);
#define DUMP_FIELDS(GENERATOR) GENERATOR(DUMP_FIELD)

	namespace internal
	{
		template<typename T>
		void PrintDescPath(std::ostream& os, DataPath path, const char* name, const T& field)
		{
			if constexpr (requires(T t, std::ostream os, DataPath path) { t.PrintPath(os, path); })
			{
				os << name << ".";
				if (path.Empty())
					os << "<error>";
				else
					field.PrintPath(os, path);
			}
			else
				os << name;
		}
	}

#define _SUBPATH_ENUM_ENTRY(field) _E_##field,
#define _SUBPATH_STRUCT_ENTRY(field) static constexpr DataPathStep field = DataPathStep(_E_##field);
#define _SUBPATH_PATH_GET(field) case _E_##field: return field.PathGet(path.Next(), type);
#define _SUBPATH_PRINT_PATH(field) case _E_##field: internal::PrintDescPath(os, path.Next(), #field, field); break;
#define _SUBPATH_QUERY_DIRTY(field) if (field.QueryDirty(disk.field)) return true;
#define _SUBPATH_COPY_DATA(field) field.CopyData(o.field);
	// TODO v9.3 make note that DESCRIPTOR_BODY() must be the first thing in the descriptor class definition, or at least before the generator members are declared - once moved to imtk
#define DESCRIPTOR_BODY(Klass, GENERATOR) \
		public: DataPathLink link; \
		private: enum : int { GENERATOR(_SUBPATH_ENUM_ENTRY) }; \
		public: struct { GENERATOR(_SUBPATH_STRUCT_ENTRY) } subpaths; \
		void* PathGet(DataPath path, std::type_index type) \
		{ \
			if (path.Empty()) \
				return typeid(decltype(*this)) == type ? static_cast<void*>(this) : nullptr; \
			switch (path.Step().v) \
			{ \
				GENERATOR(_SUBPATH_PATH_GET); \
			default: \
				return nullptr; \
			} \
		} \
		void PrintPath(std::ostream& os, DataPath path) const \
		{ \
			if (path.Empty()) \
				os << "<error>"; \
			else \
			{ \
				switch (path.Step().v) \
				{ \
					GENERATOR(_SUBPATH_PRINT_PATH); \
				default: \
					os << "<error>"; \
				} \
			} \
		} \
		bool QueryDirty(const Klass& disk) const { GENERATOR(_SUBPATH_QUERY_DIRTY); return false; } \
		void CopyData(const Klass& o) { GENERATOR(_SUBPATH_COPY_DATA); }

	extern detail::Key NullKey();

	template<typename T>
	struct PrimitiveField
	{
		DataPathLink link;
		T def;
		T value;
		detail::Key key;
		const char* label;

		PrimitiveField(DataPathLink link, T def, detail::Key key, const char* label) : link(std::move(link)), def(def), value(def), key(key), label(label) {}

		void CopyData(const PrimitiveField& o)
		{
			value = o.value;
		}

		bool QueryDirty(const PrimitiveField& disk) const
		{
			return value != disk.value;
		}

		void Load(TOMLNode node)
		{
			value = def;
			if (key != NullKey())
				Serializer<T>{}.Load(value, node[detail::encode_key(key)]);
			else
				Serializer<T>{}.Load(value, node);
		}

		void Dump(toml::table& table) const
		{
			if (key != NullKey())
				table.insert_or_assign(detail::encode_key(key), Serializer<T>{}.Dump(value));
		}

		void Dump(toml::array& array) const
		{
			array.push_back(Serializer<T>{}.Dump(value));
		}

		void* PathGet(DataPath path, std::type_index type)
		{
			if (type == typeid(decltype(value)) && path.Empty())
				return static_cast<void*>(&value);
			else
				return nullptr;
		}
	};

	struct BoolField : public PrimitiveField<bool>
	{
		using PrimitiveField<bool>::PrimitiveField;

		void Draw()
		{
			const auto initial = value;
			DescIO::Draw(label, value, def);
			if (initial != value)
				PushFieldSetAction(link.ComputePath(), initial, value);
		}
	};

	template<typename T, typename U, OptionalPrimitive<U> _Min, OptionalPrimitive<U> _Max>
	struct RangeField : public PrimitiveField<T>, public imtk::tick_processor
	{
		using Super = PrimitiveField<T>;

		inline static const OptionalPrimitive<U> Min = _Min;
		inline static const OptionalPrimitive<U> Max = _Max;

		EditSession<T> edit;

		RangeField(DataPathLink link, T def, detail::Key key, const char* label) : Super(std::move(link), def, key, label), edit(this->value) {}

		RangeField(RangeField&& o) noexcept
			: Super(std::move(o)), edit(this->value)
		{
		}

		RangeField& operator=(RangeField&& o) noexcept
		{
			if (this != &o)
				Super::operator=(std::move(o));

			return *this;
		}

		// TODO v9.3 CopyData() for fields that have edit sessions that directly copies edit?

		void Draw()
		{
			DescIO::Draw(this->label, this->edit, this->def, Min, Max);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.ConsumeModified())
				PushFieldSetAction(this->link.ComputePath(), std::move(edit.original), this->value);
		}

		void on_last_process_frame() override
		{
			edit.on_last_process_frame();
			CheckUndoAction();
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

		void Draw()
		{
			const auto initial = this->value;
			DescIO::Draw(this->label, this->value, this->def);
			if (initial != this->value)
				PushFieldSetAction(this->link.ComputePath(), initial, this->value);
		}
	};

	struct StringField : public PrimitiveField<std::string>, public imtk::tick_processor
	{
		EditSession<std::string> edit;

		StringField(DataPathLink link, std::string def, detail::Key key, const char* label) : PrimitiveField(std::move(link), std::move(def), key, label), edit(value) {}

		StringField(StringField&& o) noexcept
			: PrimitiveField(std::move(o)), edit(value)
		{
		}

		StringField& operator=(StringField&& o) noexcept
		{
			if (this != &o)
				PrimitiveField::operator=(std::move(o));

			return *this;
		}

		void Draw()
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.ConsumeModified())
				PushFieldSetAction(link.ComputePath(), std::move(edit.original), value);
		}

		void on_last_process_frame() override
		{
			edit.on_last_process_frame();
			CheckUndoAction();
		}
	};

	struct Color4Field : public PrimitiveField<Color4>, public imtk::tick_processor
	{
		EditSession<Color4> edit;

		Color4Field(DataPathLink link, Color4 def, detail::Key key, const char* label) : PrimitiveField(std::move(link), def, key, label), edit(value) {}

		Color4Field(Color4Field&& o) noexcept
			: PrimitiveField(std::move(o)), edit(value)
		{
		}

		Color4Field& operator=(Color4Field&& o) noexcept
		{
			if (this != &o)
				PrimitiveField::operator=(std::move(o));

			return *this;
		}

		void Draw()
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.ConsumeModified())
				PushFieldSetAction(link.ComputePath(), std::move(edit.original), value);
		}

		void on_last_process_frame() override
		{
			edit.on_last_process_frame();
			CheckUndoAction();
		}
	};

	struct RectField : public PrimitiveField<Rect>, public imtk::tick_processor
	{
		EditSession<Rect> edit;

		RectField(DataPathLink link, Rect def, detail::Key key, const char* label) : PrimitiveField(std::move(link), def, key, label), edit(value) {}

		RectField(RectField&& o) noexcept
			: PrimitiveField(std::move(o)), edit(value)
		{
		}

		RectField& operator=(RectField&& o) noexcept
		{
			if (this != &o)
				PrimitiveField::operator=(std::move(o));

			return *this;
		}

		void Draw()
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.ConsumeModified())
				PushFieldSetAction(link.ComputePath(), std::move(edit.original), value);
		}

		void on_last_process_frame() override
		{
			edit.on_last_process_frame();
			CheckUndoAction();
		}
	};

	struct UVRectField : public PrimitiveField<UVRect>, public imtk::tick_processor
	{
		EditSession<UVRect> edit;

		UVRectField(DataPathLink link, UVRect def, detail::Key key, const char* label) : PrimitiveField(std::move(link), def, key, label), edit(value) {}

		UVRectField(UVRectField&& o) noexcept
			: PrimitiveField(std::move(o)), edit(value)
		{
		}

		UVRectField& operator=(UVRectField&& o) noexcept
		{
			if (this != &o)
				PrimitiveField::operator=(std::move(o));

			return *this;
		}

		void Draw()
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.ConsumeModified())
				PushFieldSetAction(link.ComputePath(), std::move(edit.original), value);
		}

		void on_last_process_frame() override
		{
			edit.on_last_process_frame();
			CheckUndoAction();
		}
	};

	struct TopSidePaddingField : public PrimitiveField<TopSidePadding>, public imtk::tick_processor
	{
		EditSession<TopSidePadding> edit;

		TopSidePaddingField(DataPathLink link, TopSidePadding def, detail::Key key, const char* label) : PrimitiveField(std::move(link), def, key, label), edit(value) {}

		TopSidePaddingField(TopSidePaddingField&& o) noexcept
			: PrimitiveField(std::move(o)), edit(value)
		{
		}

		TopSidePaddingField& operator=(TopSidePaddingField&& o) noexcept
		{
			if (this != &o)
				PrimitiveField::operator=(std::move(o));

			return *this;
		}

		void Draw()
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.ConsumeModified())
				PushFieldSetAction(link.ComputePath(), std::move(edit.original), value);
		}

		void on_last_process_frame() override
		{
			edit.on_last_process_frame();
			CheckUndoAction();
		}
	};

	template<typename T, size_t N>
	struct ArrayField : public PrimitiveField<std::array<T, N>>
	{
		using Super = PrimitiveField<std::array<T, N>>;

		const char** sublabels;
		bool inline_checkboxes;

		ArrayField(DataPathLink link, std::array<T, N> def, detail::Key key, const char* label, const char* (&sublabels)[N], bool inline_checkboxes)
			: Super(std::move(link), def, key, label), sublabels(sublabels), inline_checkboxes(inline_checkboxes) {}

		void Draw()
		{
			const auto initial = this->value;
			DescIO::Draw(this->label, this->value.data(), this->def.data(), sublabels, N, inline_checkboxes);
			if (initial != this->value)
				PushFieldSetAction(this->link.ComputePath(), initial, this->value);
		}
	};

	template<size_t N>
	using BoolArrayField = ArrayField<bool, N>;

	template<typename T, size_t N>
	struct SessionArrayField : public PrimitiveField<std::array<T, N>>, public imtk::tick_processor
	{
		using Super = PrimitiveField<std::array<T, N>>;

		const char** sublabels = nullptr;
		std::array<EditSession<T>, N> edits;

		SessionArrayField(DataPathLink link, std::array<T, N> def, detail::Key key, const char* label)
			: Super(std::move(link), def, key, label), edits(_MakeEdits(this->value, std::make_index_sequence<N>{})) {}

		SessionArrayField(DataPathLink link, std::array<T, N> def, detail::Key key, const char* label, const char* (&sublabels)[N])
			: Super(std::move(link), def, key, label), edits(_MakeEdits(this->value, std::make_index_sequence<N>{})), sublabels(sublabels) {}

	private:
		template<size_t... Is>
		static std::array<EditSession<T>, N> _MakeEdits(std::array<T, N>& value, std::index_sequence<Is...>)
		{
			return { EditSession<T>{value[Is]}... };
		}

	public:
		SessionArrayField(const SessionArrayField& o)
			: Super(o), edits(_MakeEdits(this->value, std::make_index_sequence<N>{}))
		{
		}

		SessionArrayField(SessionArrayField&& o) noexcept
			: Super(std::move(o)), edits(_MakeEdits(this->value, std::make_index_sequence<N>{}))
		{
		}

		SessionArrayField& operator=(const SessionArrayField& o)
		{
			if (this != &o)
				Super::operator=(o);

			return *this;
		}

		SessionArrayField& operator=(SessionArrayField&& o) noexcept
		{
			if (this != &o)
				Super::operator=(std::move(o));

			return *this;
		}

		void Draw()
		{
			if (sublabels)
				DescIO::Draw(this->label, edits.data(), this->def.data(), sublabels, N);
			else
				DescIO::Draw(this->label, edits.data(), this->def.data(), N);

			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			for (size_t i = 0; i < N; ++i)
			{
				if (edits[i].ConsumeModified())
					PushFieldSetAction(this->link.ComputePath() / DataPathStep(i), std::move(edits[i].original), this->value[i]);
			}
		}

		void* PathGet(DataPath path, std::type_index type)
		{
			if (path.Empty())
				return typeid(decltype(this->value)) == type ? static_cast<void*>(&this->value) : nullptr;

			auto index = path.Step().v;
			if (index >= 0 && index < N)
			{
				path = path.Next();
				if (type == typeid(this->value[index]) && path.Empty())
					return static_cast<void*>(&this->value[index]);
				else
					return nullptr;
			}
			else
				return nullptr;
		}

		void on_last_process_frame() override
		{
			for (auto& edit : edits)
				edit.on_last_process_frame();
			CheckUndoAction();
		}
	};

	template<size_t N>
	using StringArrayField = SessionArrayField<std::string, N>;

	template<typename T>
	struct VectorField : public PrimitiveField<std::vector<T>>
	{
		gui::DynamicListState ui_state;

		using PrimitiveField<std::vector<T>>::PrimitiveField;
	};

	struct StringVectorField : public VectorField<std::string>
	{
		using Super = VectorField<std::string>;

		EditSession<std::vector<std::string>> edit;

		StringVectorField(DataPathLink link, std::vector<std::string> def, detail::Key key, const char* label) : Super(std::move(link), def, key, label), edit(value) {}

		StringVectorField(StringVectorField&& o) noexcept
			: Super(std::move(o)), edit(value)
		{
		}

		StringVectorField& operator=(StringVectorField&& o) noexcept
		{
			if (this != &o)
				Super::operator=(std::move(o));

			return *this;
		}

		void CheckUndoAction()
		{
			if (edit.ConsumeModified())
				PushFieldSetAction(this->link.ComputePath(), std::move(edit.original), this->value);
		}
	};

	template<typename E>
	struct DisjointEnumField
	{
		DataPathLink link;
		E def;
		int index;
		int def_index;
		detail::Key key;
		const char* label;
		const E* values;
		LabelSpanRegistry::Handle names;
		size_t count;

		template<size_t Count>
		DisjointEnumField(DataPathLink link, E def, detail::Key key, const char* label, const E (&values)[Count], const char* (&names)[Count])
			: link(std::move(link)), def(def), key(key), label(label), values(values), names(LabelSpanRegistry::Intern(std::span<const char*>(names, Count))), count(Count)
		{
			SetValue(def);
			def_index = Index(def);
		}

		void CopyData(const DisjointEnumField& o)
		{
			index = o.index;
		}

		void Draw()
		{
			const auto initial = index;
			DescIO::Draw(label, index, def_index, names);
			if (initial != index)
				PushFieldSetAction(link.ComputePath(), initial, index);
		}

		void Load(TOMLNode node)
		{
			index = Index(static_cast<E>(node[detail::encode_key(key)].value_or(def)));
		}
		
		void Dump(toml::table& table) const
		{
			table.insert_or_assign(detail::encode_key(key), Value());
		}

		E Value() const
		{
			return values[index];
		}
		
		void SetValue(const E val)
		{
			index = Index(val);
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

		void* PathGet(DataPath path, std::type_index type)
		{
			if (type == typeid(decltype(index)) && path.Empty())
				return static_cast<void*>(&index);
			else
				return nullptr;
		}

		bool QueryDirty(const DisjointEnumField<E>& disk) const
		{
			return index != disk.index;
		}
	};
	
	template<typename T, OptionalPrimitive<T> _Min, OptionalPrimitive<T> _Max>
	struct OptionalRangeField : public imtk::tick_processor
	{
		using Self = OptionalRangeField<T, _Min, _Max>;
		inline static const OptionalPrimitive<T> Min = _Min;
		inline static const OptionalPrimitive<T> Max = _Max;

		DataPathLink link;
		OptionalPrimitive<T> def;
		OptionalPrimitive<T> value;
		EditSession<OptionalPrimitive<T>> edit;
		detail::Key value_key;
		detail::Key enable_key;
		const char* label;

		OptionalRangeField(DataPathLink link, OptionalPrimitive<T> def, detail::Key value_key, detail::Key enable_key, const char* label)
			: link(std::move(link)), def(def), value(def), edit(value), value_key(value_key), enable_key(enable_key), label(label)
		{
		}

		OptionalRangeField(const OptionalRangeField& o)
			: link(o.link), def(o.def), value(o.value), edit(value), value_key(o.value_key), enable_key(o.enable_key), label(o.label)
		{
		}

		OptionalRangeField(OptionalRangeField&& o)
			: link(std::move(o.link)), def(std::move(o.def)), value(std::move(o.value)), edit(value), value_key(o.value_key), enable_key(o.enable_key), label(o.label)
		{
		}

		OptionalRangeField& operator=(const OptionalRangeField& o)
		{
			if (this != &o)
			{
				def = o.def;
				value = o.value;
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
				value = std::move(o.value);
				value_key = o.value_key;
				enable_key = o.enable_key;
				label = o.label;
			}

			return *this;
		}

		void CopyData(const OptionalRangeField& o)
		{
			value = o.value;
		}

		void Draw()
		{
			DescIO::Draw(label, edit, def, Min, Max);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.ConsumeModified())
				PushFieldSetAction(link.ComputePath(), std::move(edit.original), value);
		}

		void Load(TOMLNode node)
		{
			value = def;
			if (enable_key != NullKey() && value_key != NullKey())
			{
				Serializer<T>{}.Load(value.value, node[detail::encode_key(value_key)]);
				Serializer<bool>{}.Load(value.has_value, node[detail::encode_key(enable_key)]);
			}
		}

		void Dump(toml::table& table) const
		{
			if (enable_key != NullKey() && value_key != NullKey())
			{
				table.insert_or_assign(detail::encode_key(enable_key), Serializer<bool>{}.Dump(value.has_value));
				table.insert_or_assign(detail::encode_key(value_key), Serializer<T>{}.Dump(value.value));
			}
		}

		void* PathGet(DataPath path, std::type_index type)
		{
			if (type == typeid(decltype(value)) && path.Empty())
				return static_cast<void*>(&value);
			else
				return nullptr;
		}

		void on_last_process_frame() override
		{
			edit.on_last_process_frame();
			CheckUndoAction();
		}

		bool QueryDirty(const OptionalRangeField& disk) const
		{
			return value != disk.value;
		}
	};

	template<OptionalInt Min, OptionalInt Max>
	using OptionalIntField = OptionalRangeField<int, Min, Max>;

	template<OptionalFloat Min, OptionalFloat Max>
	using OptionalFloatField = OptionalRangeField<float, Min, Max>;

	template<OptionalDouble Min, OptionalDouble Max>
	using OptionalDoubleField = OptionalRangeField<double, Min, Max>;

	template<typename T, OptionalPrimitive<T> _Min, OptionalPrimitive<T> _Max>
	struct CompactOptionalRangeField : public imtk::tick_processor
	{
		inline static const OptionalPrimitive<T> Min = _Min;
		inline static const OptionalPrimitive<T> Max = _Max;

		DataPathLink link;
		OptionalPrimitive<T> def;
		OptionalPrimitive<T> value;
		EditSession<OptionalPrimitive<T>> edit;
		T nullopt;
		detail::Key key;
		const char* label;

		CompactOptionalRangeField(DataPathLink link, OptionalPrimitive<T> def, T nullopt, detail::Key key, const char* label)
			: link(std::move(link)), def(def), value(def), edit(value), nullopt(nullopt), key(key), label(label)
		{
		}

		CompactOptionalRangeField(const CompactOptionalRangeField& o)
			: link(o.link), def(o.def), value(o.value), edit(value), nullopt(o.nullopt), key(o.key), label(o.label)
		{
		}

		CompactOptionalRangeField(CompactOptionalRangeField&& o) noexcept
			: link(std::move(o.link)), def(std::move(o.def)), value(std::move(o.value)), edit(value), nullopt(o.nullopt), key(o.key), label(o.label)
		{
		}

		CompactOptionalRangeField& operator=(const CompactOptionalRangeField& o)
		{
			if (this != &o)
			{
				def = o.def;
				value = o.value;
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
				std::move(value) = std::move(o.value);
				nullopt = o.nullopt;
				key = o.key;
				label = o.label;
			}

			return *this;
		}

		void CopyData(const CompactOptionalRangeField& o)
		{
			value = o.value;
		}

		void Draw()
		{
			DescIO::Draw(label, edit, def, Min, Max);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.ConsumeModified())
				PushFieldSetAction(link.ComputePath(), std::move(edit.original), value);
		}

		void Load(TOMLNode node)
		{
			value = def;
			if (key != NullKey())
			{
				T temp = def.value;
				if (Serializer<T>{}.Load(temp, node[detail::encode_key(key)]))
				{
					value.has_value = temp != nullopt;
					if (value.has_value)
						value.value = temp;
				}
				else
					value.has_value = false;
			}
		}

		void Dump(toml::table& table) const
		{
			if (key != NullKey())
				table.insert_or_assign(detail::encode_key(key), Serializer<T>{}.Dump(value.has_value ? value.value : nullopt));
		}

		void* PathGet(DataPath path, std::type_index type)
		{
			if (type == typeid(decltype(value)) && path.Empty())
				return static_cast<void*>(&value);
			else
				return nullptr;
		}

		// TODO v9.3 remove DrawFinalize: have fields inherit from imtk::tick_processor, and pass DataPath to constructor of fields (store DataPathSource as member). Make sure it syncs for dynamic descriptors
		void on_last_process_frame() override
		{
			edit.on_last_process_frame();
			CheckUndoAction();
		}

		bool QueryDirty(const CompactOptionalRangeField& disk) const
		{
			return value != disk.value;
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
		DataPathLink link;
		bool def_flags[Count];
		bool value_flags[Count];
		E def;
		E value;
		detail::Key key;
		const char* label;
		const E* values;
		const char** names;
		bool inline_checkboxes;

		static const inline size_t Count = Count;

		BitsetField(DataPathLink link, E def, detail::Key key, const char* label, const E(&values)[Count], const char* (&names)[Count], bool inline_checkboxes)
			: link(std::move(link)), def(def), value(def), key(key), label(label), values(values), names(names), inline_checkboxes(inline_checkboxes)
		{
			SetFlags();
		}

		void CopyData(const BitsetField& o)
		{
			value = o.value;
		}

		void Draw(const bool (&disabled)[Count])
		{
			return Draw(static_cast<const bool*>(disabled));
		}

		void Draw()
		{
			return Draw(nullptr);
		}

	private:
		void Draw(const bool* disabled)
		{
			const auto initial = value;
			SetFlags();
			DescIO::Draw(label, value_flags, def_flags, names, disabled, Count, inline_checkboxes);
			SetEnum();
			if (initial != value)
				PushFieldSetAction(link.ComputePath(), initial, value);
		}

		void SetFlags()
		{
			for (size_t i = 0; i < Count; ++i)
			{
				value_flags[i] = static_cast<bool>(value & values[i]);
				def_flags[i] = static_cast<bool>(def & values[i]);
			}
		}

		void SetEnum()
		{
			for (size_t i = 0; i < Count; ++i)
			{
				if (value_flags[i])
					value |= values[i];
				else
					value &= ~values[i];
			}
		}

	public:
		void Load(TOMLNode node)
		{
			value = def;
			Serializer<E>{}.Load(value, node[detail::encode_key(key)]);
		}

		void Dump(toml::table& table) const
		{
			table.insert_or_assign(detail::encode_key(key), Serializer<E>{}.Dump(value));
		}

		void* PathGet(DataPath path, std::type_index type)
		{
			if (type == typeid(decltype(value)) && path.Empty())
				return static_cast<void*>(&value);
			else
				return nullptr;
		}

		bool QueryDirty(const BitsetField<E, Count>& disk) const
		{
			return value != disk.value;
		}
	};
}
