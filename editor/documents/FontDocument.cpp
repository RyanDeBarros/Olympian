#include "FontDocument.h"

#include "core/editor/Notifier.h"

#include "gui/InlineWidget.h"
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
			Notifier::NotifyWarning("Asset is not located in resource folder");

		_atlas_slots.policy = gui::ListPolicy::MinimumOne;
		_display_text = "Abc 123";
		LoadAsset();
	}

	void FontDocument::Draw()
	{
		auto pre_draw = PreDraw();

		imtk::id_scope scope(this);

		if (auto _ = imtk::tab_bar(""))
		{
			if (auto _ = imtk::tab_item("Font Face"))
				DrawFontFace();

			if (auto _ = imtk::tab_item("Font Atlases"))
				DrawFontAtlases();
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
				Notifier::NotifyError("cannot load font - corrupted asset: " + GetSourcePath().string());

			MarkClean();
		}
		else
		{
			Load(TOMLNode(), _desc.disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = GetVersion();
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

	void FontDocument::ResetAssetImpl()
	{
		Load(TOMLNode(), _desc.scratch);
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
			Draw(_desc.scratch.font_face);
	}

	void FontDocument::DrawFontAtlases()
	{
		if (auto _ = imtk::table("", 2))
		{
			ImGui::TableNextColumn();
				
			_atlas_slots.Update(*FontAtlasListAdapter());
			if (auto scope = imtk::id_scope("##Atlas"))
				_atlas_slots.DrawComboHeader({ .prompt = "Select atlas", .create_tooltip = "New atlas", .delete_tooltip = "Delete atlas", .clear_tooltip = "Clear atlases" }, "Atlas");
				
			if (auto form = Form())
			{
				if (!_desc.scratch.font_atlases.Empty())
					Draw(_desc.scratch.font_atlases[_atlas_slots.active_index]);

				if (_atlas_slots.ConsumeOps(*FontAtlasListAdapter()))
					MarkDirty();

				if (_atlas_slots.active_index.ConsumeModified())
					DestroyFont();
			}

			ImGui::TableNextColumn();
			DrawAtlasPreview();
		}
	}

	void FontDocument::DrawAtlasPreview()
	{
		if (auto _ = imtk::child("Preview", ImVec2(0, 0), ImGuiChildFlags_Borders))
		{
			ImGui::TextUnformatted("Preview");
			ImGui::Separator();

			gui::InputData<std::string>{}("Display text", _display_text);

			if (!_preview_font)
				ReloadFont();

			if (auto _ = imtk::font_scope(_preview_font))
				ImGui::TextUnformatted(_display_text.c_str());
		}
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
		for (auto& k : desc.kerning)
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

		DescIO::DrawDynamicList(desc.kerning.link, "Kerning", desc.kerning, {}, [&desc, &counter](gui::DynamicRow& row) -> DrawResult {
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

					DrawResult result;

					if (i == 0)
					{
						ImGui::TextUnformatted(k.pair.label);
						ImGui::SameLine();
						result.Query();
					}

					result |= gui::InputData<std::string>{}(k.pair.sublabels ? k.pair.sublabels[i] : ("##" + std::to_string(i)).c_str(), k.pair.edits[i].buffer);
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
				}));
			}

			components.push_back(comp::Generic([&k]() -> DrawResult {
				DrawResult result;
				imtk::controls::vertical_separator();
				ImGui::TextUnformatted(k.distance.label);
				result.Query();
				ImGui::SameLine();
				result |= gui::InputData<int>{}("##Distance", k.distance.edit.buffer);
				k.distance.edit.PostEdit(result);
				return result;
			}));

			return gui::InlineWidget::Draw(components);
		}, desc.kerning_ui_state);

		for (size_t i = 0; i < desc.kerning.Size(); ++i)
		{
			KerningDesc& k = desc.kerning[i];
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
				PushDescriptorSetAction(k.link.compute_path(), std::move(original), CloneDescData(k));
			}
		}
	}
	
	void FontDocument::Draw(FontAtlasDesc& desc)
	{
		DRAW_FIELD(font_size);
		if (gui::PropertyGrid::DirtyRow())
			DestroyFont();

		DRAW_FIELDS(FONT_ATLAS_NONPREVIEW_GENERATOR);

		if (auto subform = Subform("Common buffer"))
		{
			DRAW_FIELD(use_common_buffer_preset);

			bool preset = desc.use_common_buffer_preset.value;
			
			if (auto d = imtk::disabled(!preset))
			{
				DRAW_FIELD(common_buffer_preset);

				if (auto scope = imtk::id_scope(&desc.common_buffer_preset))
				{
					gui::PropertyGrid::Value::AddComponent(comp::Generic([&desc]() -> DrawResult {
						std::string buf = detail::buffer_of(desc.common_buffer_preset.value);
						ImGui::InputText("##PresetBuffer", buf.data(), buf.size() + 1, ImGuiInputTextFlags_ReadOnly);
						return false;
					}));
					gui::PropertyGrid::SubmitRow();
				}
			}

			if (auto d = imtk::disabled(preset))
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
				Load(TOMLNode(*array->get(i)), desc.font_atlases.Back());
			}
		}
		else
		{
			desc.font_atlases.PushBack();
			Load(TOMLNode(), desc.font_atlases.Back());
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
				Load(TOMLNode(*array->get(i)), desc.kerning.Back());
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
		return gui::MakeVectorAdapter<BriefDescPrinter>(_desc.scratch.font_atlases);
	}
}
