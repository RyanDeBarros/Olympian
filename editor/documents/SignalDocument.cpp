#include "SignalDocument.h"

#include "core/MainWindow.h"
#include "core/Logger.h"

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
		ImGui::PushID(this);

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

		ImGui::PopID();
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

		_scratch.Reset(_disk);
	}

	void SignalDocument::Dump()
	{
		toml::table table;
		Dump(table, _scratch);
		_oly_path.dump_toml(table, _meta);
		_disk.Reset(_scratch);
		MarkClean();
	}

	void SignalDocument::DrawSignals()
	{
		// TODO v8 draw only active index
		_scratch.signals.Visit([this](SignalDesc& desc) { Draw(desc); });
	}

	void SignalDocument::DrawRoutes()
	{
		// TODO v8 draw only active index
		_scratch.routes.Visit([this](RouteDesc& desc) { Draw(desc); });
	}

	void SignalDocument::Draw(SignalDesc& desc)
	{
		// TODO v8
	}
	
	void SignalDocument::Draw(RouteDesc& desc)
	{
		// TODO v8
	}

	void SignalDocument::Load(TOMLNode node, SignalFullDesc& desc)
	{
		_scratch.signals.Visit([this, subnode = node[detail::encode_key(desc.signals_key)]](SignalDesc& desc) { Load(subnode, desc); });
		_scratch.routes.Visit([this, subnode = node[detail::encode_key(desc.routes_key)]](RouteDesc& desc) { Load(subnode, desc); });
	}

	void SignalDocument::Load(TOMLNode node, SignalDesc& desc)
	{
		// TODO v8
	}

	void SignalDocument::Load(TOMLNode node, RouteDesc& desc)
	{
		// TODO v8
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
		// TODO v8
	}

	void SignalDocument::Dump(toml::table& table, RouteDesc& desc)
	{
		// TODO v8
	}
}
