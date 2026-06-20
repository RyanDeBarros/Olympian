#include "FontDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"

#include "gui/scopes/IDScope.h"
#include "gui/scopes/Form.h"
#include "gui/scopes/Subform.h"
#include "gui/graphics/Outline.h"

#include "definitions/Keys.h"
#include "util/Counter.h"
#include "util/Hash.h"
#include "util/Parser.h"

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
		gui::PropertyGrid::Clear();

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

		if (gui::PropertyGrid::DirtyGrid())
			MarkDirty();
	}

	void FontDocument::Load()
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
			Draw(_scratch.font_face);
	}

	void FontDocument::DrawFontAtlases()
	{
		if (ImGui::BeginTable("", 2))
		{
			ImGui::TableNextColumn();
			if (auto form = Form())
			{
				gui::IDScope scope("##Atlas");
				DescIO::KeyLabel("Select Atlas");
				gui::PropertyGrid::SetColumn(gui::PropertyGrid::Value);
				_atlas_slots.DrawComboHeader("Atlas", "New atlas", "Delete atlas", "Clear atlas");
				gui::PropertyGrid::SubmitRow();

				if (!_scratch.font_atlases.Empty())
					Draw(_scratch.font_atlases[_atlas_slots.active_index]);

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

	void FontDocument::Draw(FontFaceDesc& desc)
	{
		DRAW_FIELDS(FONT_FACE_PARTIAL_GENERATOR);

		struct CodepointHash
		{
			size_t operator()(const std::string& str) const
			{
				if (auto v = stocdpt(str))
					return std::hash<int>{}(*v);
				else
					return 0;
			}
		};

		struct CodepointPairEquality
		{
			bool operator()(const std::array<std::string, 2>& lhs, const std::array<std::string, 2>& rhs) const
			{
				return stocdpt(lhs[0]) == stocdpt(rhs[0]) && stocdpt(lhs[1]) == stocdpt(rhs[1]);
			}
		};

		Counter<std::array<std::string, 2>, ArrayHash<std::string, CodepointHash>, CodepointPairEquality> counter;
		for (const auto& k : desc.kerning.vector)
			counter.increment(k.pair.scratch);

		DescIO::DrawDynamicList("Kerning", desc.kerning.vector, {}, [&desc, &counter](gui::DynamicRow& row) -> DrawResult {
			auto& k = desc.kerning[row.Index()];
			DrawResult result;

			ImGui::SameLine();
			bool dup_warning = counter.count(k.pair.scratch) > 1;
			gui::Outline dup_outline;
			for (size_t i = 0; i < 2; ++i)
			{
				bool bad_codepoint = !stocdpt(k.pair.scratch[i]).has_value();
				gui::Outline bad_outline;
				if (bad_codepoint)
					dup_warning = false;

				DrawResult codepoint_result = gui::InputData<std::string>{}(k.pair.sublabels[i], k.pair.scratch[i]);
				result |= codepoint_result;

				if (dup_warning && codepoint_result.IsHovered())
					ImGui::SetTooltip("Duplicate codepoint pair");

				if (bad_codepoint)
				{
					if (codepoint_result.IsHovered())
						ImGui::SetTooltip("Bad codepoint format");

					bad_outline.Draw(Color::Error);
				}

				ImGui::SameLine();
			}

			if (dup_warning)
				dup_outline.Draw(Color::Error);

			ImGui::Text(k.pair.label); // TODO v9.1 pair label renders higher for some reason - seems similar to tree view some nodes rendering a few pixels higher

			gui::VerticalSeparator();
			result |= gui::InputData<int>{}(k.distance.label, k.distance.scratch);

			if (k.distance.scratch != k.distance.def || k.pair.scratch != k.pair.def)
			{
				// TODO v9.1 this is inside of value component draw() -> this should run before SubmitRow() so that reset buttons can be added to reset column. Reset button should be at the correct inner row within the cell as well.
				//if (DescIO::DrawRevertButton())
				//{
				//	k.distance.scratch = k.distance.def;
				//	k.pair.scratch = k.pair.def;
				//	result.SetDirty(true);
				//}
			}

			return result.IsDirty();
		}, desc.kerning_ui_state);
	}
	
	void FontDocument::Draw(FontAtlasDesc& desc)
	{
		desc.font_size.Draw();
		if (gui::PropertyGrid::DirtyValue())
			DestroyFont();

		DRAW_FIELDS(FONT_ATLAS_NONPREVIEW_GENERATOR);

		if (auto subform = Subform("Common buffer"))
		{
			desc.use_common_buffer_preset.Draw();

			bool preset = desc.use_common_buffer_preset.scratch;
			
			if (auto disabled = DisabledSection(!preset))
			{
				desc.common_buffer_preset.Draw();

				gui::IDScope scope(&desc.common_buffer_preset);
				scope.Push("##Preview");
				DescIO::KeyLabel("Preview");
				gui::PropertyGrid::SetColumn(gui::PropertyGrid::Value);
				std::string buf = detail::buffer_of(desc.common_buffer_preset.scratch);
				ImGui::InputText("##PresetBuffer", buf.data(), buf.size() + 1, ImGuiInputTextFlags_ReadOnly);
				gui::PropertyGrid::SubmitRow();
			}

			if (auto disabled = DisabledSection(preset))
				desc.common_buffer.Draw();
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
