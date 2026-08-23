#include "SignalDocument.h"

#include "core/editor/Notifier.h"

#include "assets/TranslateKey.h"
#include "definitions/Keys.h"

#include "util/DynamicArray.h"

namespace oly::editor
{
	struct BriefDescPrinter
	{
		void operator()(std::ostream& os, const SignalDesc& desc) const
		{
			os << "SignalDesc[id=" << desc.id.value << ", binding=" << desc.binding.value << ", ...]";
		}

		void operator()(std::ostream& os, const RouteDesc& desc) const
		{
			os << "SignalDesc[id=" << desc.id.value << ", ...]";
		}
	};

	const char* SignalDocument::GetVersion()
	{
		return "1.0";
	}

	void SignalDocument::InitImpl()
	{
		if (!GetOlyPath().is_resource())
			Notifier::NotifyWarning("Asset is not located in resource folder");

		LoadAsset();
	}

	void SignalDocument::Draw()
	{
		auto pre_draw = PreDraw();

		_stop_listening = true;
		imtk::id_scope scope(this);

		// TODO v9.3 imtk::w::tab_bar widget with a map<std::string, imtk::w::tab_item> where the tab items have callbacks?
		if (auto _ = imtk::tab_bar(""))
		{
			if (auto _ = imtk::tab_item("Signals"))
				Draw(_desc.scratch.signals);

			if (auto _ = imtk::tab_item("Routes"))
				Draw(_desc.scratch.routes);
		}

		if (_stop_listening)
			_listen_mode = ListenMode::None;
	}

	void SignalDocument::LoadImpl()
	{
		if (_oly_path.is_file())
		{
			_meta = detail::MetaSplitter::decode_meta(_oly_path);

			toml::table table;
			std::string err = _oly_path.load_toml(table);
			if (err.empty())
				Load(imtk::toml_node(table), _desc.disk);
			else
				Notifier::NotifyError("cannot load signal - corrupted asset: " + _oly_path.string());

			MarkClean();
		}
		else
		{
			Load(imtk::toml_node(), _desc.disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = GetVersion();
			_meta.map[detail::Key::Meta_Import] = "0";
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Signal);

			MarkDirty();
		}

		_desc.LoadFromDisk();
		_signal_slots.Init(*gui::MakeVectorAdapter<BriefDescPrinter>(_desc.scratch.signals));
		_route_slots.Init(*gui::MakeVectorAdapter<BriefDescPrinter>(_desc.scratch.routes));
	}

	void SignalDocument::DumpImpl()
	{
		toml::table table;
		Dump(table, _desc.scratch);
		_oly_path.dump_toml(table, _meta);
		_desc.WriteToDisk();
		MarkClean();
	}

	void SignalDocument::ResetAssetImpl()
	{
		Load(imtk::toml_node(), _desc.scratch);
	}

	const IDoubleDescriptor& SignalDocument::GetDoubleDescriptor() const
	{
		return _desc;
	}

	IDoubleDescriptor& SignalDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	void SignalDocument::Draw(imtk::desc::vector<SignalDesc>& desc)
	{
		_signal_slots.Update(*gui::MakeVectorAdapter<BriefDescPrinter>(desc));

		if (auto scope = imtk::id_scope("##Signal"))
		{
			_signal_slots.DrawComboHeader({ .prompt = "Select signal", .create_tooltip = "New signal", .delete_tooltip = "Delete signal", .clear_tooltip = "Clear signals" },
				[&desc](size_t i) {
					if (i < desc.size())
					{
						std::string id = desc[i].id.value;
						if (!id.empty())
							return id;
					}
					return "<Signal #" + std::to_string(i) + ">";
				});
		}

		if (auto form = imtk::prop::form())
		{
			if (!desc.empty())
				Draw(desc[_signal_slots.active_index]);

			if (_signal_slots.ConsumeOps(*gui::MakeVectorAdapter<BriefDescPrinter>(desc)))
				MarkDirty();

			_signal_slots.active_index.consume_modified();
		}
	}

