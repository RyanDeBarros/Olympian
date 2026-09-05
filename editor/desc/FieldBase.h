#pragma once

#include "desc/DescIO.h"

#include "gui/DynamicList.h"

#include <imp/group.hpp>

namespace oly::editor
{
	// TODO v9.3 separate data structure from draw behaviour?

	template<typename T>
	struct PrimitiveField : public imtk::tick_processor
	{
		imtk::datapath_link link;
		T def;
		T value;
		imtk::edit_session<T> edit;
		imtk::key key;
		const char* label;

		PrimitiveField(imtk::datapath_link link, T def, imtk::key key, const char* label)
			: imtk::tick_processor(imtk::tick_process_phase::check_undo), link(std::move(link)), def(def), value(def), edit(value), key(key), label(label)
		{
		}

		PrimitiveField(PrimitiveField&& o) noexcept
			: imtk::tick_processor(std::move(o)), link(std::move(o.link)), def(std::move(o.def)), value(std::move(o.value)), edit(value), key(o.key), label(o.label)
		{
		}

		PrimitiveField& operator=(const PrimitiveField&) = delete;
		PrimitiveField& operator=(PrimitiveField&&) noexcept = default;

		void copy_data(const PrimitiveField& o)
		{
			edit.publish_reset(o.value);
		}

		bool query_dirty(const PrimitiveField& disk) const
		{
			return value != disk.value;
		}

		void load(imtk::toml_node node)
		{
			T val = def;
			if (key != imtk::key::null())
				imtk::serializer<T>{}.load(val, node[imtk::encode_key(key)]);
			else
				imtk::serializer<T>{}.load(val, node);
			edit.publish_reset(std::move(val));
		}

		void dump(toml::table& table) const
		{
			if (key != imtk::key::null())
				table.insert_or_assign(imtk::encode_key(key), imtk::serializer<T>{}.dump(edit.truth()));
		}

		void* resolve(imtk::datapath_view path, imp::type_erasure type)
		{
			return path.empty() ? imp::matches_type(type, &value) : nullptr;
		}

	protected:
		void on_last_process_frame() override
		{
			CheckUndoAction();
		}

	public:
		void CheckUndoAction()
		{
			if (auto original = edit.consume_published_from())
				imtk::field::push_set_action(link.compute_path(), std::move(*original), edit.truth());
		}

		void draw()
		{
			if (auto row = imtk::prop::make_row_scope(label, edit, def))
				imtk::prop::value::add_component(std::make_unique<imtk::w::bound_widget<imtk::edit_session<T>>>(edit));

			CheckUndoAction();
		}
	};

	struct BoolField
	{
		imtk::datapath_link link;
		bool def;
		bool value;
		imtk::key key;
		const char* label;

		BoolField(imtk::datapath_link link, bool def, imtk::key key, const char* label)
			: link(std::move(link)), def(def), value(def), key(key), label(label)
		{
		}

		void copy_data(const BoolField& o)
		{
			value = o.value;
		}

		bool query_dirty(const BoolField& disk) const
		{
			return value != disk.value;
		}

		void load(imtk::toml_node node)
		{
			value = def;
			if (key != imtk::key::null())
				imtk::serializer<bool>{}.load(value, node[imtk::encode_key(key)]);
			else
				imtk::serializer<bool>{}.load(value, node);
		}

		void dump(toml::table& table) const
		{
			if (key != imtk::key::null())
				table.insert_or_assign(imtk::encode_key(key), imtk::serializer<bool>{}.dump(value));
		}

		void* resolve(imtk::datapath_view path, imp::type_erasure type)
		{
			return path.empty() ? imp::matches_type(type, &value) : nullptr;
		}

		void draw()
		{
			const bool og = value;

			if (auto row = imtk::prop::make_row_scope(label, value, def))
				imtk::prop::value::add_component(std::make_unique<imtk::w::bound_widget<bool>>(value));

			if (og != value)
				imtk::field::push_set_action(link.compute_path(), og, value);
		}
	};

	template<typename T, typename U, imp::potential<U> _Min, imp::potential<U> _Max>
	struct RangeField : public PrimitiveField<T>
	{
		using Super = PrimitiveField<T>;

		inline static const imp::potential<U> Min = _Min;
		inline static const imp::potential<U> Max = _Max;

		using Super::Super;

