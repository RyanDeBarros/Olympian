#include "ProjectDocument.h"

#include "core/ProjectInfo.h"
#include "core/Logger.h"
#include "core/MainWindow.h"

#include "gui/Subform.h"

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

			// TODO v12 Run menu to configure cmake, build project, and run executable

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

	// TODO v8 File menu in main menu bar to open project settings

	void ProjectDocument::Draw(ProjectDesc& desc)
	{
		if (auto form = Form())
			Draw(form, desc.context);
	}
	
	void ProjectDocument::Draw(Form& form, ContextDesc& desc)
	{
		if (auto subform = Subform(form, "Platform"))
			Draw(subform.GetForm(), desc.platform);

		if (auto subform = Subform(form, "Logger"))
			Draw(subform.GetForm(), desc.logger);
	}

	void ProjectDocument::Draw(Form& form, PlatformDesc& desc)
	{
		if (auto subform = Subform(form, "Window"))
			Draw(subform.GetForm(), desc.window);
		
		DRAW_FIELDS(PLATFORM_PARTIAL_GENERATOR);
	}
	
	void ProjectDocument::Draw(Form& form, WindowDesc& desc)
	{
		DRAW_FIELDS(WINDOW_PARTIAL_GENERATOR);

		if (auto subform = Subform(form, "Window hints"))
			Draw(subform.GetForm(), desc.window_hints);
	}
	
	void ProjectDocument::Draw(Form& form, WindowHintsDesc& desc)
	{
		DRAW_FIELDS(WINDOW_HINTS_GENERATOR);
	}

	void ProjectDocument::Draw(Form& form, LoggerDesc& desc)
	{
		DRAW_FIELDS(LOGGER_PARTIAL_GENERATOR);
		if (auto subform = Subform(form, "Enable Streams"))
			Draw(subform.GetForm(), desc.enable);
	}
	
	void ProjectDocument::Draw(Form& form, LoggerEnableDesc& desc)
	{
		DRAW_FIELDS(LOGGER_ENABLE_GENERATOR);
	}

	void ProjectDocument::Load(TOMLNode node, ProjectDesc& desc)
	{
		Load(node[detail::encode_key(desc.context_key)], desc.context);
	}
	
	void ProjectDocument::Load(TOMLNode node, ContextDesc& desc)
	{
		Load(node[detail::encode_key(desc.platform_key)], desc.platform);
		Load(node[detail::encode_key(desc.logger_key)], desc.logger);
	}

	void ProjectDocument::Load(TOMLNode node, PlatformDesc& desc)
	{
		LOAD_FIELDS(PLATFORM_PARTIAL_GENERATOR);

		Load(node[detail::encode_key(desc.window_key)], desc.window);
	}

	void ProjectDocument::Load(TOMLNode node, WindowDesc& desc)
	{
		LOAD_FIELDS(WINDOW_PARTIAL_GENERATOR);

		Load(node[detail::encode_key(desc.window_hints_key)], desc.window_hints);
	}
	
	void ProjectDocument::Load(TOMLNode node, WindowHintsDesc& desc)
	{
		LOAD_FIELDS(WINDOW_HINTS_GENERATOR);
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
		Dump(subtable, desc.platform);
		table.insert_or_assign(detail::encode_key(desc.platform_key), std::move(subtable));
		subtable.clear();
		Dump(subtable, desc.logger);
		table.insert_or_assign(detail::encode_key(desc.logger_key), std::move(subtable));
	}

	void ProjectDocument::Dump(toml::table& table, PlatformDesc& desc)
	{
		DUMP_FIELDS(PLATFORM_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.window);
		table.insert_or_assign(detail::encode_key(desc.window_key), std::move(subtable));
	}

	void ProjectDocument::Dump(toml::table& table, WindowDesc& desc)
	{
		DUMP_FIELDS(WINDOW_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.window_hints);
		table.insert_or_assign(detail::encode_key(desc.window_hints_key), std::move(subtable));
	}

	void ProjectDocument::Dump(toml::table& table, WindowHintsDesc& desc)
	{
		DUMP_FIELDS(WINDOW_HINTS_GENERATOR);
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