	void SignalDocument::Draw(imtk::desc::vector<RouteDesc>& desc)
	{
		_route_slots.Update(*gui::MakeVectorAdapter<BriefDescPrinter>(desc));

		if (auto scope = imtk::id_scope("##Route"))
		{
			_route_slots.DrawComboHeader({ .prompt = "Select route", .create_tooltip = "New route", .delete_tooltip = "Delete route", .clear_tooltip = "Clear routes" },
				[&desc](size_t i) {
					if (i < desc.size())
					{
						std::string id = desc[i].id.value;
						if (!id.empty())
							return id;
					}
					return "<Signal #" + std::to_string(i) + ">";
				});
		}

		if (auto form = imtk::prop::form())
		{
			if (!desc.empty())
				Draw(desc[_route_slots.active_index]);

			if (_route_slots.ConsumeOps(*gui::MakeVectorAdapter<BriefDescPrinter>(desc)))
				MarkDirty();

			_route_slots.active_index.consume_modified();
		}
	}

	Counter<std::string> SignalDocument::GetSignalIDCounter() const
	{
		Counter<std::string> id_counter;

		for (const auto& subdesc : _desc.scratch.signals)
			id_counter.increment(subdesc.id.value);

		return id_counter;
	}
	
	Counter<std::string> SignalDocument::GetRouteIDCounter() const
	{
		Counter<std::string> id_counter;

		for (const auto& subdesc : _desc.scratch.routes)
			id_counter.increment(subdesc.id.value);

		return id_counter;
	}

	Counter<std::string> SignalDocument::GetIDCounter() const
	{
		Counter<std::string> id_counter = GetSignalIDCounter();
		id_counter.accumulate(GetRouteIDCounter());
		return id_counter;
	}

	void SignalDocument::Draw(SignalDesc& desc)
	{
		imtk::outline dup_outline;
		
		desc.id.draw();
		if (GetIDCounter().count(desc.id.value) > 1)
		{
			if (imtk::prop::row::get_draw_result().state.hovered())
				ImGui::SetTooltip("Duplicate signal/route id");

			dup_outline.draw(imtk::col::error);
		}

		auto initial_binding = desc.binding.value;
		desc.binding.draw();

		switch (desc.binding.value)
		{
#define SWITCH_CASE(T) \
		case detail::SignalBindingType::T: \
		{ \
			if (!desc.variant.try_get<T##Desc>()) \
			{ \
				SignalDesc initial_desc = imtk::desc::clone_data(desc); \
				initial_desc.binding.value = initial_binding; \
				desc.variant.set<T##Desc>(); \
				PushDescriptorSetAction<SignalDesc, BriefDescPrinter>(desc.link.compute_path(), std::move(initial_desc), imtk::desc::clone_data(desc)); \
			} \
			break; \
		}

			BINDING_TYPE_GENERATOR(SWITCH_CASE)

#undef SWITCH_CASE
		}

		desc.variant.visit([this](auto& desc) { Draw(desc); });
	}
	
	void SignalDocument::Draw(RouteDesc& desc)
	{
		auto signal_id_counter = GetSignalIDCounter();
		auto id_counter = GetIDCounter();

		Counter<std::string> local_id_counter;
		local_id_counter.accumulate(desc.signals.value);

		imtk::outline dup_outline;

		desc.id.draw();
		if (id_counter.count(desc.id.value) > 1)
		{
			if (imtk::prop::row::get_draw_result().state.hovered())
				ImGui::SetTooltip("Duplicate signal/route id");

			dup_outline.draw(imtk::col::error);
		}

		desc.signals.edit.pre_edit();
		DescIO::DrawDynamicListRevertButtons(desc.signals.edit, desc.signals.def);

		DescIO::DrawDynamicList(desc.signals.link, desc.signals.label, desc.signals.edit, desc.signals.def, [&](gui::DynamicRow& row) -> imtk::item_result {
			imtk::w::widget_row components;
			components.subwidgets.push_back(std::make_unique<imtk::w::generic_widget>([&]() -> imtk::item_result {
				std::string& element = desc.signals.edit.buffer()[row.Index()];

				imtk::outline outline;
				auto result = imtk::w::bound_widget<std::string>(element).draw();

				if (!signal_id_counter.contains(element))
				{
					outline.draw(imtk::col::warning);
					if (result.state.hovered())
						ImGui::SetTooltip("Signal id is not present in asset");
				}
				else if (local_id_counter.count(element) > 1)
				{
					outline.draw(imtk::col::warning);
					if (result.state.hovered())
						ImGui::SetTooltip("Duplicate signal id listing in route");
				}

				if (result.state.activated())
					row.OnSelect();

				return result;
			}));
			return components.draw();
		}, desc.signals.ui_state);

		DescIO::CheckDynamicListRevertButtons(desc.signals.edit, desc.signals.def);

		desc.signals.CheckUndoAction();
	}

	void SignalDocument::Draw(KeyDesc& desc)
	{
		const auto initial = desc.key.index;
		if (auto row = imtk::prop::make_row_scope(desc.key.label, desc.key.index, desc.key.def_index))
		{
			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([this, &desc]() -> imtk::item_result {
				_stop_listening = false;
				std::optional<detail::KeyInput> key;
				imtk::item_result result = InputListener::DrawKeyListener(_listen_mode, key);
				ImGui::SameLine();
				if (key)
				{
					if (*key != desc.key.Value())
					{
						desc.key.SetValue(*key);
						result.modified = true;
					}
					else
						result.modified = false;
				}
				return result | imtk::w::combo_widget(desc.key.index, desc.key.names).draw();
			}));
		}
		
		if (initial != desc.key.index)
			PushFieldSetAction(desc.key.link.compute_path(), initial, desc.key.index);

		if (auto subform = imtk::prop::subform("Keyboard Mods", { .start_open = true }))
		{
			bool disabled_required_mods[desc.required_mods.Count]{};
			for (size_t i = 0; i < desc.required_mods.Count; ++i)
				disabled_required_mods[i] = (desc.forbidden_mods.value & desc.forbidden_mods.values[i]) && !(desc.required_mods.value & desc.required_mods.values[i]);
			desc.required_mods.draw(disabled_required_mods);

			bool disabled_forbidden_mods[desc.forbidden_mods.Count]{};
			for (size_t i = 0; i < desc.forbidden_mods.Count; ++i)
				disabled_forbidden_mods[i] = (desc.required_mods.value & desc.required_mods.values[i]) && !(desc.forbidden_mods.value & desc.forbidden_mods.values[i]);
			desc.forbidden_mods.draw(disabled_forbidden_mods);
		}

		if (auto subform = imtk::prop::subform("Modifiers"))
			Draw(*desc.modifier);
	}
	