		void draw()
		{
			if (auto row = imtk::prop::make_row_scope(this->label, this->edit, this->def))
			{
				auto widget = std::make_unique<imtk::w::bound_widget<imtk::edit_session<T>>>(this->edit);
				widget->subwidget.config.min = Min;
				widget->subwidget.config.max = Max;
				imtk::prop::value::add_component(std::move(widget));
			}

			this->CheckUndoAction();
		}
	};

	template<imp::potential<int> Min, imp::potential<int> Max>
	using IntField = RangeField<int, int, Min, Max>;

	template<imp::potential<float> Min, imp::potential<float> Max>
	using FloatField = RangeField<float, float, Min, Max>;

	template<imp::potential<double> Min, imp::potential<double> Max>
	using DoubleField = RangeField<double, double, Min, Max>;

	template<typename E> requires (std::is_enum_v<E>)
		struct EnumField
	{
		imtk::datapath_link link;
		E def;
		E value;
		imtk::key key;
		const char* label;

		EnumField(imtk::datapath_link link, E def, imtk::key key, const char* label)
			: link(std::move(link)), def(def), value(def), key(key), label(label)
		{
		}

		void copy_data(const EnumField& o)
		{
			value = o.value;
		}

		bool query_dirty(const EnumField& disk) const
		{
			return value != disk.value;
		}

		void load(imtk::toml_node node)
		{
			value = def;
			if (key != imtk::key::null())
				imtk::serializer<E>{}.load(value, node[imtk::encode_key(key)]);
			else
				imtk::serializer<E>{}.load(value, node);
		}

		void dump(toml::table& table) const
		{
			if (key != imtk::key::null())
				table.insert_or_assign(imtk::encode_key(key), imtk::serializer<E>{}.dump(value));
		}

		void* resolve(imtk::datapath_view path, imp::type_erasure type)
		{
			return path.empty() ? imp::matches_type(type, &value) : nullptr;
		}

		void draw()
		{
			const E og = value;
			int int_value = static_cast<int>(value);
			const int int_default = static_cast<int>(def);

			if (auto row = imtk::prop::make_row_scope(label, int_value, int_default))
				imtk::prop::value::add_component(std::make_unique<imtk::w::combo_widget>(int_value, ComboNames()));

			value = static_cast<E>(int_value);
			if (og != value)
				imtk::field::push_set_action(link.compute_path(), og, value);
		}

		static imtk::label_span_registry::handle ComboNames();
	};

	using StringField = PrimitiveField<std::string>;
	using Color4Field = PrimitiveField<imtk::color4>;

	template<typename T, size_t N>
	struct ArrayField : public PrimitiveField<std::array<T, N>>
	{
		using Super = PrimitiveField<std::array<T, N>>;

		imtk::label_span_registry::handle sublabels = {};

		ArrayField(imtk::datapath_link link, std::array<T, N> def, imtk::key key, const char* label)
			: Super(std::move(link), def, key, label)
		{
		}

		ArrayField(imtk::datapath_link link, std::array<T, N> def, imtk::key key, const char* label, const char* (&sublabels)[N])
			: Super(std::move(link), def, key, label), sublabels(imtk::label_span_registry::intern(std::span<const char* const>(sublabels, N)))
		{
		}

		void* resolve(imtk::datapath_view path, imp::type_erasure type)
		{
			if (path.empty())
				return imp::matches_type(type, &this->value);

			int index = path.step();
			if (index >= 0 && index < N)
			{
				path = path.next();
				return path.empty() ? imp::matches_type(type, &this->value[index]) : nullptr;
			}
			else
				return nullptr;
		}

		void draw()
		{
			DescIO::Draw(this->label, this->edit, this->def, sublabels);
			this->CheckUndoAction();
		}
	};

	template<size_t N>
	struct BoolArrayField
	{
		imtk::datapath_link link;
		std::array<bool, N> def;
		std::array<bool, N> value;
		imtk::key key;
		const char* label;
		imtk::label_span_registry::handle sublabels = {};
		bool inline_checkboxes;

		BoolArrayField(imtk::datapath_link link, std::array<bool, N> def, imtk::key key, const char* label, const char* (&sublabels)[N], bool inline_checkboxes)
			: link(std::move(link)), def(def), value(def), key(key), label(label),
			sublabels(imtk::label_span_registry::intern(std::span<const char* const>(sublabels, N))), inline_checkboxes(inline_checkboxes)
		{
		}

		void copy_data(const BoolArrayField& o)
		{
			value = o.value;
		}

		bool query_dirty(const BoolArrayField& disk) const
		{
			return value != disk.value;
		}

