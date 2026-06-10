#include "SignalDocument.h"

#include "core/MainWindow.h"
#include "core/Logger.h"

#include "gui/IDScope.h"
#include "gui/Subform.h"

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
	}

	void SignalDocument::DrawMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save Changes", "Ctrl+S"))
					Dump();

				if (ImGui::MenuItem("Discard Changes"))
					Load();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void SignalDocument::Load()
	{
		if (_oly_path.exists())
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
			// TODO v8 draw only active index
			_scratch.signals.Visit([this, &form](SignalDesc& desc) { Draw(form, desc); });
		}
	}

	void SignalDocument::DrawRoutes()
	{
		if (auto form = Form())
		{
			// TODO v8 draw only active index
			_scratch.routes.Visit([this, &form](RouteDesc& desc) { Draw(form, desc); });
		}
	}

	void SignalDocument::Draw(Form& form, SignalDesc& desc)
	{
		DRAW_FIELDS(SIGNAL_PARTIAL_GENERATOR);

		desc.variant.Visit([this, &form](auto& desc) { Draw(form, desc); });
	}
	
	void SignalDocument::Draw(Form& form, RouteDesc& desc)
	{
		DRAW_FIELDS(ROUTE_GENERATOR);
	}

	void SignalDocument::Draw(Form& form, KeyDesc& desc)
	{
		DRAW_FIELDS(KEY_PARTIAL_GENERATOR);
		if (auto subform = Subform(form, "Modifiers"))
			Draw(form, desc.modifier);
	}
	
	void SignalDocument::Draw(Form& form, MouseButtonDesc& desc)
	{
		DRAW_FIELDS(MOUSE_BUTTON_PARTIAL_GENERATOR);
		if (auto subform = Subform(form, "Modifiers"))
			Draw(form, desc.modifier);
	}
	
	void SignalDocument::Draw(Form& form, GamepadButtonDesc& desc)
	{
		DRAW_FIELDS(GAMEPAD_BUTTON_PARTIAL_GENERATOR);
		if (auto subform = Subform(form, "Modifiers"))
			Draw(form, desc.modifier);
	}
	
	void SignalDocument::Draw(Form& form, GamepadAxis1dDesc& desc)
	{
		DRAW_FIELDS(GAMEPAD_AXIS_1D_PARTIAL_GENERATOR);
		if (auto subform = Subform(form, "Modifiers"))
			Draw(form, desc.modifier);
	}
	
	void SignalDocument::Draw(Form& form, GamepadAxis2dDesc& desc)
	{
		DRAW_FIELDS(GAMEPAD_AXIS_2D_PARTIAL_GENERATOR);
		if (auto subform = Subform(form, "Modifiers"))
			Draw(form, desc.modifier);
	}
	
	void SignalDocument::Draw(Form& form, CursorPosDesc& desc)
	{
		DRAW_FIELDS(CURSOR_POS_PARTIAL_GENERATOR);
		if (auto subform = Subform(form, "Modifiers"))
			Draw(form, desc.modifier);
	}
	
	void SignalDocument::Draw(Form& form, ScrollDesc& desc)
	{
		DRAW_FIELDS(SCROLL_PARTIAL_GENERATOR);
		if (auto subform = Subform(form, "Modifiers"))
			Draw(form, desc.modifier);
	}

	void SignalDocument::Draw(Form& form, Modifier0dDesc& desc)
	{
		DRAW_FIELDS(MODIFIER_0D_PARTIAL_GENERATOR);
		Draw(form, desc.base);
	}
	
	void SignalDocument::Draw(Form& form, Modifier1dDesc& desc)
	{
		DRAW_FIELDS(MODIFIER_1D_PARTIAL_GENERATOR);
		Draw(form, desc.base);
	}
	
	void SignalDocument::Draw(Form& form, Modifier2dDesc& desc)
	{
		DRAW_FIELDS(MODIFIER_2D_PARTIAL_GENERATOR);
		Draw(form, desc.base);
	}
	
	void SignalDocument::Draw(Form& form, ModifierBaseDesc& desc)
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
		case detail::SignalBindingType::Key:
		{
			KeyDesc subdesc;
			Load(node, subdesc);
			desc.variant.Set(std::move(subdesc));
			break;
		}
		case detail::SignalBindingType::MouseButton:
		{
			MouseButtonDesc subdesc;
			Load(node, subdesc);
			desc.variant.Set(std::move(subdesc));
			break;
		}
		case detail::SignalBindingType::GamepadButton:
		{
			GamepadButtonDesc subdesc;
			Load(node, subdesc);
			desc.variant.Set(std::move(subdesc));
			break;
		}
		case detail::SignalBindingType::GamepadAxis1D:
		{
			GamepadAxis1dDesc subdesc;
			Load(node, subdesc);
			desc.variant.Set(std::move(subdesc));
			break;
		}
		case detail::SignalBindingType::GamepadAxis2D:
		{
			GamepadAxis2dDesc subdesc;
			Load(node, subdesc);
			desc.variant.Set(std::move(subdesc));
			break;
		}
		case detail::SignalBindingType::CursorPos:
		{
			CursorPosDesc subdesc;
			Load(node, subdesc);
			desc.variant.Set(std::move(subdesc));
			break;
		}
		case detail::SignalBindingType::Scroll:
		{
			ScrollDesc subdesc;
			Load(node, subdesc);
			desc.variant.Set(std::move(subdesc));
			break;
		}
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
	
	void SignalDocument::Load(TOMLNode node, GamepadAxis1dDesc& desc)
	{
		LOAD_FIELDS(GAMEPAD_AXIS_1D_PARTIAL_GENERATOR);
		Load(node[detail::encode_key(SignalDesc::modifier_key)], desc.modifier);
	}
	
	void SignalDocument::Load(TOMLNode node, GamepadAxis2dDesc& desc)
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
	
	void SignalDocument::Dump(toml::table& table, GamepadAxis1dDesc& desc)
	{
		DUMP_FIELDS(GAMEPAD_AXIS_1D_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.modifier);
		table.insert_or_assign(detail::encode_key(SignalDesc::modifier_key), std::move(subtable));
	}
	
	void SignalDocument::Dump(toml::table& table, GamepadAxis2dDesc& desc)
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
