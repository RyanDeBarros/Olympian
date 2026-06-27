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

	void SignalDocument::InitImpl()
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
		auto pre_draw = PreDraw();

		_stop_listening = true;
		gui::IDScope scope(this);

		if (ImGui::BeginTabBar(""))
		{
			if (ImGui::BeginTabItem("Signals"))
			{
				Draw(DataPath() / _desc.scratch.subpaths.signals, _desc.scratch.signals);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Routes"))
			{
				Draw(DataPath() / _desc.scratch.subpaths.routes, _desc.scratch.routes);
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
				Load(TOMLNode(table), _desc.disk);
			else
			{
				Notification notif(LogLevel::Error, "cannot load signal - corrupted asset: " + _oly_path.string());
				MainWindow::Instance().PushNotification(std::move(notif));
			}

			MarkClean();
		}
		else
		{
			Load(TOMLNode(), _desc.disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = "1.0";
			_meta.map[detail::Key::Meta_Import] = "1";
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Signal);

			MarkDirty();
		}

		_desc.LoadFromDisk();
		_signal_slots.Init(*_desc.scratch.signals.ListAdapter());
		_route_slots.Init(*_desc.scratch.routes.ListAdapter());
	}

	void SignalDocument::Dump()
	{
		toml::table table;
		Dump(table, _desc.scratch);
		_oly_path.dump_toml(table, _meta);
		_desc.WriteToDisk();
		MarkClean();
	}

	IDoubleDescriptor& SignalDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	void SignalDocument::Draw(DataPath path, VectorDesc<SignalDesc>& desc)
	{
		if (auto form = Form())
		{
			if (auto scope = gui::IDScope("##Signal"))
			{
				gui::PropertyGrid::Key::SetLabel("Select Signal");
				gui::PropertyGrid::Value::AddComponent(comp::Generic([this, &desc]() -> DrawResult {
					return _signal_slots.DrawComboHeader([&desc](size_t i) {
						if (i < desc.Size())
						{
							std::string id = desc[i].id.scratch;
							if (!id.empty())
								return id;
						}
						return "<Signal #" + std::to_string(i) + ">";
					}, "New signal", "Delete signal", "Clear signals");
				}));
				gui::PropertyGrid::SubmitRow();
			}

			if (!desc.Empty())
				Draw(path / desc.Subpath(_signal_slots.active_index), desc[_signal_slots.active_index]);

			if (_signal_slots.ConsumeOps(*desc.ListAdapter()))
				MarkDirty();

			_signal_slots.active_index.ConsumeModified();
		}
	}

	void SignalDocument::Draw(DataPath path, VectorDesc<RouteDesc>& desc)
	{
		if (auto form = Form())
		{
			if (auto scope = gui::IDScope("##Route"))
			{
				gui::PropertyGrid::Key::SetLabel("Select Route");
				gui::PropertyGrid::Value::AddComponent(comp::Generic([this, &desc]() -> DrawResult {
					return _route_slots.DrawComboHeader([&desc](size_t i) {
						if (i < desc.Size())
						{
							std::string id = desc[i].id.scratch;
							if (!id.empty())
								return id;
						}
						return "<Route #" + std::to_string(i) + ">";
					}, "New route", "Delete route", "Clear routes");
				}));
				gui::PropertyGrid::SubmitRow();
			}

			if (!desc.Empty())
				Draw(path / desc.Subpath(_route_slots.active_index), desc[_route_slots.active_index]);

			if (_route_slots.ConsumeOps(*desc.ListAdapter()))
				MarkDirty();

			_route_slots.active_index.ConsumeModified();
		}
	}

	Counter<std::string> SignalDocument::GetSignalIDCounter() const
	{
		Counter<std::string> id_counter;

		for (const auto& subdesc : _desc.scratch.signals)
			id_counter.increment(subdesc.id.scratch);

		return id_counter;
	}
	
	Counter<std::string> SignalDocument::GetRouteIDCounter() const
	{
		Counter<std::string> id_counter;

		for (const auto& subdesc : _desc.scratch.routes)
			id_counter.increment(subdesc.id.scratch);

		return id_counter;
	}

	Counter<std::string> SignalDocument::GetIDCounter() const
	{
		Counter<std::string> id_counter = GetSignalIDCounter();
		id_counter.accumulate(GetRouteIDCounter());
		return id_counter;
	}

	void SignalDocument::Draw(DataPath path, SignalDesc& desc)
	{
		gui::Outline dup_outline;
		
		DRAW_FIELD(id);
		if (GetIDCounter().count(desc.id.scratch) > 1)
		{
			if (gui::PropertyGrid::GetFullDrawResult().IsHovered())
				ImGui::SetTooltip("Duplicate signal/route id");

			dup_outline.Draw(Color::Error);
		}

		DRAW_FIELD(binding);

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

		desc.variant.Visit([this, path = path / desc.subpaths.variant](auto& desc) { Draw(path, desc); });
	}
	
	void SignalDocument::Draw(DataPath path, RouteDesc& desc)
	{
		auto signal_id_counter = GetSignalIDCounter();
		auto id_counter = GetIDCounter();

		Counter<std::string> local_id_counter;
		local_id_counter.accumulate(desc.signals.scratch);

		gui::Outline dup_outline;

		DRAW_FIELD(id);
		if (id_counter.count(desc.id.scratch) > 1)
		{
			if (gui::PropertyGrid::GetFullDrawResult().IsHovered())
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

	void SignalDocument::Draw(DataPath path, KeyDesc& desc)
	{
		gui::PropertyGrid::Value::AddComponent(comp::Generic([this, &desc]() -> DrawResult {
			_stop_listening = false;
			std::optional<detail::KeyInput> key;
			DrawResult result = InputListener::DrawKeyListener(_listen_mode, key);
			if (key)
			{
				if (*key != desc.key.Scratch())
				{
					desc.key.SetScratch(*key);
					result.SetDirty(true);
				}
				else
					result.SetDirty(false);
			}
			return result;
		}));
		DRAW_FIELD(key);

		bool disabled_required_mods[desc.required_mods.Count]{};
		for (size_t i = 0; i < desc.required_mods.Count; ++i)
			disabled_required_mods[i] = (desc.forbidden_mods.scratch & desc.forbidden_mods.values[i]) && !(desc.required_mods.scratch & desc.required_mods.values[i]);
		desc.required_mods.Draw(path / desc.subpaths.required_mods, disabled_required_mods);

		bool disabled_forbidden_mods[desc.forbidden_mods.Count]{};
		for (size_t i = 0; i < desc.forbidden_mods.Count; ++i)
			disabled_forbidden_mods[i] = (desc.required_mods.scratch & desc.required_mods.values[i]) && !(desc.forbidden_mods.scratch & desc.forbidden_mods.values[i]);
		desc.forbidden_mods.Draw(path / desc.subpaths.forbidden_mods, disabled_forbidden_mods);

		if (auto subform = Subform("Modifiers"))
			Draw(path / desc.subpaths.modifier, desc.modifier);
	}
	
	void SignalDocument::Draw(DataPath path, MouseButtonDesc& desc)
	{
		gui::PropertyGrid::Value::AddComponent(comp::Generic([this, &desc]() -> DrawResult {
			_stop_listening = false;
			std::optional<detail::MouseButton> mb;
			DrawResult result = InputListener::DrawMouseButtonListener(_listen_mode, mb);
			if (mb)
			{
				if (*mb != desc.button.Scratch())
				{
					desc.button.SetScratch(*mb);
					result.SetDirty(true);
				}
				else
					result.SetDirty(false);
			}
			return result;
		}));
		DRAW_FIELD(button);

		bool disabled_required_mods[desc.required_mods.Count]{};
		for (size_t i = 0; i < desc.required_mods.Count; ++i)
			disabled_required_mods[i] = (desc.forbidden_mods.scratch & desc.forbidden_mods.values[i]) && !(desc.required_mods.scratch & desc.required_mods.values[i]);
		desc.required_mods.Draw(path / desc.subpaths.required_mods, disabled_required_mods);

		bool disabled_forbidden_mods[desc.forbidden_mods.Count]{};
		for (size_t i = 0; i < desc.forbidden_mods.Count; ++i)
			disabled_forbidden_mods[i] = (desc.required_mods.scratch & desc.required_mods.values[i]) && !(desc.forbidden_mods.scratch & desc.forbidden_mods.values[i]);
		desc.forbidden_mods.Draw(path / desc.subpaths.forbidden_mods, disabled_forbidden_mods);

		if (auto subform = Subform("Modifiers"))
			Draw(path / desc.subpaths.modifier, desc.modifier);
	}
	
	void SignalDocument::Draw(DataPath path, GamepadButtonDesc& desc)
	{
		gui::PropertyGrid::Value::AddComponent(comp::Generic([this, &desc]() -> DrawResult {
			_stop_listening = false;
			std::optional<GLenum> button;
			DrawResult result = InputListener::DrawGamepadButtonListener(_listen_mode, button);
			if (button)
			{
				if (*button != desc.button.Scratch())
				{
					desc.button.SetScratch(*button);
					result.SetDirty(true);
				}
				else
					result.SetDirty(false);
			}
			return result;
		}));
		DRAW_FIELD(button);

		if (auto subform = Subform("Modifiers"))
			Draw(path / desc.subpaths.modifier, desc.modifier);
	}
	
	void SignalDocument::Draw(DataPath path, GamepadAxis1DDesc& desc)
	{
		gui::PropertyGrid::Value::AddComponent(comp::Generic([this, &desc]() -> DrawResult {
			_stop_listening = false;
			std::optional<GLenum> axis;
			DrawResult result = InputListener::DrawGamepadAxis1DListener(_listen_mode, axis);
			if (axis)
			{
				if (*axis != desc.axis.Scratch())
				{
					desc.axis.SetScratch(*axis);
					result.SetDirty(true);
				}
				else
					result.SetDirty(false);
			}
			return result;
		}));
		DRAW_FIELD(axis);

		DRAW_FIELD(deadzone);
		if (auto subform = Subform("Modifiers"))
			Draw(path / desc.subpaths.modifier, desc.modifier);
	}
	
	void SignalDocument::Draw(DataPath path, GamepadAxis2DDesc& desc)
	{
		gui::PropertyGrid::Value::AddComponent(comp::Generic([this, &desc]() -> DrawResult {
			_stop_listening = false;
			std::optional<detail::GamepadAxis2D> axis;
			DrawResult result = InputListener::DrawGamepadAxis2DListener(_listen_mode, axis);
			if (axis)
			{
				if (*axis != desc.axis.scratch)
				{
					desc.axis.scratch = *axis;
					result.SetDirty(true);
				}
				else
					result.SetDirty(false);
			}
			return result;
		}));
		DRAW_FIELD(axis);

		DRAW_FIELD(deadzone);
		if (auto subform = Subform("Modifiers"))
			Draw(path / desc.subpaths.modifier, desc.modifier);
	}
	
	void SignalDocument::Draw(DataPath path, CursorPosDesc& desc)
	{
		DRAW_FIELDS(CURSOR_POS_PARTIAL_GENERATOR);
		if (auto subform = Subform("Modifiers"))
			Draw(path / desc.subpaths.modifier, desc.modifier);
	}
	
	void SignalDocument::Draw(DataPath path, ScrollDesc& desc)
	{
		DRAW_FIELDS(SCROLL_PARTIAL_GENERATOR);
		if (auto subform = Subform("Modifiers"))
			Draw(path / desc.subpaths.modifier, desc.modifier);
	}

	void SignalDocument::Draw(DataPath path, Modifier0dDesc& desc)
	{
		DRAW_FIELDS(MODIFIER_0D_PARTIAL_GENERATOR);
		Draw(path / desc.subpaths.base, desc.base);
	}
	
	void SignalDocument::Draw(DataPath path, Modifier1dDesc& desc)
	{
		DRAW_FIELDS(MODIFIER_1D_PARTIAL_GENERATOR);
		Draw(path / desc.subpaths.base, desc.base);
	}
	
	void SignalDocument::Draw(DataPath path, Modifier2dDesc& desc)
	{
		DRAW_FIELDS(MODIFIER_2D_PARTIAL_GENERATOR);
		Draw(path / desc.subpaths.base, desc.base);
	}
	
	void SignalDocument::Draw(DataPath path, ModifierBaseDesc& desc)
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
		desc.signals.Visit([this, &subtable](SignalDesc& desc) { Dump(subtable, desc); });
		table.insert_or_assign(detail::encode_key(desc.signals_key), std::move(subtable));

		subtable.clear();
		desc.routes.Visit([this, &subtable](RouteDesc& desc) { Dump(subtable, desc); });
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