		void load(imtk::toml_node node)
		{
			if (key != imtk::key::null())
				imtk::serializer<std::array<bool, N>>{}.load(value, node[imtk::encode_key(key)]);
			else
				imtk::serializer<std::array<bool, N>>{}.load(value, node);
		}

		void dump(toml::table& table) const
		{
			if (key != imtk::key::null())
				table.insert_or_assign(imtk::encode_key(key), imtk::serializer<std::array<bool, N>>{}.dump(value));
		}

		void* resolve(imtk::datapath_view path, imp::type_erasure type)
		{
			if (path.empty())
				return imp::matches_type(type, &this->value);

			int index = path.step();
			if (index >= 0 && index < N)
			{
				path = path.next();
				return path.empty() ? imp::matches_type(type, &this->value[index]) : nullptr;
			}
			else
				return nullptr;
		}

		void draw()
		{
			const std::array<bool, N> og = value;

			imp::group<bool> data_group(value);
			imp::group<const bool> def_group(def);

			if (auto row = imtk::prop::make_row_scope(label, data_group, def_group))
			{
				std::vector<std::unique_ptr<imtk::w::widget>> widgets;

				for (size_t i = 0; i < N; ++i)
					widgets.push_back(imtk::w::unique_bound_widget(value[i], { .label = imtk::label_span_registry::string(sublabels, i) }));

				if (inline_checkboxes)
					imtk::prop::value::add_component(std::make_unique<imtk::w::widget_row>(std::move(widgets)));
				else
					imtk::prop::value::add_component(std::make_unique<imtk::w::sequence>(std::move(widgets)));
			}

			if (og != value)
				imtk::field::push_set_action(link.compute_path(), og, value);
		}
	};

	template<size_t N>
	using StringArrayField = ArrayField<std::string, N>;

	template<typename T>
	struct VectorField : public PrimitiveField<std::vector<T>>
	{
		gui::DynamicListState ui_state;

		using PrimitiveField<std::vector<T>>::PrimitiveField;
	};

	using StringVectorField = VectorField<std::string>;

	template<typename E>
	struct DisjointEnumField
	{
		imtk::datapath_link link;
		E def;
		int index;
		int def_index;
		imtk::key key;
		const char* label;
		const E* values;
		imtk::label_span_registry::handle names = {};
		size_t count;

		template<size_t Count>
		DisjointEnumField(imtk::datapath_link link, E def, imtk::key key, const char* label, const E(&values)[Count], const char* (&names)[Count])
			: link(std::move(link)), def(def), key(key), label(label), values(values), names(imtk::label_span_registry::intern(std::span<const char*>(names, Count))), count(Count)
		{
			SetValue(def);
			def_index = Index(def);
		}

		void copy_data(const DisjointEnumField& o)
		{
			index = o.index;
		}

		bool query_dirty(const DisjointEnumField<E>& disk) const
		{
			return index != disk.index;
		}

		void draw()
		{
			const auto initial = index;

			if (auto row = imtk::prop::make_row_scope(label, index, def_index))
				imtk::prop::value::add_component(std::make_unique<imtk::w::combo_widget>(index, names));

			if (initial != index)
				imtk::field::push_set_action(link.compute_path(), initial, index);
		}

		void load(imtk::toml_node node)
		{
			index = Index(static_cast<E>(node[imtk::encode_key(key)].value_or(def)));
		}

		void dump(toml::table& table) const
		{
			table.insert_or_assign(imtk::encode_key(key), Value());
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

		void* resolve(imtk::datapath_view path, imp::type_erasure type)
		{
			return path.empty() ? imp::matches_type(type, &index) : nullptr;
		}
	};

	template<typename T, imp::potential<T> _Min, imp::potential<T> _Max>
	struct OptionalRangeField : public imtk::tick_processor
	{
		using Self = OptionalRangeField<T, _Min, _Max>;
		inline static const imp::potential<T> Min = _Min;
		inline static const imp::potential<T> Max = _Max;

		imtk::datapath_link link;
		imp::potential<T> def;
		imp::potential<T> value;
		imtk::edit_session<imp::potential<T>> edit;
		imtk::key value_key;
		imtk::key enable_key;
		const char* label;

		OptionalRangeField(imtk::datapath_link link, imp::potential<T> def, imtk::key value_key, imtk::key enable_key, const char* label)
			: imtk::tick_processor(imtk::tick_process_phase::check_undo), link(std::move(link)), def(def), value(def), edit(value), value_key(value_key), enable_key(enable_key), label(label)
		{
		}

