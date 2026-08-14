#pragma once

#include "desc/DescIO.h"
#include "desc/Serializer.h"
#include "desc/FieldSetAction.h"

#include "assets/TranslateKey.h"

#include "gui/DynamicList.h"

#include <array>

namespace oly::editor
{
	extern detail::Key NullKey();

	template<typename T>
	struct PrimitiveField
	{
		imtk::datapath_link link;
		T def;
		T value;
		detail::Key key;
		const char* label;

		PrimitiveField(imtk::datapath_link link, T def, detail::Key key, const char* label) : link(std::move(link)), def(def), value(def), key(key), label(label) {}

		void copy_data(const PrimitiveField& o)
		{
			value = o.value;
		}

		bool query_dirty(const PrimitiveField& disk) const
		{
			return value != disk.value;
		}

		void load(imtk::toml_node node)
		{
			value = def;
			if (key != NullKey())
				imtk::serializer<T>{}.load(value, node[detail::encode_key(key)]);
			else
				imtk::serializer<T>{}.load(value, node);
		}

		void dump(toml::table& table) const
		{
			if (key != NullKey())
				table.insert_or_assign(detail::encode_key(key), imtk::serializer<T>{}.dump(value));
		}

		void dump(toml::array& array) const
		{
			array.push_back(imtk::serializer<T>{}.dump(value));
		}

		void* resolve(imtk::datapath_view path, std::type_index type)
		{
			if (type == typeid(decltype(value)) && path.empty())
				return static_cast<void*>(&value);
			else
				return nullptr;
		}
	};

	struct BoolField : public PrimitiveField<bool>
	{
		using PrimitiveField<bool>::PrimitiveField;

		void draw()
		{
			const auto initial = value;
			DescIO::Draw(label, value, def);
			if (initial != value)
				PushFieldSetAction(link.compute_path(), initial, value);
		}
	};

	template<typename T, typename U, OptionalPrimitive<U> _Min, OptionalPrimitive<U> _Max>
	struct RangeField : public PrimitiveField<T>, public imtk::tick_processor
	{
		using Super = PrimitiveField<T>;

		inline static const OptionalPrimitive<U> Min = _Min;
		inline static const OptionalPrimitive<U> Max = _Max;

		imtk::edit_session<T> edit;

		RangeField(imtk::datapath_link link, T def, detail::Key key, const char* label)
			: Super(std::move(link), def, key, label), imtk::tick_processor(imtk::tick_process_phase::check_undo), edit(this->value)
		{
		}

		RangeField(RangeField&& o) noexcept
			: Super(std::move(o)), imtk::tick_processor(std::move(o)), edit(this->value)
		{
		}

		RangeField& operator=(RangeField&& o) noexcept
		{
			if (this != &o)
			{
				Super::operator=(std::move(o));
				imtk::tick_processor::operator=(std::move(o));
			}

			return *this;
		}

		// TODO v9.3 copy_data() for fields that have edit sessions that directly copies edit?

		void draw()
		{
			DescIO::Draw(this->label, this->edit, this->def, Min, Max);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.consume_modified())
				PushFieldSetAction(this->link.compute_path(), edit.original(), this->value);
		}

		void on_last_process_frame() override
		{
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

		void draw()
		{
			const auto initial = this->value;
			DescIO::Draw(this->label, this->value, this->def);
			if (initial != this->value)
				PushFieldSetAction(this->link.compute_path(), initial, this->value);
		}
	};

	struct StringField : public PrimitiveField<std::string>, public imtk::tick_processor
	{
		imtk::edit_session<std::string> edit;

		StringField(imtk::datapath_link link, std::string def, detail::Key key, const char* label)
			: PrimitiveField(std::move(link), std::move(def), key, label), imtk::tick_processor(imtk::tick_process_phase::check_undo), edit(value)
		{
		}

		StringField(StringField&& o) noexcept
			: PrimitiveField(std::move(o)), imtk::tick_processor(std::move(o)), edit(value)
		{
		}

