#include "FontDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"

#include "gui/IDScope.h"
#include "gui/Subform.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	const char* FontDocument::GetVersion()
	{
		return "1.0";
	}

	FontDocument::~FontDocument()
	{
		DestroyFont();
	}

	void FontDocument::Init()
	{
		if (!GetSourcePath().is_resource())
		{
			Notification notif(LogLevel::Warning, "Asset is not located in resource folder");
			MainWindow::Instance().PushNotification(std::move(notif));
		}

		_atlas_slots.policy = gui::ListPolicy::MinimumOne;
		_display_text = "Abc 123";
		Load();
	}

	void FontDocument::Draw()
	{
		gui::IDScope scope(this);

		if (ImGui::BeginTabBar(""))
		{
			if (ImGui::BeginTabItem("Font Face"))
			{
				DrawFontFace();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Font Atlases"))
			{
				DrawFontAtlases();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	void FontDocument::DrawMenuBar()
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

	void FontDocument::Load()
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
				Notification notif(LogLevel::Error, "cannot load font - corrupted asset: " + GetSourcePath().string());
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
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Font);

			MarkDirty();
		}

		_scratch = _disk;

		_atlas_slots.Init(*_scratch.font_atlases.ListAdapter());
	}

	void FontDocument::Dump()
	{
		toml::table table;
		Dump(table, _scratch);
		_oly_path.dump_toml(table, _meta);
		_disk = _scratch;
		MarkClean();
	}

	detail::ResourcePath FontDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}

	void FontDocument::ReloadFont()
	{
		DestroyFont();
		_preview_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(GetSourcePath().string().c_str(), _scratch.font_atlases[_atlas_slots.active_index].font_size.scratch);
	}

	void FontDocument::DestroyFont()
	{
		if (_preview_font)
		{
			ImGui::GetIO().Fonts->RemoveFont(_preview_font);
			_preview_font = nullptr;
		}
	}

	void FontDocument::DrawFontFace()
	{
		if (auto form = Form())
			Draw(form, _scratch.font_face);
	}

	void FontDocument::DrawFontAtlases()
	{
		if (ImGui::BeginTable("", 2))
		{
			ImGui::TableNextColumn();
			if (auto form = Form())
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Select Route");

				ImGui::TableNextColumn();
				_atlas_slots.DrawComboHeader("Atlas", "New atlas", "Delete atlas", "Clear atlas");

				if (!_scratch.font_atlases.Empty())
					Draw(form, _scratch.font_atlases[_atlas_slots.active_index]);

				if (_atlas_slots.ConsumeOps(*_scratch.font_atlases.ListAdapter()))
					MarkDirty();

				if (_atlas_slots.active_index.ConsumeModified())
					DestroyFont();
			}

			ImGui::TableNextColumn();
			DrawAtlasPreview();

			ImGui::EndTable();
		}
	}

	void FontDocument::DrawAtlasPreview()
	{
		if (ImGui::BeginChild("Preview", ImVec2(0, 0), ImGuiChildFlags_Borders))
		{
			ImGui::Text("Preview");
			ImGui::Separator();

			gui::InputText("Display text", _display_text);

			if (!_preview_font)
				ReloadFont();

			ImGui::PushFont(_preview_font);
			ImGui::Text(_display_text.c_str());
			ImGui::PopFont();
		}

		ImGui::EndChild();
	}

	void FontDocument::Draw(Form& form, FontFaceDesc& desc)
	{
		DRAW_FIELDS(FONT_FACE_PARTIAL_GENERATOR);

		DescIO::DrawDynamicList("Kerning", desc.kerning.vector, {}, [&desc](gui::DynamicRow& row) {
			auto& k = desc.kerning[row.Index()];
			bool dirty = false;
			
			ImGui::SameLine();
			ImGui::Text(k.pair.label); // TODO v8 pair label renders higher for some reason - seems similar to tree view some nodes rendering a few pixels higher

			for (size_t i = 0; i < 2; ++i)
			{
				ImGui::SameLine();
				dirty |= gui::InputData<std::string>{}(k.pair.sublabels[i], k.pair.scratch[i]);
			}

			ImGui::SameLine();
			ImGui::Text(k.distance.label);
			ImGui::SameLine();
			dirty |= gui::InputData<int>{}("##Distance", k.distance.scratch);

			if (k.distance.scratch != k.distance.def || k.pair.scratch != k.pair.def)
			{
				if (DescIO::DrawRevertButton())
				{
					// TODO v8 StructField so that there's one scratch/def
					k.distance.scratch = k.distance.def;
					k.pair.scratch = k.pair.def;
					dirty = true;
				}
			}
			
			return dirty;
		}, desc.kerning_ui_state);
	}

	void FontDocument::Draw(Form& form, FontAtlasDesc& desc)
	{
		if (desc.font_size.Draw())
		{
			MarkDirty();
			DestroyFont();
		}

		DRAW_FIELDS(FONT_ATLAS_NONPREVIEW_GENERATOR);

		if (auto subform = Subform(form, "Common buffer"))
		{
			DRAW_FIELD(use_common_buffer_preset);

			bool preset = desc.use_common_buffer_preset.scratch;
			
			if (auto disabled = DisabledSection(!preset))
			{
				DRAW_FIELD(common_buffer_preset);

				DescIO::PrepareValue("Preview");
				std::string buf = detail::buffer_of(desc.common_buffer_preset.scratch);
				ImGui::InputText("##PresetBuffer", buf.data(), buf.size() + 1, ImGuiInputTextFlags_ReadOnly);
			}

			if (auto disabled = DisabledSection(preset))
			{
				DRAW_FIELD(common_buffer);
			}
		}
	}

	void FontDocument::Load(TOMLNode node, FullFontDesc& desc)
	{
		Load(node[detail::encode_key(desc.font_face_key)], desc.font_face);

		TOMLArray array = node[detail::encode_key(desc.font_atlas_key)].as_array();
		if (array && !array->empty())
		{
			for (size_t i = 0; i < array->size(); ++i)
			{
				desc.font_atlases.PushBack();
				Load(TOMLNode(*array->get(i)), desc.font_atlases.vector.back());
			}
		}
		else
		{
			desc.font_atlases.PushBack();
			Load(TOMLNode(), desc.font_atlases.vector.back());
		}
	}

	void FontDocument::Load(TOMLNode node, FontFaceDesc& desc)
	{
		LOAD_FIELDS(FONT_FACE_PARTIAL_GENERATOR);

		TOMLArray array = node[detail::encode_key(desc.kerning_key)].as_array();
		if (array && !array->empty())
		{
			for (size_t i = 0; i < array->size(); ++i)
			{
				desc.kerning.PushBack();
				Load(TOMLNode(*array->get(i)), desc.kerning.vector.back());
			}
		}
	}

	void FontDocument::Load(TOMLNode node, KerningDesc& desc)
	{
		LOAD_FIELDS(KERNING_GENERATOR);
	}

	void FontDocument::Load(TOMLNode node, FontAtlasDesc& desc)
	{
		LOAD_FIELDS(FONT_ATLAS_GENERATOR);
	}

	void FontDocument::Dump(toml::table& table, FullFontDesc& desc)
	{
		toml::table subtable;
		Dump(subtable, desc.font_face);
		table.insert_or_assign(detail::encode_key(desc.font_face_key), std::move(subtable));

		toml::array array;
		for (auto& d : desc.font_atlases)
		{
			toml::table subtable;
			Dump(subtable, d);
			array.push_back(std::move(subtable));
		}
		table.insert_or_assign(detail::encode_key(desc.font_atlas_key), std::move(array));
	}

	void FontDocument::Dump(toml::table& table, FontFaceDesc& desc)
	{
		DUMP_FIELDS(FONT_FACE_PARTIAL_GENERATOR);
		
		toml::array array;
		for (auto& d : desc.kerning)
		{
			toml::table subtable;
			Dump(subtable, d);
			array.push_back(std::move(subtable));
		}
		table.insert_or_assign(detail::encode_key(desc.kerning_key), std::move(array));
	}

	void FontDocument::Dump(toml::table& table, KerningDesc& desc)
	{
		DUMP_FIELDS(KERNING_GENERATOR);
	}

	void FontDocument::Dump(toml::table& table, FontAtlasDesc& desc)
	{
		DUMP_FIELDS(FONT_ATLAS_GENERATOR);
	}
}