		OptionalRangeField(OptionalRangeField&& o) noexcept
			: imtk::tick_processor(std::move(o)), link(std::move(o.link)), def(std::move(o.def)), value(std::move(o.value)), edit(value), value_key(o.value_key), enable_key(o.enable_key), label(o.label)
		{
		}

		OptionalRangeField& operator=(const OptionalRangeField&) = delete;
		OptionalRangeField& operator=(OptionalRangeField&&) noexcept = default;

		void copy_data(const OptionalRangeField& o)
		{
			edit.publish_reset(o.value);
		}

		bool query_dirty(const OptionalRangeField& disk) const
		{
			return value != disk.value;
		}

		void load(imtk::toml_node node)
		{
			imp::potential<T> val = def;

			if (enable_key != imtk::key::null() && value_key != imtk::key::null())
			{
				imtk::serializer<T>{}.load(val.value, node[imtk::encode_key(value_key)]);
				imtk::serializer<bool>{}.load(val.has_value, node[imtk::encode_key(enable_key)]);
			}

			edit.publish_reset(std::move(val));
		}

		void dump(toml::table& table) const
		{
			if (enable_key != imtk::key::null() && value_key != imtk::key::null())
			{
				table.insert_or_assign(imtk::encode_key(enable_key), imtk::serializer<bool>{}.dump(edit.truth().has_value));
				table.insert_or_assign(imtk::encode_key(value_key), imtk::serializer<T>{}.dump(edit.truth().value));
			}
		}

		void draw()
		{
			if (auto row = imtk::prop::make_row_scope(label, edit, def))
			{
				auto widget = std::make_unique<imtk::w::bound_widget<imtk::edit_session<imp::potential<T>>>>(edit);
				widget->subwidget.value.config.min = Min;
				widget->subwidget.value.config.max = Max;
				imtk::prop::value::add_component(std::move(widget));
			}

			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (auto original = edit.consume_published_from())
				imtk::field::push_set_action(link.compute_path(), std::move(*original), edit.truth());
		}

		void* resolve(imtk::datapath_view path, imp::type_erasure type)
		{
			return path.empty() ? imp::matches_type(type, &value) : nullptr;
		}

	protected:
		void on_last_process_frame() override
		{
			CheckUndoAction();
		}
	};

	template<imp::potential<int> Min, imp::potential<int> Max>
	using OptionalIntField = OptionalRangeField<int, Min, Max>;

	template<imp::potential<float> Min, imp::potential<float> Max>
	using OptionalFloatField = OptionalRangeField<float, Min, Max>;

	template<imp::potential<double> Min, imp::potential<double> Max>
	using OptionalDoubleField = OptionalRangeField<double, Min, Max>;

	template<typename T, imp::potential<T> _Min, imp::potential<T> _Max>
	struct CompactOptionalRangeField : public imtk::tick_processor
	{
		inline static const imp::potential<T> Min = _Min;
		inline static const imp::potential<T> Max = _Max;

		imtk::datapath_link link;
		imp::potential<T> def;
		imp::potential<T> value;
		imtk::edit_session<imp::potential<T>> edit;
		T nullopt;
		imtk::key key;
		const char* label;

		CompactOptionalRangeField(imtk::datapath_link link, imp::potential<T> def, T nullopt, imtk::key key, const char* label)
			: imtk::tick_processor(imtk::tick_process_phase::check_undo), link(std::move(link)), def(def), value(def), edit(value), nullopt(nullopt), key(key), label(label)
		{
		}

		CompactOptionalRangeField(CompactOptionalRangeField&& o) noexcept
			: imtk::tick_processor(std::move(o)), link(std::move(o.link)), def(std::move(o.def)), value(std::move(o.value)), edit(value), nullopt(o.nullopt), key(o.key), label(o.label)
		{
		}

		CompactOptionalRangeField& operator=(const CompactOptionalRangeField&) = delete;
		CompactOptionalRangeField& operator=(CompactOptionalRangeField&&) noexcept = default;

		void copy_data(const CompactOptionalRangeField& o)
		{
			edit.publish_reset(o.value);
		}

		bool query_dirty(const CompactOptionalRangeField& disk) const
		{
			return value != disk.value;
		}

		void draw()
		{
			if (auto row = imtk::prop::make_row_scope(label, edit, def))
			{
				auto widget = std::make_unique<imtk::w::bound_widget<imtk::edit_session<imp::potential<T>>>>(edit);
				widget->subwidget.value.config.min = Min;
				widget->subwidget.value.config.max = Max;
				imtk::prop::value::add_component(std::move(widget));
			}

			CheckUndoAction();
		}