	void SignalDocument::Draw(MouseButtonDesc& desc)
	{
		const auto initial = desc.button.index;
		if (auto row = imtk::prop::make_row_scope(desc.button.label, desc.button.index, desc.button.def_index))
		{
			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([this, &desc]() -> imtk::item_result {
				_stop_listening = false;
				std::optional<detail::MouseButton> mb;
				imtk::item_result result = InputListener::DrawMouseButtonListener(_listen_mode, mb);
				ImGui::SameLine();
				if (mb)
				{
					if (*mb != desc.button.Value())
					{
						desc.button.SetValue(*mb);
						result.modified = true;
					}
					else
						result.modified = false;
				}
				return result | imtk::w::combo_widget(desc.button.index, desc.button.names).draw();
			}));
		}

		if (initial != desc.button.index)
			PushFieldSetAction(desc.button.link.compute_path(), initial, desc.button.index);

		if (auto subform = imtk::prop::subform("Keyboard Mods", { .start_open = true }))
		{
			bool disabled_required_mods[desc.required_mods.Count]{};
			for (size_t i = 0; i < desc.required_mods.Count; ++i)
				disabled_required_mods[i] = (desc.forbidden_mods.value & desc.forbidden_mods.values[i]) && !(desc.required_mods.value & desc.required_mods.values[i]);
			desc.required_mods.draw(disabled_required_mods);

			bool disabled_forbidden_mods[desc.forbidden_mods.Count]{};
			for (size_t i = 0; i < desc.forbidden_mods.Count; ++i)
				disabled_forbidden_mods[i] = (desc.required_mods.value & desc.required_mods.values[i]) && !(desc.forbidden_mods.value & desc.forbidden_mods.values[i]);
			desc.forbidden_mods.draw(disabled_forbidden_mods);
		}

