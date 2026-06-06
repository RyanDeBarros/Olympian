#include "ProjectDocument.h"

#include "core/ProjectInfo.h"
#include "core/Logger.h"
#include "core/MainWindow.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	const char* ProjectDocument::GetVersion()
	{
		return "1.0";
	}

	void ProjectDocument::Init()
	{
		Load();
	}

	void ProjectDocument::Draw()
	{
		ImGui::PushID(this);
		Draw(_scratch);
		ImGui::PopID();
	}

	void ProjectDocument::DrawMenuBar()
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

	void ProjectDocument::Load()
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
				Notification notif(LogLevel::Error, "cannot load project file - corrupted asset: " + GetOlyPath().string());
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
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Project);

			MarkDirty();
		}

		_scratch.Reset(_disk);
	}

	void ProjectDocument::Dump()
	{
		toml::table table;
		Dump(table, _scratch);
		_oly_path.dump_toml(table, _meta);
		_disk.Reset(_scratch);
		MarkClean();
	}

	std::string ProjectDocument::TabName() const
	{
		return ProjectInfo::Instance().ProjectName();
	}

	void ProjectDocument::Draw(ProjectDesc& desc)
	{
		if (auto form = Form())
		{
			Draw(form, desc.context);
		}
	}
	
	void ProjectDocument::Draw(Form& form, ContextDesc& desc)
	{
		Draw(form, desc.logger);
	}
	
	void ProjectDocument::Draw(Form& form, LoggerDesc& desc)
	{
		if (auto collapse = form.Collapse("Logger"))
		{
			DRAW_FIELDS(LOGGER_PARTIAL_GENERATOR);

			Draw(form, desc.enable);
		}
	}
	
	void ProjectDocument::Draw(Form& form, LoggerEnableDesc& desc)
	{
		if (auto collapse = form.Collapse("Enable Streams"))
		{
			DRAW_FIELDS(LOGGER_ENABLE_GENERATOR);
		}
	}

	void ProjectDocument::Load(TOMLNode node, ProjectDesc& desc)
	{
		Load(node[detail::encode_key(desc.context_key)], desc.context);
	}
	
	void ProjectDocument::Load(TOMLNode node, ContextDesc& desc)
	{
		Load(node[detail::encode_key(desc.logger_key)], desc.logger);
	}
	
	void ProjectDocument::Load(TOMLNode node, LoggerDesc& desc)
	{
		LOAD_FIELDS(LOGGER_PARTIAL_GENERATOR);

		Load(node[detail::encode_key(desc.enable_key)], desc.enable);
	}
	
	void ProjectDocument::Load(TOMLNode node, LoggerEnableDesc& desc)
	{
		LOAD_FIELDS(LOGGER_ENABLE_GENERATOR);
	}

	void ProjectDocument::Dump(toml::table& table, ProjectDesc& desc)
	{
		toml::table subtable;
		Dump(subtable, desc.context);
		table.insert_or_assign(detail::encode_key(desc.context_key), std::move(subtable));
	}
	
	void ProjectDocument::Dump(toml::table& table, ContextDesc& desc)
	{
		toml::table subtable;
		Dump(subtable, desc.logger);
		table.insert_or_assign(detail::encode_key(desc.logger_key), std::move(subtable));
	}
	
	void ProjectDocument::Dump(toml::table& table, LoggerDesc& desc)
	{
		DUMP_FIELDS(LOGGER_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.enable);
		table.insert_or_assign(detail::encode_key(desc.enable_key), std::move(subtable));
	}
	
	void ProjectDocument::Dump(toml::table& table, LoggerEnableDesc& desc)
	{
		DUMP_FIELDS(LOGGER_ENABLE_GENERATOR);
	}
}