		void CheckUndoAction()
		{
			if (auto original = edit.consume_published_from())
				imtk::field::push_set_action(link.compute_path(), std::move(*original), edit.truth());
		}

		void load(imtk::toml_node node)
		{
			imp::potential<T> val = def;

			if (key != imtk::key::null())
			{
				T temp = def.value;
				if (imtk::serializer<T>{}.load(temp, node[imtk::encode_key(key)]))
				{
					val.has_value = temp != nullopt;
					if (val.has_value)
						val.value = temp;
				}
				else
					val.has_value = false;
			}

			edit.publish_reset(std::move(val));
		}

		void dump(toml::table& table) const
		{
			if (key != imtk::key::null())
				table.insert_or_assign(imtk::encode_key(key), imtk::serializer<T>{}.dump(edit.truth().has_value ? edit.truth().value : nullopt));
		}

		void* resolve(imtk::datapath_view path, imp::type_erasure type)
		{
			return path.empty() ? imp::matches_type(type, &value) : nullptr;
		}

		void on_last_process_frame() override
		{
			CheckUndoAction();
		}
	};

	template<imp::potential<int> Min, imp::potential<int> Max>
	using CompactOptionalIntField = CompactOptionalRangeField<int, Min, Max>;

	template<imp::potential<float> Min, imp::potential<float> Max>
	using CompactOptionalFloatField = CompactOptionalRangeField<float, Min, Max>;

	template<imp::potential<float> Min, imp::potential<float> Max, glm::length_t L>
	using VecField = RangeField<glm::vec<L, float>, float, Min, Max>;

	template<imp::potential<float> Min, imp::potential<float> Max>
	using Vec2Field = VecField<Min, Max, 2>;

	template<imp::potential<float> Min, imp::potential<float> Max>
	using Vec3Field = VecField<Min, Max, 3>;

	template<imp::potential<float> Min, imp::potential<float> Max>
	using Vec4Field = VecField<Min, Max, 4>;

	template<typename E, size_t Count>
	struct BitsetField
	{
		imtk::datapath_link link;
		bool def_flags[Count];
		bool value_flags[Count];
		E def;
		E value;
		imtk::key key;
		const char* label;
		const E* values;
		imtk::label_span_registry::handle names = {};
		bool inline_checkboxes;

		static const inline size_t Count = Count;

		BitsetField(imtk::datapath_link link, E def, imtk::key key, const char* label, const E(&values)[Count], const char* (&names)[Count], bool inline_checkboxes)
			: link(std::move(link)), def(def), value(def), key(key), label(label), values(values),
			names(imtk::label_span_registry::intern(std::span<const char* const>(names, Count))), inline_checkboxes(inline_checkboxes)
		{
			SetFlags();
		}

		void copy_data(const BitsetField& o)
		{
			value = o.value;
		}

		void draw(const bool(&disabled)[Count])
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

			imp::group<bool> data_group(value_flags, Count);
			imp::group<const bool> def_group(def_flags, Count);

			if (auto row = imtk::prop::make_row_scope(label, data_group, def_group))
			{
				std::vector<std::unique_ptr<imtk::w::widget>> widgets;

				for (size_t i = 0; i < Count; ++i)
				{
					widgets.push_back(std::make_unique<imtk::w::disabler>(
						imtk::w::unique_bound_widget(value_flags[i], { .label = imtk::label_span_registry::string(names, i) }),
						disabled && disabled[i]
					));
				}

				if (inline_checkboxes)
					imtk::prop::value::add_component(std::make_unique<imtk::w::widget_row>(std::move(widgets)));
				else
					imtk::prop::value::add_component(std::make_unique<imtk::w::sequence>(std::move(widgets)));
			}

			SetEnum();
			if (initial != value)
				imtk::field::push_set_action(link.compute_path(), initial, value);
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
			imtk::serializer<E>{}.load(value, node[imtk::encode_key(key)]);
		}

		void dump(toml::table& table) const
		{
			table.insert_or_assign(imtk::encode_key(key), imtk::serializer<E>{}.dump(value));
		}

		void* resolve(imtk::datapath_view path, imp::type_erasure type)
		{
			return path.empty() ? imp::matches_type(type, &value) : nullptr;
		}

		bool query_dirty(const BitsetField<E, Count>& disk) const
		{
			return value != disk.value;
		}
	};
}
