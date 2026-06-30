#include "FontDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"

#include "gui/InlineWidget.h"
#include "gui/scopes/IDScope.h"
#include "gui/scopes/Form.h"
#include "gui/scopes/Subform.h"
#include "gui/graphics/Outline.h"

#include "definitions/Keys.h"

#include "util/Counter.h"
#include "util/Hash.h"
#include "util/Parser.h"
#include "util/DynamicArray.h"

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

	void FontDocument::InitImpl()
	{
		if (!GetSourcePath().is_resource())
		{
			Notification notif(LogLevel::Warning, "Asset is not located in resource folder");
			MainWindow::Instance().PushNotification(std::move(notif));
		}

		_atlas_slots.policy = gui::ListPolicy::MinimumOne;
		_display_text = "Abc 123";
		LoadAsset();
	}

	void FontDocument::Draw()
	{
		auto pre_draw = PreDraw();

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

	void FontDocument::LoadImpl()
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
				Notification notif(LogLevel::Error, "cannot load font - corrupted asset: " + GetSourcePath().string());
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
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Font);

			MarkDirty();
		}

		_desc.LoadFromDisk();

		_atlas_slots.Init(*FontAtlasListAdapter());
	}

	void FontDocument::DumpImpl()
	{
		toml::table table;
		Dump(table, _desc.scratch);
		_oly_path.dump_toml(table, _meta);
		_desc.WriteToDisk();
		MarkClean();
	}

	const IDoubleDescriptor& FontDocument::GetDoubleDescriptor() const
	{
		return _desc;
	}

	IDoubleDescriptor& FontDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	detail::ResourcePath FontDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}

	void FontDocument::ReloadFont()
	{
		DestroyFont();
		_preview_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(GetSourcePath().string().c_str(), _desc.scratch.font_atlases[_atlas_slots.active_index].font_size.value);
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
			Draw(DataPath() / _desc.scratch.subpaths.font_face, _desc.scratch.font_face);
	}

	void FontDocument::DrawFontAtlases()
	{
		if (ImGui::BeginTable("", 2))
		{
			ImGui::TableNextColumn();
			if (auto form = Form())
			{
				_atlas_slots.Update(*FontAtlasListAdapter());
				
				if (auto scope = gui::IDScope("##Atlas"))
				{
					gui::PropertyGrid::Key::SetLabel("Select Atlas");
					gui::PropertyGrid::Value::AddComponent(comp::Generic([this]() -> DrawResult {
						return _atlas_slots.DrawComboHeader("Atlas", "New atlas", "Delete atlas", "Clear atlas");
					}, false));
					gui::PropertyGrid::SubmitRow();
				}

				if (!_desc.scratch.font_atlases.Empty())
					Draw(DataPath() / _desc.scratch.subpaths.font_atlases, _desc.scratch.font_atlases[_atlas_slots.active_index]);

				if (_atlas_slots.ConsumeOps(*FontAtlasListAdapter()))
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

	void FontDocument::Draw(DataPath path, FontFaceDesc& desc)
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
		for (auto& k : desc.kerning.vector)
		{
			k.distance.edit.PreEdit();
			k.pair.edits[0].PreEdit();
			k.pair.edits[1].PreEdit();
			counter.increment({ k.pair.edits[0].buffer, k.pair.edits[1].buffer });
		}

		for (size_t i = 0; i < desc.kerning.Size(); ++i)
		{
			auto& k = desc.kerning[i];
			if (k.distance.edit.buffer != k.distance.def || k.pair.edits[0].buffer != k.pair.def[0] || k.pair.edits[1].buffer != k.pair.def[1])
				gui::PropertyGrid::Reset::Button(1 + i);
		}

		DescIO::DrawDynamicList(path / desc.subpaths.kerning, "Kerning", desc.kerning.vector, {}, [&desc, &counter](gui::DynamicRow& row) -> DrawResult {
			DynamicArray<gui::WidgetComponent> components;
			auto& k = desc.kerning[row.Index()];

			bool dup_warning = counter.count({ k.pair.edits[0].buffer, k.pair.edits[1].buffer }) > 1;
			gui::Outline dup_outline;
			for (size_t i = 0; i < 2; ++i)
			{
				components.push_back(comp::Generic([&k, i, &dup_warning, &dup_outline]() -> DrawResult {
					bool bad_codepoint = !stocdpt(k.pair.edits[i].buffer).has_value();
					gui::Outline bad_outline;
					if (bad_codepoint)
						dup_warning = false;

					DrawResult result = gui::InputData<std::string>{}(k.pair.sublabels ? k.pair.sublabels[i] : ("##" + std::to_string(i)).c_str(), k.pair.edits[i].buffer);
					k.pair.edits[i].PostEdit(result);

					if (dup_warning && result.IsHovered())
						ImGui::SetTooltip("Duplicate codepoint pair");

					if (bad_codepoint)
					{
						if (result.IsHovered())
							ImGui::SetTooltip("Bad codepoint format");

						bad_outline.Draw(Color::Error);
					}

					if (i == 1)
					{
						if (dup_warning)
							dup_outline.Draw(Color::Error);
					}

					return result;
				}, true));
			}

			components.push_back(comp::Text(k.pair.label));
			components.push_back(comp::VerticalSeparator());

			components.push_back(comp::Generic([&k]() -> DrawResult {
				DrawResult result = gui::InputData<int>{}("##Distance", k.distance.edit.buffer);
				k.distance.edit.PostEdit(result);
				return result;
			}, true));
			std::string distance_label = k.distance.label + std::string(" ");
			components.push_back(comp::Text(distance_label.c_str()));

			return gui::InlineWidget(components);
		}, desc.kerning_ui_state);

		auto kerning_path = path / desc.subpaths.kerning;
		for (size_t i = 0; i < desc.kerning.Size(); ++i)
		{
			KerningDesc& k = desc.kerning[i];
			auto kerning_subpath = kerning_path / desc.kerning.Subpath(i);
			if (gui::PropertyGrid::Reset::Activated(1 + i))
			{
				k.distance.edit.PublishReset(k.distance.def);
				k.pair.edits[0].PublishReset(k.pair.def[0]);
				k.pair.edits[1].PublishReset(k.pair.def[1]);
				MarkDirty();
			}

			bool publish_action = false;
			publish_action |= k.distance.edit.ConsumeModified();
			publish_action |= k.pair.edits[0].ConsumeModified();
			publish_action |= k.pair.edits[1].ConsumeModified();
			if (publish_action)
			{
				KerningDesc original;
				original.distance.value = std::move(k.distance.edit.original);
				original.pair.value[0] = std::move(k.pair.edits[0].original);
				original.pair.value[1] = std::move(k.pair.edits[1].original);
				PushFieldSetAction(kerning_subpath, std::move(original), k);
			}
		}
	}
	
	void FontDocument::Draw(DataPath path, FontAtlasDesc& desc)
	{
		DRAW_FIELD(font_size);
		if (gui::PropertyGrid::DirtyRow())
			DestroyFont();

		DRAW_FIELDS(FONT_ATLAS_NONPREVIEW_GENERATOR);

		if (auto subform = Subform("Common buffer"))
		{
			DRAW_FIELD(use_common_buffer_preset);

			bool preset = desc.use_common_buffer_preset.value;
			
			if (auto disabled = DisabledSection(!preset))
			{
				DRAW_FIELD(common_buffer_preset);

				if (auto scope = gui::IDScope(&desc.common_buffer_preset))
				{
					scope.Push("##Preview");
					gui::PropertyGrid::Key::SetLabel("Preview");
					gui::PropertyGrid::Value::AddComponent(comp::Generic([&desc]() -> DrawResult {
						std::string buf = detail::buffer_of(desc.common_buffer_preset.value);
						ImGui::InputText("##PresetBuffer", buf.data(), buf.size() + 1, ImGuiInputTextFlags_ReadOnly);
						return false;
					}, true));
					gui::PropertyGrid::SubmitRow();
				}
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

	struct BriefDescPrinter
	{
		void operator()(std::ostream& os, const FontAtlasDesc& desc) const
		{
			os << "FontAtlasDesc[font_size=" << desc.font_size.value << ", ...]";
		}
	};

	std::unique_ptr<gui::IListAdapter> FontDocument::FontAtlasListAdapter()
	{
		return _desc.scratch.font_atlases.ListAdapter<BriefDescPrinter>(DataPath() / _desc.scratch.subpaths.font_atlases);
	}
}