		StringField& operator=(StringField&& o) noexcept
		{
			if (this != &o)
			{
				PrimitiveField::operator=(std::move(o));
				imtk::tick_processor::operator=(std::move(o));
			}

			return *this;
		}

		void draw()
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.consume_modified())
				PushFieldSetAction(link.compute_path(), edit.original(), value);
		}

		void on_last_process_frame() override
		{
			CheckUndoAction();
		}
	};

	struct Color4Field : public PrimitiveField<Color4>, public imtk::tick_processor
	{
		imtk::edit_session<Color4> edit;

		Color4Field(imtk::datapath_link link, Color4 def, detail::Key key, const char* label)
			: PrimitiveField(std::move(link), def, key, label), imtk::tick_processor(imtk::tick_process_phase::check_undo), edit(value)
		{
		}

		Color4Field(Color4Field&& o) noexcept
			: PrimitiveField(std::move(o)), imtk::tick_processor(std::move(o)), edit(value)
		{
		}

		Color4Field& operator=(Color4Field&& o) noexcept
		{
			if (this != &o)
			{
				PrimitiveField::operator=(std::move(o));
				imtk::tick_processor::operator=(std::move(o));
			}

			return *this;
		}

		void draw()
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.consume_modified())
				PushFieldSetAction(link.compute_path(), edit.original(), value);
		}

		void on_last_process_frame() override
		{
			CheckUndoAction();
		}
	};

	struct RectField : public PrimitiveField<Rect>, public imtk::tick_processor
	{
		imtk::edit_session<Rect> edit;

		RectField(imtk::datapath_link link, Rect def, detail::Key key, const char* label)
			: PrimitiveField(std::move(link), def, key, label), imtk::tick_processor(imtk::tick_process_phase::check_undo), edit(value)
		{
		}

		RectField(RectField&& o) noexcept
			: PrimitiveField(std::move(o)), imtk::tick_processor(std::move(o)), edit(value)
		{
		}

		RectField& operator=(RectField&& o) noexcept
		{
			if (this != &o)
			{
				PrimitiveField::operator=(std::move(o));
				imtk::tick_processor::operator=(std::move(o));
			}

			return *this;
		}

		void draw()
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.consume_modified())
				PushFieldSetAction(link.compute_path(), edit.original(), value);
		}

		void on_last_process_frame() override
		{
			CheckUndoAction();
		}
	};

	struct UVRectField : public PrimitiveField<UVRect>, public imtk::tick_processor
	{
		imtk::edit_session<UVRect> edit;

		UVRectField(imtk::datapath_link link, UVRect def, detail::Key key, const char* label)
			: PrimitiveField(std::move(link), def, key, label), imtk::tick_processor(imtk::tick_process_phase::check_undo), edit(value)
		{
		}

		UVRectField(UVRectField&& o) noexcept
			: PrimitiveField(std::move(o)), imtk::tick_processor(std::move(o)), edit(value)
		{
		}

		UVRectField& operator=(UVRectField&& o) noexcept
		{
			if (this != &o)
			{
				PrimitiveField::operator=(std::move(o));
				imtk::tick_processor::operator=(std::move(o));
			}

			return *this;
		}

		void draw()
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.consume_modified())
				PushFieldSetAction(link.compute_path(), edit.original(), value);
		}

		void on_last_process_frame() override
		{
			CheckUndoAction();
		}
	};

	struct TopSidePaddingField : public PrimitiveField<TopSidePadding>, public imtk::tick_processor
	{
		imtk::edit_session<TopSidePadding> edit;

		TopSidePaddingField(imtk::datapath_link link, TopSidePadding def, detail::Key key, const char* label)
			: PrimitiveField(std::move(link), def, key, label), imtk::tick_processor(imtk::tick_process_phase::check_undo), edit(value)
		{
		}

		TopSidePaddingField(TopSidePaddingField&& o) noexcept
			: PrimitiveField(std::move(o)), imtk::tick_processor(std::move(o)), edit(value)
		{
		}

		TopSidePaddingField& operator=(TopSidePaddingField&& o) noexcept
		{
			if (this != &o)
			{
				PrimitiveField::operator=(std::move(o));
				imtk::tick_processor::operator=(std::move(o));
			}

			return *this;
		}

		void draw()
		{
			DescIO::Draw(label, edit, def);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.consume_modified())
				PushFieldSetAction(link.compute_path(), edit.original(), value);
		}

		void on_last_process_frame() override
		{
			CheckUndoAction();
		}
	};

	template<typename T, size_t N>
	struct ArrayField : public PrimitiveField<std::array<T, N>>
	{
		using Super = PrimitiveField<std::array<T, N>>;

		const char** sublabels;
		bool inline_checkboxes;

		ArrayField(imtk::datapath_link link, std::array<T, N> def, detail::Key key, const char* label, const char* (&sublabels)[N], bool inline_checkboxes)
			: Super(std::move(link), def, key, label), sublabels(sublabels), inline_checkboxes(inline_checkboxes) {}

		void draw()
		{
			const auto initial = this->value;
			DescIO::Draw(this->label, this->value.data(), this->def.data(), sublabels, N, inline_checkboxes);
			if (initial != this->value)
				PushFieldSetAction(this->link.compute_path(), initial, this->value);
		}
	};

	template<size_t N>
	using BoolArrayField = ArrayField<bool, N>;

	template<typename T, size_t N>
	struct SessionArrayField : public PrimitiveField<std::array<T, N>>, public imtk::tick_processor
	{
		using Super = PrimitiveField<std::array<T, N>>;

		const char** sublabels = nullptr;
		std::array<imtk::edit_session<T>, N> edits;

		SessionArrayField(imtk::datapath_link link, std::array<T, N> def, detail::Key key, const char* label)
			: Super(std::move(link), def, key, label), imtk::tick_processor(imtk::tick_process_phase::check_undo),
			edits(_MakeEdits(this->value, std::make_index_sequence<N>{}))
		{
		}

		SessionArrayField(imtk::datapath_link link, std::array<T, N> def, detail::Key key, const char* label, const char* (&sublabels)[N])
			: Super(std::move(link), def, key, label), imtk::tick_processor(imtk::tick_process_phase::check_undo),
			edits(_MakeEdits(this->value, std::make_index_sequence<N>{})), sublabels(sublabels)
		{
		}

	private:
		template<size_t... Is>
		static std::array<imtk::edit_session<T>, N> _MakeEdits(std::array<T, N>& value, std::index_sequence<Is...>)
		{
			return { imtk::edit_session<T>{value[Is]}... };
		}

	public:
		SessionArrayField(SessionArrayField&& o) noexcept
			: Super(std::move(o)), imtk::tick_processor(std::move(o)), edits(_MakeEdits(this->value, std::make_index_sequence<N>{}))
		{
		}

		SessionArrayField& operator=(SessionArrayField&& o) noexcept
		{
			if (this != &o)
			{
				Super::operator=(std::move(o));
				imtk::tick_processor::operator=(std::move(o));
			}

			return *this;
		}

		void draw()
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
				if (edits[i].consume_modified())
					PushFieldSetAction(this->link.compute_path() / imtk::datapath::step(i), std::move(edits[i].original()), this->value[i]);
			}
		}

		void* resolve(imtk::datapath_view path, std::type_index type)
		{
			if (path.empty())
				return typeid(decltype(this->value)) == type ? static_cast<void*>(&this->value) : nullptr;

			int index = path.step();
			if (index >= 0 && index < N)
			{
				path = path.next();
				if (type == typeid(this->value[index]) && path.empty())
					return static_cast<void*>(&this->value[index]);
				else
					return nullptr;
			}
			else
				return nullptr;
		}

		void on_last_process_frame() override
		{
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

		imtk::edit_session<std::vector<std::string>> edit;

		StringVectorField(imtk::datapath_link link, std::vector<std::string> def, detail::Key key, const char* label) : Super(std::move(link), def, key, label), edit(value) {}

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
			if (edit.consume_modified())
				PushFieldSetAction(this->link.compute_path(), edit.original(), this->value);
		}
	};

	template<typename E>
	struct DisjointEnumField
	{
		imtk::datapath_link link;
		E def;
		int index;
		int def_index;
		detail::Key key;
		const char* label;
		const E* values;
		imtk::label_span_registry::handle names;
		size_t count;

		template<size_t Count>
		DisjointEnumField(imtk::datapath_link link, E def, detail::Key key, const char* label, const E (&values)[Count], const char* (&names)[Count])
			: link(std::move(link)), def(def), key(key), label(label), values(values), names(imtk::label_span_registry::intern(std::span<const char*>(names, Count))), count(Count)
		{
			SetValue(def);
			def_index = Index(def);
		}

		void copy_data(const DisjointEnumField& o)
		{
			index = o.index;
		}

		void draw()
		{
			const auto initial = index;
			DescIO::Draw(label, index, def_index, names);
			if (initial != index)
				PushFieldSetAction(link.compute_path(), initial, index);
		}

		void load(imtk::toml_node node)
		{
			index = Index(static_cast<E>(node[detail::encode_key(key)].value_or(def)));
		}
		
		void dump(toml::table& table) const
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

		void* resolve(imtk::datapath_view path, std::type_index type)
		{
			if (type == typeid(decltype(index)) && path.empty())
				return static_cast<void*>(&index);
			else
				return nullptr;
		}

		bool query_dirty(const DisjointEnumField<E>& disk) const
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

		imtk::datapath_link link;
		OptionalPrimitive<T> def;
		OptionalPrimitive<T> value;
		imtk::edit_session<OptionalPrimitive<T>> edit;
		detail::Key value_key;
		detail::Key enable_key;
		const char* label;

		OptionalRangeField(imtk::datapath_link link, OptionalPrimitive<T> def, detail::Key value_key, detail::Key enable_key, const char* label)
			: imtk::tick_processor(imtk::tick_process_phase::check_undo), link(std::move(link)), def(def), value(def), edit(value), value_key(value_key), enable_key(enable_key), label(label)
		{
		}

		OptionalRangeField(OptionalRangeField&& o)
			: imtk::tick_processor(std::move(o)), link(std::move(o.link)), def(std::move(o.def)), value(std::move(o.value)), edit(value), value_key(o.value_key), enable_key(o.enable_key), label(o.label)
		{
		}

		OptionalRangeField& operator=(OptionalRangeField&& o)
		{
			if (this != &o)
			{
				imtk::tick_processor::operator=(std::move(o));
				def = std::move(o.def);
				value = std::move(o.value);
				value_key = o.value_key;
				enable_key = o.enable_key;
				label = o.label;
			}

			return *this;
		}

		void copy_data(const OptionalRangeField& o)
		{
			value = o.value;
		}

		void draw()
		{
			DescIO::Draw(label, edit, def, Min, Max);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.consume_modified())
				PushFieldSetAction(link.compute_path(), edit.original(), value);
		}

		void load(imtk::toml_node node)
		{
			value = def;
			if (enable_key != NullKey() && value_key != NullKey())
			{
				imtk::serializer<T>{}.load(value.value, node[detail::encode_key(value_key)]);
				imtk::serializer<bool>{}.load(value.has_value, node[detail::encode_key(enable_key)]);
			}
		}

		void dump(toml::table& table) const
		{
			if (enable_key != NullKey() && value_key != NullKey())
			{
				table.insert_or_assign(detail::encode_key(enable_key), imtk::serializer<bool>{}.dump(value.has_value));
				table.insert_or_assign(detail::encode_key(value_key), imtk::serializer<T>{}.dump(value.value));
			}
		}

		void* resolve(imtk::datapath_view path, std::type_index type)
		{
			if (type == typeid(decltype(value)) && path.empty())
				return static_cast<void*>(&value);
			else
				return nullptr;
		}

		void on_last_process_frame() override
		{
			CheckUndoAction();
		}

		bool query_dirty(const OptionalRangeField& disk) const
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

		imtk::datapath_link link;
		OptionalPrimitive<T> def;
		OptionalPrimitive<T> value;
		imtk::edit_session<OptionalPrimitive<T>> edit;
		T nullopt;
		detail::Key key;
		const char* label;

		CompactOptionalRangeField(imtk::datapath_link link, OptionalPrimitive<T> def, T nullopt, detail::Key key, const char* label)
			: imtk::tick_processor(imtk::tick_process_phase::check_undo), link(std::move(link)), def(def), value(def), edit(value), nullopt(nullopt), key(key), label(label)
		{
		}

		CompactOptionalRangeField(CompactOptionalRangeField&& o) noexcept
			: imtk::tick_processor(std::move(o)), link(std::move(o.link)), def(std::move(o.def)), value(std::move(o.value)), edit(value), nullopt(o.nullopt), key(o.key), label(o.label)
		{
		}

		CompactOptionalRangeField& operator=(CompactOptionalRangeField&& o) noexcept
		{
			if (this != &o)
			{
				imtk::tick_processor::operator=(std::move(o));
				std::move(def) = std::move(o.def);
				std::move(value) = std::move(o.value);
				nullopt = o.nullopt;
				key = o.key;
				label = o.label;
			}

			return *this;
		}

		void copy_data(const CompactOptionalRangeField& o)
		{
			value = o.value;
		}

		void draw()
		{
			DescIO::Draw(label, edit, def, Min, Max);
			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (edit.consume_modified())
				PushFieldSetAction(link.compute_path(), edit.original(), value);
		}

		void load(imtk::toml_node node)
		{
			value = def;
			if (key != NullKey())
			{
				T temp = def.value;
				if (imtk::serializer<T>{}.load(temp, node[detail::encode_key(key)]))
				{
					value.has_value = temp != nullopt;
					if (value.has_value)
						value.value = temp;
				}
				else
					value.has_value = false;
			}
		}

		void dump(toml::table& table) const
		{
			if (key != NullKey())
				table.insert_or_assign(detail::encode_key(key), imtk::serializer<T>{}.dump(value.has_value ? value.value : nullopt));
		}

		void* resolve(imtk::datapath_view path, std::type_index type)
		{
			if (type == typeid(decltype(value)) && path.empty())
				return static_cast<void*>(&value);
			else
				return nullptr;
		}

		void on_last_process_frame() override
		{
			CheckUndoAction();
		}

		bool query_dirty(const CompactOptionalRangeField& disk) const
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
		imtk::datapath_link link;
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

		BitsetField(imtk::datapath_link link, E def, detail::Key key, const char* label, const E(&values)[Count], const char* (&names)[Count], bool inline_checkboxes)
			: link(std::move(link)), def(def), value(def), key(key), label(label), values(values), names(names), inline_checkboxes(inline_checkboxes)
		{
			SetFlags();
		}

		void copy_data(const BitsetField& o)
		{
			value = o.value;
		}

		void draw(const bool (&disabled)[Count])
		{
			return draw(static_cast<const bool*>(disabled));
		}

		void draw()
		{
			return draw(nullptr);
		}

	private:
		void draw(const bool* disabled)
		{
			const auto initial = value;
			SetFlags();
			DescIO::Draw(label, value_flags, def_flags, names, disabled, Count, inline_checkboxes);
			SetEnum();
			if (initial != value)
				PushFieldSetAction(link.compute_path(), initial, value);
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
		void load(imtk::toml_node node)
		{
			value = def;
			imtk::serializer<E>{}.load(value, node[detail::encode_key(key)]);
		}

		void dump(toml::table& table) const
		{
			table.insert_or_assign(detail::encode_key(key), imtk::serializer<E>{}.dump(value));
		}

		void* resolve(imtk::datapath_view path, std::type_index type)
		{
			if (type == typeid(decltype(value)) && path.empty())
				return static_cast<void*>(&value);
			else
				return nullptr;
		}

		bool query_dirty(const BitsetField<E, Count>& disk) const
		{
			return value != disk.value;
		}
	};
}