		if (auto subform = imtk::prop::subform("Modifiers"))
			Draw(*desc.modifier);
	}
	
	void SignalDocument::Draw(GamepadButtonDesc& desc)
	{
		const auto initial = desc.button.index;
		if (auto row = imtk::prop::make_row_scope(desc.button.label, desc.button.index, desc.button.def_index))
		{
			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([this, &desc]() -> imtk::item_result {
				_stop_listening = false;
				std::optional<GLenum> button;
				imtk::item_result result = InputListener::DrawGamepadButtonListener(_listen_mode, button);
				ImGui::SameLine();
				if (button)
				{
					if (*button != desc.button.Value())
					{
						desc.button.SetValue(*button);
						result.modified = true;
					}
					else
						result.modified = false;
				}
				return result | imtk::w::combo_widget(desc.button.index, desc.button.names).draw();
			}));
		}

		if (initial != desc.button.index)
			PushFieldSetAction(desc.button.link.compute_path(), initial, desc.button.index);

		if (auto subform = imtk::prop::subform("Modifiers"))
			Draw(*desc.modifier);
	}
	
	void SignalDocument::Draw(GamepadAxis1DDesc& desc)
	{
		const auto initial = desc.axis.index;
		if (auto row = imtk::prop::make_row_scope(desc.axis.label, desc.axis.index, desc.axis.def_index))
		{
			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([this, &desc]() -> imtk::item_result {
				_stop_listening = false;
				std::optional<GLenum> axis;
				imtk::item_result result = InputListener::DrawGamepadAxis1DListener(_listen_mode, axis);
				ImGui::SameLine();
				if (axis)
				{
					if (*axis != desc.axis.Value())
					{
						desc.axis.SetValue(*axis);
						result.modified = true;
					}
					else
						result.modified = false;
				}
				return result | imtk::w::combo_widget(desc.axis.index, desc.axis.names).draw();
			}));
		}

		if (initial != desc.axis.index)
			PushFieldSetAction(desc.axis.link.compute_path(), initial, desc.axis.index);

		desc.deadzone.draw();
		if (auto subform = imtk::prop::subform("Modifiers"))
			Draw(*desc.modifier);
	}
	
	void SignalDocument::Draw(GamepadAxis2DDesc& desc)
	{
		const detail::GamepadAxis2D og = desc.axis.value;
		int int_value = static_cast<int>(desc.axis.value);
		const int int_default = static_cast<int>(desc.axis.def);

		if (auto row = imtk::prop::row_scope(desc.axis.label, int_value, int_default))
		{
			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([this, &desc, &int_value]() -> imtk::item_result {
				_stop_listening = false;
				std::optional<detail::GamepadAxis2D> axis;
				imtk::item_result result = InputListener::DrawGamepadAxis2DListener(_listen_mode, axis);
				ImGui::SameLine();
				if (axis)
				{
					if (static_cast<int>(*axis) != int_value)
					{
						int_value = static_cast<int>(*axis);
						result.modified = true;
					}
					else
						result.modified = false;
				}
				return result | imtk::w::combo_widget(int_value, desc.axis.ComboNames()).draw();
			}));
		}

		desc.axis.value = static_cast<detail::GamepadAxis2D>(int_value);
		if (og != desc.axis.value)
			PushFieldSetAction(desc.axis.link.compute_path(), og, desc.axis.value);

		desc.deadzone.draw();
		if (auto subform = imtk::prop::subform("Modifiers"))
			Draw(*desc.modifier);
	}
	
	void SignalDocument::Draw(CursorPosDesc& desc)
	{
		IMTK_DRAW_FIELDS(CURSOR_POS_PARTIAL_GENERATOR);
		if (auto subform = imtk::prop::subform("Modifiers"))
			Draw(*desc.modifier);
	}
	
	void SignalDocument::Draw(ScrollDesc& desc)
	{
		IMTK_DRAW_FIELDS(SCROLL_PARTIAL_GENERATOR);
		if (auto subform = imtk::prop::subform("Modifiers"))
			Draw(*desc.modifier);
	}

	void SignalDocument::Draw(Modifier0dDesc& desc)
	{
		IMTK_DRAW_FIELDS(MODIFIER_0D_PARTIAL_GENERATOR);
		Draw(desc.base);
	}
	
	void SignalDocument::Draw(Modifier1dDesc& desc)
	{
		IMTK_DRAW_FIELDS(MODIFIER_1D_PARTIAL_GENERATOR);
		Draw(desc.base);
	}
	
	void SignalDocument::Draw(Modifier2dDesc& desc)
	{
		IMTK_DRAW_FIELDS(MODIFIER_2D_PARTIAL_GENERATOR);
		Draw(desc.base);
	}
	
	void SignalDocument::Draw(ModifierBaseDesc& desc)
	{
		IMTK_DRAW_FIELDS(MODIFIER_BASE_GENERATOR);
	}

	void SignalDocument::Load(imtk::toml_node node, SignalFullDesc& desc)
	{
		const toml::array* signal_array = desc.signals.subnode(node).as_array();
		if (signal_array && !signal_array->empty())
		{
			for (size_t i = 0; i < signal_array->size(); ++i)
				desc.signals.push_back();
			
			for (size_t i = 0; i < desc.signals.size(); ++i)
				Load(imtk::toml_node(*signal_array->get(i)), desc.signals[i]);
		}

		const toml::array* route_array = desc.routes.subnode(node).as_array();
		if (route_array && !route_array->empty())
		{
			for (size_t i = 0; i < route_array->size(); ++i)
				desc.routes.push_back();

			for (size_t i = 0; i < desc.routes.size(); ++i)
				Load(imtk::toml_node(*route_array->get(i)), desc.routes[i]);
		}
	}

	void SignalDocument::Load(imtk::toml_node node, SignalDesc& desc)
	{
		IMTK_LOAD_FIELDS(SIGNAL_PARTIAL_GENERATOR);

		switch (desc.binding.value)
		{
#define SWITCH_CASE(T) \
		case detail::SignalBindingType::T: \
		{ \
			T##Desc subdesc; \
			Load(node, subdesc); \
			desc.variant.set(std::move(subdesc)); \
			break; \
		}

			BINDING_TYPE_GENERATOR(SWITCH_CASE)

#undef SWITCH_CASE
		}
	}

	void SignalDocument::Load(imtk::toml_node node, RouteDesc& desc)
	{
		IMTK_LOAD_FIELDS(ROUTE_GENERATOR);
	}

	void SignalDocument::Load(imtk::toml_node node, KeyDesc& desc)
	{
		IMTK_LOAD_FIELDS(KEY_PARTIAL_GENERATOR);
		Load(desc.modifier.subnode(node), *desc.modifier);
	}

	void SignalDocument::Load(imtk::toml_node node, MouseButtonDesc& desc)
	{
		IMTK_LOAD_FIELDS(MOUSE_BUTTON_PARTIAL_GENERATOR);
		Load(desc.modifier.subnode(node), *desc.modifier);
	}

	void SignalDocument::Load(imtk::toml_node node, GamepadButtonDesc& desc)
	{
		IMTK_LOAD_FIELDS(GAMEPAD_BUTTON_PARTIAL_GENERATOR);
		Load(desc.modifier.subnode(node), *desc.modifier);
	}
	
	void SignalDocument::Load(imtk::toml_node node, GamepadAxis1DDesc& desc)
	{
		IMTK_LOAD_FIELDS(GAMEPAD_AXIS_1D_PARTIAL_GENERATOR);
		Load(desc.modifier.subnode(node), *desc.modifier);
	}
	
	void SignalDocument::Load(imtk::toml_node node, GamepadAxis2DDesc& desc)
	{
		IMTK_LOAD_FIELDS(GAMEPAD_AXIS_2D_PARTIAL_GENERATOR);
		Load(desc.modifier.subnode(node), *desc.modifier);
	}
	
	void SignalDocument::Load(imtk::toml_node node, CursorPosDesc& desc)
	{
		IMTK_LOAD_FIELDS(CURSOR_POS_PARTIAL_GENERATOR);
		Load(desc.modifier.subnode(node), *desc.modifier);
	}
	
	void SignalDocument::Load(imtk::toml_node node, ScrollDesc& desc)
	{
		IMTK_LOAD_FIELDS(SCROLL_PARTIAL_GENERATOR);
		Load(desc.modifier.subnode(node), *desc.modifier);
	}
	
	void SignalDocument::Load(imtk::toml_node node, Modifier0dDesc& desc)
	{
		IMTK_LOAD_FIELDS(MODIFIER_0D_PARTIAL_GENERATOR);
		Load(node, desc.base);
	}
	
	void SignalDocument::Load(imtk::toml_node node, Modifier1dDesc& desc)
	{
		IMTK_LOAD_FIELDS(MODIFIER_1D_PARTIAL_GENERATOR);
		Load(node, desc.base);
	}
	
	void SignalDocument::Load(imtk::toml_node node, Modifier2dDesc& desc)
	{
		IMTK_LOAD_FIELDS(MODIFIER_2D_PARTIAL_GENERATOR);
		Load(node, desc.base);
	}
	
	void SignalDocument::Load(imtk::toml_node node, ModifierBaseDesc& desc)
	{
		IMTK_LOAD_FIELDS(MODIFIER_BASE_GENERATOR);
	}

	void SignalDocument::Dump(toml::table& table, SignalFullDesc& desc)
	{
		toml::array signal_array;
		for (auto& d : desc.signals)
			Dump(signal_array.emplace_back<toml::table>(), d);
		desc.signals.dump_into(table, std::move(signal_array));

		toml::array route_array;
		for (auto& d : desc.routes)
			Dump(route_array.emplace_back<toml::table>(), d);
		desc.routes.dump_into(table, std::move(route_array));
	}

	void SignalDocument::Dump(toml::table& table, SignalDesc& desc)
	{
		IMTK_DUMP_FIELDS(SIGNAL_PARTIAL_GENERATOR);
		desc.variant.visit([this, &table](auto& desc) { Dump(table, desc); });
	}

	void SignalDocument::Dump(toml::table& table, RouteDesc& desc)
	{
		IMTK_DUMP_FIELDS(ROUTE_GENERATOR);
	}

	void SignalDocument::Dump(toml::table& table, KeyDesc& desc)
	{
		IMTK_DUMP_FIELDS(KEY_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, *desc.modifier);
		desc.modifier.dump_into(table, std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, MouseButtonDesc& desc)
	{
		IMTK_DUMP_FIELDS(MOUSE_BUTTON_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, *desc.modifier);
		desc.modifier.dump_into(table, std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, GamepadButtonDesc& desc)
	{
		IMTK_DUMP_FIELDS(GAMEPAD_BUTTON_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, *desc.modifier);
		desc.modifier.dump_into(table, std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, GamepadAxis1DDesc& desc)
	{
		IMTK_DUMP_FIELDS(GAMEPAD_AXIS_1D_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, *desc.modifier);
		desc.modifier.dump_into(table, std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, GamepadAxis2DDesc& desc)
	{
		IMTK_DUMP_FIELDS(GAMEPAD_AXIS_2D_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, *desc.modifier);
		desc.modifier.dump_into(table, std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, CursorPosDesc& desc)
	{
		IMTK_DUMP_FIELDS(CURSOR_POS_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, *desc.modifier);
		desc.modifier.dump_into(table, std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, ScrollDesc& desc)
	{
		IMTK_DUMP_FIELDS(SCROLL_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, *desc.modifier);
		desc.modifier.dump_into(table, std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, Modifier0dDesc& desc)
	{
		IMTK_DUMP_FIELDS(MODIFIER_0D_PARTIAL_GENERATOR);
		Dump(table, desc.base);
	}
	
	void SignalDocument::Dump(toml::table& table, Modifier1dDesc& desc)
	{
		IMTK_DUMP_FIELDS(MODIFIER_1D_PARTIAL_GENERATOR);
		Dump(table, desc.base);
	}
	
	void SignalDocument::Dump(toml::table& table, Modifier2dDesc& desc)
	{
		IMTK_DUMP_FIELDS(MODIFIER_2D_PARTIAL_GENERATOR);
		Dump(table, desc.base);
	}
	
	void SignalDocument::Dump(toml::table& table, ModifierBaseDesc& desc)
	{
		IMTK_DUMP_FIELDS(MODIFIER_BASE_GENERATOR);
	}
}
