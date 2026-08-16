#include "ProjectDocument.h"

#include "core/editor/ProjectInfo.h"
#include "core/editor/Notifier.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	const char* ProjectDocument::GetVersion()
	{
		return "1.0";
	}

	ProjectDocument::ProjectDocument()
		: IDocument(ProjectInfo::Instance().ProjectPath())
	{
	}

	void ProjectDocument::InitImpl()
	{
		LoadAsset();
	}

	void ProjectDocument::Draw()
	{
		auto pre_draw = PreDraw();

		imtk::id_scope scope(this);
		Draw(_desc.scratch);
	}

	void ProjectDocument::DrawMenuBar()
	{
		if (auto _ = imtk::menu_bar())
		{
			if (auto _ = imtk::menu("File"))
			{
				if (ImGui::MenuItem("Save Changes", "Ctrl+S"))
					DumpAsset();

				if (ImGui::MenuItem("Discard Changes"))
					LoadAsset();

				if (ImGui::MenuItem("Reset Project Settings"))
					ResetAsset();
			}

			// TODO v13 Run menu to configure cmake, build project, and run executable
		}
	}

	void ProjectDocument::LoadImpl()
	{
		if (_oly_path.is_file())
		{
			_meta = detail::MetaSplitter::decode_meta(_oly_path);

			toml::table table;
			std::string err = _oly_path.load_toml(table);
			if (err.empty())
				Load(imtk::toml_node(table), _desc.disk);
			else
				Notifier::NotifyError("cannot load project file - corrupted asset: " + GetOlyPath().string());

			MarkClean();
		}
		else
		{
			Load(imtk::toml_node(), _desc.disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = GetVersion();
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Project);

			MarkDirty();
		}

		_desc.LoadFromDisk();
	}

	void ProjectDocument::DumpImpl()
	{
		toml::table table;
		Dump(table, _desc.scratch);
		_oly_path.dump_toml(table, _meta);
		_desc.WriteToDisk();
		MarkClean();
	}

	void ProjectDocument::ResetAssetImpl()
	{
		Load(imtk::toml_node(), _desc.scratch);
	}

	const IDoubleDescriptor& ProjectDocument::GetDoubleDescriptor() const
	{
		return _desc;
	}

	IDoubleDescriptor& ProjectDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	std::string ProjectDocument::TabName() const
	{
		return ProjectInfo::Instance().ProjectName();
	}

	void ProjectDocument::Draw(ProjectDesc& desc)
	{
		if (auto form = imtk::prop::form())
			Draw(desc.context);
	}
	
	void ProjectDocument::Draw(ContextDesc& desc)
	{
		if (auto subform = imtk::prop::subform("Platform"))
			Draw(desc.platform);

		if (auto subform = imtk::prop::subform("Collision"))
			Draw(desc.collision);

		if (auto subform = imtk::prop::subform("Logger"))
			Draw(desc.logger);

		if (auto subform = imtk::prop::subform("Frame Rate"))
			Draw(desc.frame_rate);
	}

	void ProjectDocument::Draw(PlatformDesc& desc)
	{
		if (auto subform = imtk::prop::subform("Window"))
			Draw(desc.window);
		
		IMTK_DRAW_FIELDS(PLATFORM_PARTIAL_GENERATOR);
	}
	
	void ProjectDocument::Draw(WindowDesc& desc)
	{
		IMTK_DRAW_FIELDS(WINDOW_PARTIAL_GENERATOR);

		if (auto subform = imtk::prop::subform("Viewport"))
			Draw(desc.viewport);

		if (auto subform = imtk::prop::subform("Window hints"))
			Draw(desc.window_hints);
	}

	void ProjectDocument::Draw(ViewportDesc& desc)
	{
		IMTK_DRAW_FIELDS(VIEWPORT_GENERATOR);
	}

	void ProjectDocument::Draw(WindowHintsDesc& desc)
	{
		IMTK_DRAW_FIELDS(WINDOW_HINTS_GENERATOR);
	}

	void ProjectDocument::Draw(CollisionDesc& desc)
	{
		IMTK_DRAW_FIELDS(COLLISION_GENERATOR);
	}

	void ProjectDocument::Draw(LoggerDesc& desc)
	{
		IMTK_DRAW_FIELDS(LOGGER_PARTIAL_GENERATOR);
		if (auto subform = imtk::prop::subform("Enable Streams"))
			Draw(desc.enable);
	}
	
	void ProjectDocument::Draw(LoggerEnableDesc& desc)
	{
		IMTK_DRAW_FIELDS(LOGGER_ENABLE_GENERATOR);
	}

	void ProjectDocument::Draw(FrameRateDesc& desc)
	{
		IMTK_DRAW_FIELDS(FRAME_RATE_GENERATOR);
	}

	void ProjectDocument::Load(imtk::toml_node node, ProjectDesc& desc)
	{
		Load(node[detail::encode_key(desc.context_key)], desc.context);
	}
	
	void ProjectDocument::Load(imtk::toml_node node, ContextDesc& desc)
	{
		Load(node[detail::encode_key(desc.platform_key)], desc.platform);
		Load(node[detail::encode_key(desc.collision_key)], desc.collision);
		Load(node[detail::encode_key(desc.logger_key)], desc.logger);
		Load(node[detail::encode_key(desc.frame_rate_key)], desc.frame_rate);
	}

	void ProjectDocument::Load(imtk::toml_node node, PlatformDesc& desc)
	{
		IMTK_LOAD_FIELDS(PLATFORM_PARTIAL_GENERATOR);

		Load(node[detail::encode_key(desc.window_key)], desc.window);
	}

	void ProjectDocument::Load(imtk::toml_node node, WindowDesc& desc)
	{
		IMTK_LOAD_FIELDS(WINDOW_PARTIAL_GENERATOR);

		Load(node[detail::encode_key(desc.viewport_key)], desc.viewport);
		Load(node[detail::encode_key(desc.window_hints_key)], desc.window_hints);
	}

	void ProjectDocument::Load(imtk::toml_node node, ViewportDesc& desc)
	{
		IMTK_LOAD_FIELDS(VIEWPORT_GENERATOR);
	}

	void ProjectDocument::Load(imtk::toml_node node, WindowHintsDesc& desc)
	{
		IMTK_LOAD_FIELDS(WINDOW_HINTS_GENERATOR);
	}

	void ProjectDocument::Load(imtk::toml_node node, CollisionDesc& desc)
	{
		IMTK_LOAD_FIELDS(COLLISION_GENERATOR);
	}

	void ProjectDocument::Load(imtk::toml_node node, LoggerDesc& desc)
	{
		IMTK_LOAD_FIELDS(LOGGER_PARTIAL_GENERATOR);

		Load(node[detail::encode_key(desc.enable_key)], desc.enable);
	}
	
	void ProjectDocument::Load(imtk::toml_node node, LoggerEnableDesc& desc)
	{
		IMTK_LOAD_FIELDS(LOGGER_ENABLE_GENERATOR);
	}

	void ProjectDocument::Load(imtk::toml_node node, FrameRateDesc& desc)
	{
		IMTK_LOAD_FIELDS(FRAME_RATE_GENERATOR);
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
		Dump(subtable, desc.collision);
		table.insert_or_assign(detail::encode_key(desc.collision_key), std::move(subtable));

		subtable.clear();
		Dump(subtable, desc.logger);
		table.insert_or_assign(detail::encode_key(desc.logger_key), std::move(subtable));

		subtable.clear();
		Dump(subtable, desc.frame_rate);
		table.insert_or_assign(detail::encode_key(desc.frame_rate_key), std::move(subtable));
	}

	void ProjectDocument::Dump(toml::table& table, PlatformDesc& desc)
	{
		IMTK_DUMP_FIELDS(PLATFORM_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.window);
		table.insert_or_assign(detail::encode_key(desc.window_key), std::move(subtable));
	}

	void ProjectDocument::Dump(toml::table& table, WindowDesc& desc)
	{
		IMTK_DUMP_FIELDS(WINDOW_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.viewport);
		table.insert_or_assign(detail::encode_key(desc.viewport_key), std::move(subtable));
		subtable.clear();
		Dump(subtable, desc.window_hints);
		table.insert_or_assign(detail::encode_key(desc.window_hints_key), std::move(subtable));
	}

	void ProjectDocument::Dump(toml::table& table, ViewportDesc& desc)
	{
		IMTK_DUMP_FIELDS(VIEWPORT_GENERATOR);
	}

	void ProjectDocument::Dump(toml::table& table, WindowHintsDesc& desc)
	{
		IMTK_DUMP_FIELDS(WINDOW_HINTS_GENERATOR);
	}

	void ProjectDocument::Dump(toml::table& table, CollisionDesc& desc)
	{
		IMTK_DUMP_FIELDS(COLLISION_GENERATOR);
	}

	void ProjectDocument::Dump(toml::table& table, LoggerDesc& desc)
	{
		IMTK_DUMP_FIELDS(LOGGER_PARTIAL_GENERATOR);

		toml::table subtable;
		Dump(subtable, desc.enable);
		table.insert_or_assign(detail::encode_key(desc.enable_key), std::move(subtable));
	}
	
	void ProjectDocument::Dump(toml::table& table, LoggerEnableDesc& desc)
	{
		IMTK_DUMP_FIELDS(LOGGER_ENABLE_GENERATOR);
	}

	void ProjectDocument::Dump(toml::table& table, FrameRateDesc& desc)
	{
		IMTK_DUMP_FIELDS(FRAME_RATE_GENERATOR);
	}
}
