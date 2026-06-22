#include "SignalDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"

#include "gui/scopes/IDScope.h"
#include "gui/scopes/Form.h"
#include "gui/scopes/Subform.h"
#include "gui/graphics/Outline.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	const char* SignalDocument::GetVersion()
	{
		return "1.0";
	}

	void SignalDocument::Init()
	{
		if (!GetOlyPath().is_resource())
		{
			Notification notif(LogLevel::Warning, "Asset is not located in resource folder");
			MainWindow::Instance().PushNotification(std::move(notif));
		}

		Load();
	}

	void SignalDocument::Draw()
	{
		auto grid = Grid();

		_stop_listening = true;
		gui::IDScope scope(this);

		if (ImGui::BeginTabBar(""))
		{
			if (ImGui::BeginTabItem("Signals"))
			{
				DrawSignals();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Routes"))
			{
				DrawRoutes();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		if (_stop_listening)
			_listen_mode = ListenMode::None;
	}

	void SignalDocument::Load()
	{
		if (_oly_path.is_file())
		{
			_meta = detail::MetaSplitter::decode_meta(_oly_path);

			toml::table table;
			std::string err = _oly_path.load_toml(table);
			if (err.empty())
				Load(TOMLNode(table), _disk);
			else
			{
				Notification notif(LogLevel::Error, "cannot load signal - corrupted asset: " + _oly_path.string());
				MainWindow::Instance().PushNotification(std::move(notif));
			}

			MarkClean();
		}
		else
		{
			Load(TOMLNode(), _disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = "1.0";
			_meta.map[detail::Key::Meta_Import] = "1";
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Signal);

			MarkDirty();
		}

		_scratch = _disk;
		_signal_slots.Init(*_scratch.signals.ListAdapter());
		_route_slots.Init(*_scratch.routes.ListAdapter());
	}

	void SignalDocument::Dump()
	{
		toml::table table;
		Dump(table, _scratch);
		_oly_path.dump_toml(table, _meta);
		_disk = _scratch;
		MarkClean();
	}

	void SignalDocument::DrawSignals()
	{
		if (auto form = Form())
		{
			if (auto scope = gui::IDScope("##Signal"))
			{
				gui::PropertyGrid::Key::SetLabel("Select Signal");
				gui::PropertyGrid::Value::AddComponent({ [this]() -> DrawResult {
					return _signal_slots.DrawComboHeader([this](size_t i) {
						if (i < _scratch.signals.Size())
						{
							std::string id = _scratch.signals[i].id.scratch;
							if (!id.empty())
								return id;
						}
						return "<Signal #" + std::to_string(i) + ">";
					}, "New signal", "Delete signal", "Clear signals");
				} });
				gui::PropertyGrid::SubmitRow();
			}

			if (!_scratch.signals.Empty())
				Draw(_scratch.signals[_signal_slots.active_index]);

			if (_signal_slots.ConsumeOps(*_scratch.signals.ListAdapter()))
				MarkDirty();

			_signal_slots.active_index.ConsumeModified();
		}
	}

	void SignalDocument::DrawRoutes()
	{
		if (auto form = Form())
		{
			if (auto scope = gui::IDScope("##Route"))
			{
				gui::PropertyGrid::Key::SetLabel("Select Route");
				gui::PropertyGrid::Value::AddComponent({ [this]() -> DrawResult {
					return _route_slots.DrawComboHeader([this](size_t i) {
						if (i < _scratch.routes.Size())
						{
							std::string id = _scratch.routes[i].id.scratch;
							if (!id.empty())
								return id;
						}
						return "<Route #" + std::to_string(i) + ">";
					}, "New route", "Delete route", "Clear routes");
				} });
				gui::PropertyGrid::SubmitRow();
			}

			if (!_scratch.routes.Empty())
				Draw(_scratch.routes[_route_slots.active_index]);

			if (_route_slots.ConsumeOps(*_scratch.routes.ListAdapter()))
				MarkDirty();

			_route_slots.active_index.ConsumeModified();
		}
	}

	Counter<std::string> SignalDocument::GetSignalIDCounter() const
	{
		Counter<std::string> id_counter;

		for (const auto& subdesc : _scratch.signals)
			id_counter.increment(subdesc.id.scratch);

		return id_counter;
	}
	
	Counter<std::string> SignalDocument::GetRouteIDCounter() const
	{
		Counter<std::string> id_counter;

		for (const auto& subdesc : _scratch.routes)
			id_counter.increment(subdesc.id.scratch);

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
		gui::Outline dup_outline;
		
		desc.id.Draw();
		if (GetIDCounter().count(desc.id.scratch) > 1)
		{
			if (gui::PropertyGrid::Value::GetDrawResult().IsHovered())
				ImGui::SetTooltip("Duplicate signal/route id");

			dup_outline.Draw(Color::Error);
		}

		desc.binding.Draw();

		switch (desc.binding.scratch)
		{
#define SWITCH_CASE(T) \
		case detail::SignalBindingType::T: \
		{ \
			if (!desc.variant.TryGet<T##Desc>()) \
				desc.variant.Set<T##Desc>(); \
			break; \
		}

			BINDING_TYPE_GENERATOR(SWITCH_CASE)

#undef SWITCH_CASE
		}

		desc.variant.Visit([this](auto& desc) { Draw(desc); });
	}
	
	void SignalDocument::Draw(RouteDesc& desc)
	{
		auto signal_id_counter = GetSignalIDCounter();
		auto id_counter = GetIDCounter();

		Counter<std::string> local_id_counter;
		local_id_counter.accumulate(desc.signals.scratch);

		gui::Outline dup_outline;

		desc.id.Draw();
		if (id_counter.count(desc.id.scratch) > 1)
		{
			if (gui::PropertyGrid::Value::GetDrawResult().IsHovered())
				ImGui::SetTooltip("Duplicate signal/route id");

			dup_outline.Draw(Color::Error);
		}

		DescIO::DrawDynamicListRevertButtons(desc.signals.scratch, desc.signals.def);

		DescIO::DrawDynamicList(desc.signals.label, desc.signals.scratch, desc.signals.def, [&](gui::DynamicRow& row) {
			std::string& element = desc.signals.scratch[row.Index()];

			DrawResult result;

			ImGui::SameLine();
			gui::Outline outline;
			auto item_result = gui::InputData<std::string>{}("##Item", element);
			result |= item_result;

			if (!signal_id_counter.contains(element))
			{
				outline.Draw(Color::Warning);
				if (item_result.IsHovered())
					ImGui::SetTooltip("Signal id is not present in asset");
			}
			else if (local_id_counter.count(element) > 1)
			{
				outline.Draw(Color::Warning);
				if (item_result.IsHovered())
					ImGui::SetTooltip("Duplicate signal id listing in route");
			}

			if (ImGui::IsItemActivated())
				row.OnSelect();

			return result;
		}, desc.signals.ui_state);

		DescIO::CheckDynamicListRevertButtons(desc.signals.scratch, desc.signals.def);
	}

	void SignalDocument::Draw(KeyDesc& desc)
	{
		gui::PropertyGrid::Value::AddComponent({ [this, &desc]() -> DrawResult {
			_stop_listening = false;
			std::optional<detail::KeyInput> key;
			DrawResult result = InputListener::DrawKeyListener(_listen_mode, key);
			if (key)
			{
				if (*key != desc.key.Scratch())
				{
					desc.key.SetScratch(*key);
					return true;
				}
				else
					result.SetDirty(false);
			}
			return result;
		} });
		desc.key.Draw();

		bool disabled_required_mods[desc.required_mods.Count]{};
		for (size_t i = 0; i < desc.required_mods.Count; ++i)
			disabled_required_mods[i] = desc.forbidden_mods.scratch & desc.forbidden_mods.values[i];
		desc.required_mods.Draw(disabled_required_mods);

		bool disabled_forbidden_mods[desc.forbidden_mods.Count]{};
		for (size_t i = 0; i < desc.forbidden_mods.Count; ++i)
			disabled_forbidden_mods[i] = desc.required_mods.scratch & desc.required_mods.values[i];
		desc.forbidden_mods.Draw(disabled_forbidden_mods);

		if (auto subform = Subform("Modifiers"))
			Draw(desc.modifier);
	}
	
	void SignalDocument::Draw(MouseButtonDesc& desc)
	{
		// TODO v9.1
		//{
		//	DescIO::PrepareValue(desc.button.label);
		//	gui::IDScope scope(&desc.button);

		//	_stop_listening = false;
		//	if (auto mb = InputListener::DrawMouseButtonListener(_listen_mode))
		//	{
		//		if (*mb != desc.button.Scratch())
		//		{
		//			desc.button.SetScratch(*mb);
		//			MarkDirty();
		//		}
		//	}

		//	ImGui::SameLine();
		//	if (gui::InputData<int>{}("", desc.button.scratch, desc.button.names, desc.button.count))
		//		MarkDirty();

		//	if (DescIO::CheckRevertButton(desc.button.scratch, desc.button.def_index))
		//		MarkDirty();
		//}

		bool disabled_required_mods[desc.required_mods.Count]{};
		for (size_t i = 0; i < desc.required_mods.Count; ++i)
			disabled_required_mods[i] = desc.forbidden_mods.scratch & desc.forbidden_mods.values[i];
		desc.required_mods.Draw(disabled_required_mods);

		bool disabled_forbidden_mods[desc.forbidden_mods.Count]{};
		for (size_t i = 0; i < desc.forbidden_mods.Count; ++i)
			disabled_forbidden_mods[i] = desc.required_mods.scratch & desc.required_mods.values[i];
		desc.forbidden_mods.Draw(disabled_forbidden_mods);

		if (auto subform = Subform("Modifiers"))
			Draw(desc.modifier);
	}
	
	void SignalDocument::Draw(GamepadButtonDesc& desc)
	{
		// TODO v9.1
		//{
		//	DescIO::PrepareValue(desc.button.label);
		//	gui::IDScope scope(&desc.button);

		//	_stop_listening = false;
		//	if (auto button = InputListener::DrawGamepadButtonListener(_listen_mode))
		//	{
		//		if (*button != desc.button.Scratch())
		//		{
		//			desc.button.SetScratch(*button);
		//			MarkDirty();
		//		}
		//	}

		//	ImGui::SameLine();
		//	if (gui::InputData<int>{}("", desc.button.scratch, desc.button.names, desc.button.count))
		//		MarkDirty();

		//	if (DescIO::CheckRevertButton(desc.button.scratch, desc.button.def_index))
		//		MarkDirty();
		//}

		if (auto subform = Subform("Modifiers"))
			Draw(desc.modifier);
	}
	
	void SignalDocument::Draw(GamepadAxis1DDesc& desc)
	{
		// TODO v9.1
		//{
		//	DescIO::PrepareValue(desc.axis.label);
		//	gui::IDScope scope(&desc.axis);

		//	_stop_listening = false;
		//	if (auto axis = InputListener::DrawGamepadAxis1DListener(_listen_mode))
		//	{
		//		if (*axis != desc.axis.Scratch())
		//		{
		//			desc.axis.SetScratch(*axis);
		//			MarkDirty();
		//		}
		//	}

		//	ImGui::SameLine();
		//	if (gui::InputData<int>{}("", desc.axis.scratch, desc.axis.names, desc.axis.count))
		//		MarkDirty();

		//	if (DescIO::CheckRevertButton(desc.axis.scratch, desc.axis.def_index))
		//		MarkDirty();
		//}

		desc.deadzone.Draw();
		if (auto subform = Subform("Modifiers"))
			Draw(desc.modifier);
	}
	
	void SignalDocument::Draw(GamepadAxis2DDesc& desc)
	{
		// TODO v9.1
		//{
		//	DescIO::PrepareValue(desc.axis.label);
		//	gui::IDScope scope(&desc.axis);

		//	_stop_listening = false;
		//	if (auto axis = InputListener::DrawGamepadAxis2DListener(_listen_mode))
		//	{
		//		if (*axis != desc.axis.scratch)
		//		{
		//			desc.axis.scratch = *axis;
		//			MarkDirty();
		//		}
		//	}

		//	ImGui::SameLine();

		//	if (DescIO::DrawCombo("", desc.axis.scratch))
		//		MarkDirty();

		//	if (DescIO::CheckRevertButton(desc.axis.scratch, desc.axis.def))
		//		MarkDirty();
		//}

		desc.deadzone.Draw();
		if (auto subform = Subform("Modifiers"))
			Draw(desc.modifier);
	}
	
	void SignalDocument::Draw(CursorPosDesc& desc)
	{
		DRAW_FIELDS(CURSOR_POS_PARTIAL_GENERATOR);
		if (auto subform = Subform("Modifiers"))
			Draw(desc.modifier);
	}
	
	void SignalDocument::Draw(ScrollDesc& desc)
	{
		DRAW_FIELDS(SCROLL_PARTIAL_GENERATOR);
		if (auto subform = Subform("Modifiers"))
			Draw(desc.modifier);
	}

	void SignalDocument::Draw(Modifier0dDesc& desc)
	{
		DRAW_FIELDS(MODIFIER_0D_PARTIAL_GENERATOR);
		Draw(desc.base);
	}
	
	void SignalDocument::Draw(Modifier1dDesc& desc)
	{
		DRAW_FIELDS(MODIFIER_1D_PARTIAL_GENERATOR);
		Draw(desc.base);
	}
	
	void SignalDocument::Draw(Modifier2dDesc& desc)
	{
		DRAW_FIELDS(MODIFIER_2D_PARTIAL_GENERATOR);
		Draw(desc.base);
	}
	
	void SignalDocument::Draw(ModifierBaseDesc& desc)
	{
		DRAW_FIELDS(MODIFIER_BASE_GENERATOR);
	}

	void SignalDocument::Load(TOMLNode node, SignalFullDesc& desc)
	{
		TOMLArray signal_array = node[detail::encode_key(desc.signals_key)].as_array();
		if (signal_array && !signal_array->empty())
		{
			for (size_t i = 0; i < signal_array->size(); ++i)
				desc.signals.PushBack();

			desc.signals.VisitIndexed([this, &signal_array](size_t i, auto& d) { Load(TOMLNode(*signal_array->get(i)), d); });
		}

		TOMLArray route_array = node[detail::encode_key(desc.routes_key)].as_array();
		if (route_array && !route_array->empty())
		{
			for (size_t i = 0; i < route_array->size(); ++i)
				desc.routes.PushBack();

			desc.routes.VisitIndexed([this, &route_array](size_t i, auto& d) { Load(TOMLNode(*route_array->get(i)), d); });
		}
	}

	void SignalDocument::Load(TOMLNode node, SignalDesc& desc)
	{
		LOAD_FIELDS(SIGNAL_PARTIAL_GENERATOR);

		detail::SignalBindingType type = detail::SignalBindingType::Key;
		if (auto v = node[detail::encode_key(desc.modifier_key)].value<int64_t>())
			type = static_cast<detail::SignalBindingType>(*v);

		switch (type)
		{
#define SWITCH_CASE(T) \
		case detail::SignalBindingType::T: \
		{ \
			T##Desc subdesc; \
			Load(node, subdesc); \
			desc.variant.Set(std::move(subdesc)); \
			break; \
		}

			BINDING_TYPE_GENERATOR(SWITCH_CASE)

#undef SWITCH_CASE
		}
	}

	void SignalDocument::Load(TOMLNode node, RouteDesc& desc)
	{
		LOAD_FIELDS(ROUTE_GENERATOR);
	}

	void SignalDocument::Load(TOMLNode node, KeyDesc& desc)
	{
		LOAD_FIELDS(KEY_PARTIAL_GENERATOR);
		Load(node[detail::encode_key(SignalDesc::modifier_key)], desc.modifier);
	}

	void SignalDocument::Load(TOMLNode node, MouseButtonDesc& desc)
	{
		LOAD_FIELDS(MOUSE_BUTTON_PARTIAL_GENERATOR);
		Load(node[detail::encode_key(SignalDesc::modifier_key)], desc.modifier);
	}

	void SignalDocument::Load(TOMLNode node, GamepadButtonDesc& desc)
	{
		LOAD_FIELDS(GAMEPAD_BUTTON_PARTIAL_GENERATOR);
		Load(node[detail::encode_key(SignalDesc::modifier_key)], desc.modifier);
	}
	
	void SignalDocument::Load(TOMLNode node, GamepadAxis1DDesc& desc)
	{
		LOAD_FIELDS(GAMEPAD_AXIS_1D_PARTIAL_GENERATOR);
		Load(node[detail::encode_key(SignalDesc::modifier_key)], desc.modifier);
	}
	
	void SignalDocument::Load(TOMLNode node, GamepadAxis2DDesc& desc)
	{
		LOAD_FIELDS(GAMEPAD_AXIS_2D_PARTIAL_GENERATOR);
		Load(node[detail::encode_key(SignalDesc::modifier_key)], desc.modifier);
	}
	
	void SignalDocument::Load(TOMLNode node, CursorPosDesc& desc)
	{
		LOAD_FIELDS(CURSOR_POS_PARTIAL_GENERATOR);
		Load(node[detail::encode_key(SignalDesc::modifier_key)], desc.modifier);
	}
	
	void SignalDocument::Load(TOMLNode node, ScrollDesc& desc)
	{
		LOAD_FIELDS(SCROLL_PARTIAL_GENERATOR);
		Load(node[detail::encode_key(SignalDesc::modifier_key)], desc.modifier);
	}
	
	void SignalDocument::Load(TOMLNode node, Modifier0dDesc& desc)
	{
		LOAD_FIELDS(MODIFIER_0D_PARTIAL_GENERATOR);
		Load(node, desc.base);
	}
	
	void SignalDocument::Load(TOMLNode node, Modifier1dDesc& desc)
	{
		LOAD_FIELDS(MODIFIER_1D_PARTIAL_GENERATOR);
		Load(node, desc.base);
	}
	
	void SignalDocument::Load(TOMLNode node, Modifier2dDesc& desc)
	{
		LOAD_FIELDS(MODIFIER_2D_PARTIAL_GENERATOR);
		Load(node, desc.base);
	}
	
	void SignalDocument::Load(TOMLNode node, ModifierBaseDesc& desc)
	{
		LOAD_FIELDS(MODIFIER_BASE_GENERATOR);
	}

	void SignalDocument::Dump(toml::table& table, SignalFullDesc& desc)
	{
		toml::table subtable;
		_scratch.signals.Visit([this, &subtable](SignalDesc& desc) { Dump(subtable, desc); });
		table.insert_or_assign(detail::encode_key(desc.signals_key), std::move(subtable));

		subtable.clear();
		_scratch.routes.Visit([this, &subtable](RouteDesc& desc) { Dump(subtable, desc); });
		table.insert_or_assign(detail::encode_key(desc.routes_key), std::move(subtable));
	}

	void SignalDocument::Dump(toml::table& table, SignalDesc& desc)
	{
		DUMP_FIELDS(SIGNAL_PARTIAL_GENERATOR);
		desc.variant.Visit([this, &table](auto& desc) { Dump(table, desc); });
	}

	void SignalDocument::Dump(toml::table& table, RouteDesc& desc)
	{
		DUMP_FIELDS(ROUTE_GENERATOR);
	}

	void SignalDocument::Dump(toml::table& table, KeyDesc& desc)
	{
		DUMP_FIELDS(KEY_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.modifier);
		table.insert_or_assign(detail::encode_key(SignalDesc::modifier_key), std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, MouseButtonDesc& desc)
	{
		DUMP_FIELDS(MOUSE_BUTTON_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.modifier);
		table.insert_or_assign(detail::encode_key(SignalDesc::modifier_key), std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, GamepadButtonDesc& desc)
	{
		DUMP_FIELDS(GAMEPAD_BUTTON_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.modifier);
		table.insert_or_assign(detail::encode_key(SignalDesc::modifier_key), std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, GamepadAxis1DDesc& desc)
	{
		DUMP_FIELDS(GAMEPAD_AXIS_1D_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.modifier);
		table.insert_or_assign(detail::encode_key(SignalDesc::modifier_key), std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, GamepadAxis2DDesc& desc)
	{
		DUMP_FIELDS(GAMEPAD_AXIS_2D_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.modifier);
		table.insert_or_assign(detail::encode_key(SignalDesc::modifier_key), std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, CursorPosDesc& desc)
	{
		DUMP_FIELDS(CURSOR_POS_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.modifier);
		table.insert_or_assign(detail::encode_key(SignalDesc::modifier_key), std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, ScrollDesc& desc)
	{
		DUMP_FIELDS(SCROLL_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.modifier);
		table.insert_or_assign(detail::encode_key(SignalDesc::modifier_key), std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, Modifier0dDesc& desc)
	{
		DUMP_FIELDS(MODIFIER_0D_PARTIAL_GENERATOR);
		Dump(table, desc.base);
	}
	
	void SignalDocument::Dump(toml::table& table, Modifier1dDesc& desc)
	{
		DUMP_FIELDS(MODIFIER_1D_PARTIAL_GENERATOR);
		Dump(table, desc.base);
	}
	
	void SignalDocument::Dump(toml::table& table, Modifier2dDesc& desc)
	{
		DUMP_FIELDS(MODIFIER_2D_PARTIAL_GENERATOR);
		Dump(table, desc.base);
	}
	
	void SignalDocument::Dump(toml::table& table, ModifierBaseDesc& desc)
	{
		DUMP_FIELDS(MODIFIER_BASE_GENERATOR);
	}
}
